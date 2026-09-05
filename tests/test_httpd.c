/*
 * test_httpd.c — Tests for the first-party graph-UI HTTP server.
 *
 * Two layers:
 *   1. Parser/helper unit tests against httpd.h's pure functions
 *      (no sockets): request-line parsing, strict CRLF, Content-Length
 *      edge cases, chunked rejection, NUL/percent-decode rules,
 *      query-param decoding, route pattern matching.
 *   2. Live-socket integration tests against the full UI server
 *      (http_server.c) on an ephemeral port: routing, CORS policy,
 *      RPC dispatch, transport limits, receive deadline, clean shutdown.
 */
#include "../src/foundation/compat.h"
#include "../src/foundation/compat_fs.h"
#include "../src/foundation/compat_thread.h"
#include "../src/foundation/log.h"
#include "../src/foundation/platform.h"
#include "../src/cli/cli.h"
#include "../src/daemon/host_internal.h"
#include "../src/git/git_context.h" /* #798 follow-up: live-socket git-resolve repro */
#include "../src/ui/http_server.h"
#include "test_framework.h"
#include "test_helpers.h"
#include "ui/httpd.h"
#include "ui/http_server.h"
#include "foundation/identity.h"
#include "adr/adr_fill.h"
#include "mcp/mcp.h"
#include "pipeline/pipeline.h"
#include "pipeline/pipeline_internal.h"
#include <sqlite3.h>
#include <store/store.h>
#include <watcher/watcher.h>
#include <ctype.h>

#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h> /* #798 follow-up: CreateThread/WaitForSingleObject watchdog */
typedef SOCKET th_sock_t;
#define th_sock_close closesocket
#define th_sock_shutdown(s) shutdown((s), SD_BOTH)
#define TH_SOCK_BAD INVALID_SOCKET
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h> /* struct timeval for the SO_RCVTIMEO watchdog (#798 follow-up) */
#include <sys/wait.h> /* fork/waitpid crash-isolation for the browse overflow guard */
#include <unistd.h>
typedef int th_sock_t;
#define th_sock_close close
#define th_sock_shutdown(s) shutdown((s), SHUT_RDWR)
#define TH_SOCK_BAD (-1)
#endif

static char httpd_log_buf[8192];

static void httpd_capture_log(const char *line) {
    size_t used = strlen(httpd_log_buf);
    size_t avail = sizeof(httpd_log_buf) - used;
    if (avail <= 1)
        return;
    int n = snprintf(httpd_log_buf + used, avail, "%s\n", line ? line : "");
    if (n < 0 || (size_t)n >= avail)
        httpd_log_buf[sizeof(httpd_log_buf) - 1] = '\0';
}

/* ── Raw-socket test client ───────────────────────────────────── */

static th_sock_t th_connect_with_recv_buffer(int port, int recv_buffer) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); /* refcounted; cleanup not needed in tests */
#endif
    th_sock_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == TH_SOCK_BAD)
        return TH_SOCK_BAD;
    if (recv_buffer > 0 && setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char *)&recv_buffer,
                                      sizeof(recv_buffer)) != 0) {
        th_sock_close(s);
        return TH_SOCK_BAD;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(0x7F000001); /* 127.0.0.1 */
    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        th_sock_close(s);
        return TH_SOCK_BAD;
    }
    return s;
}

static th_sock_t th_connect(int port) {
    return th_connect_with_recv_buffer(port, 0);
}

static int th_send_all(th_sock_t s, const char *data, size_t len) {
    size_t off = 0;
    while (off < len) {
#ifdef _WIN32
        int n = send(s, data + off, (int)(len - off), 0);
#else
        ssize_t n = send(s, data + off, len - off, 0);
#endif
        if (n <= 0)
            return -1;
        off += (size_t)n;
    }
    return 0;
}

/* Read until the server closes the connection (Connection: close model). */
static int th_recv_until_close(th_sock_t s, char *buf, size_t bufsz) {
    size_t off = 0;
    for (;;) {
#ifdef _WIN32
        int n = recv(s, buf + off, (int)(bufsz - 1 - off), 0);
#else
        ssize_t n = recv(s, buf + off, bufsz - 1 - off, 0);
#endif
        if (n <= 0)
            break;
        off += (size_t)n;
        if (off >= bufsz - 1)
            break;
    }
    buf[off] = '\0';
    return (int)off;
}

/* One-shot raw HTTP exchange. Returns response length, 0 on connect failure. */
static int th_http_raw(int port, const char *request, char *resp, size_t respsz) {
    th_sock_t s = th_connect(port);
    if (s == TH_SOCK_BAD)
        return 0;
    if (th_send_all(s, request, strlen(request)) != 0) {
        th_sock_close(s);
        return 0;
    }
    int n = th_recv_until_close(s, resp, respsz);
    th_sock_close(s);
    return n;
}

/* Existing route tests focus on endpoint behavior. Add the loopback Host and
 * JSON mutation header the browser supplies. Security tests use th_http_raw()
 * to exercise missing/hostile headers without this convenience layer. */
static char *th_request_with_ui_headers(int port, const char *request) {
    const char *head_end = strstr(request, "\r\n\r\n");
    if (!head_end)
        return strdup(request);

    const char *target = strchr(request, ' ');
    target = target ? target + 1 : NULL;
    bool protected_route =
        target && (strncmp(target, "/api/", 5) == 0 || strncmp(target, "/rpc ", 5) == 0 ||
                   strncmp(target, "/rpc?", 5) == 0);
    bool mutation = strncmp(request, "POST ", 5) == 0;
    bool have_host = strstr(request, "\r\nHost:") != NULL;
    bool have_content_type = strstr(request, "\r\nContent-Type:") != NULL;

    size_t request_len = strlen(request);
    size_t capacity = request_len + 256;
    char *result = malloc(capacity);
    if (!result)
        return NULL;

    size_t head_len = (size_t)(head_end - request);
    memcpy(result, request, head_len);
    size_t pos = head_len;
    if (!have_host)
        pos += (size_t)snprintf(result + pos, capacity - pos, "\r\nHost: 127.0.0.1:%d", port);
    if (protected_route && mutation && !have_content_type) {
        pos += (size_t)snprintf(result + pos, capacity - pos, "\r\nContent-Type: application/json");
    }
    (void)snprintf(result + pos, capacity - pos, "\r\n\r\n%s", head_end + 4);
    return result;
}

static int th_http(int port, const char *request, char *resp, size_t respsz) {
    char *prepared = th_request_with_ui_headers(port, request);
    if (!prepared)
        return 0;
    int result = th_http_raw(port, prepared, resp, respsz);
    free(prepared);
    return result;
}

/* HTTP status code from a raw response ("HTTP/1.1 404 ..."), or -1. */
static int th_status(const char *resp) {
    if (strncmp(resp, "HTTP/1.1 ", 9) != 0)
        return -1;
    return atoi(resp + 9);
}

/* ── Parser unit tests ────────────────────────────────────────── */

TEST(httpd_parse_simple_get) {
    const char *raw = "GET /api/logs?lines=5 HTTP/1.1\r\n"
                      "Host: 127.0.0.1\r\n"
                      "Origin: http://localhost:5173\r\n"
                      "\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 99;
    int rc = cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen);
    ASSERT_EQ(rc, 0);
    ASSERT_STR_EQ(req.method, "GET");
    ASSERT_STR_EQ(req.path, "/api/logs");
    ASSERT_STR_EQ(req.query, "lines=5");
    ASSERT_EQ(req.http_minor, 1);
    ASSERT_STR_EQ(req.origin, "http://localhost:5173");
    ASSERT_EQ((int)clen, 0);
    ASSERT_EQ((int)body_off, (int)strlen(raw));
    PASS();
}

TEST(httpd_parse_security_headers_and_rejects_duplicates) {
    const char *raw = "POST /rpc HTTP/1.1\r\n"
                      "Host: 127.0.0.1:9749\r\n"
                      "Content-Type: application/json\r\n"
                      "Origin: http://127.0.0.1:9749\r\n"
                      "Content-Length: 0\r\n\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, content_length = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &content_length), 0);
    ASSERT_STR_EQ(req.host, "127.0.0.1:9749");
    ASSERT_STR_EQ(req.content_type, "application/json");

    static const char *duplicates[] = {
        "GET / HTTP/1.1\r\nHost: localhost\r\nHost: localhost\r\n\r\n",
        "GET / HTTP/1.1\r\nOrigin: http://localhost\r\nOrigin: http://localhost\r\n\r\n",
        ("POST / HTTP/1.1\r\nContent-Type: application/json\r\nContent-Type: "
         "application/json\r\n\r\n"),
    };
    for (size_t i = 0; i < sizeof(duplicates) / sizeof(duplicates[0]); i++) {
        ASSERT_EQ(cbm_http_parse_head(duplicates[i], strlen(duplicates[i]), &req, &body_off,
                                      &content_length),
                  400);
    }
    PASS();
}

TEST(httpd_parse_post_with_body_offset) {
    const char *raw = "POST /rpc HTTP/1.1\r\n"
                      "Content-Length: 7\r\n"
                      "Content-Type: application/json\r\n"
                      "\r\n"
                      "{\"a\":1}";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    int rc = cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen);
    ASSERT_EQ(rc, 0);
    ASSERT_STR_EQ(req.method, "POST");
    ASSERT_STR_EQ(req.path, "/rpc");
    ASSERT_STR_EQ(req.query, "");
    ASSERT_EQ((int)clen, 7);
    ASSERT_STR_EQ(raw + body_off, "{\"a\":1}");
    PASS();
}

TEST(httpd_parse_origin_case_insensitive) {
    const char *raw = "GET / HTTP/1.1\r\n"
                      "origin: http://127.0.0.1:9749\r\n"
                      "\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 0);
    ASSERT_STR_EQ(req.origin, "http://127.0.0.1:9749");
    PASS();
}

TEST(httpd_parse_rejects_bare_lf) {
    const char *raw = "GET / HTTP/1.1\nHost: x\n\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 400);
    PASS();
}

TEST(httpd_parse_rejects_chunked) {
    const char *raw = "POST /rpc HTTP/1.1\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 411);
    PASS();
}

TEST(httpd_parse_rejects_oversized_content_length) {
    char raw[256];
    snprintf(raw, sizeof(raw), "POST /rpc HTTP/1.1\r\nContent-Length: %d\r\n\r\n",
             CBM_HTTP_MAX_BODY + 1);
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 413);
    PASS();
}

TEST(httpd_parse_rejects_garbage_content_length) {
    const char *raw = "POST /rpc HTTP/1.1\r\nContent-Length: abc\r\n\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 400);

    const char *neg = "POST /rpc HTTP/1.1\r\nContent-Length: -5\r\n\r\n";
    ASSERT_EQ(cbm_http_parse_head(neg, strlen(neg), &req, &body_off, &clen), 400);
    PASS();
}

TEST(httpd_parse_rejects_percent00_in_target) {
    const char *raw = "GET /a%00b HTTP/1.1\r\n\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 400);

    /* %00 hidden in the query string is rejected too */
    const char *q = "GET /ok?x=%00 HTTP/1.1\r\n\r\n";
    ASSERT_EQ(cbm_http_parse_head(q, strlen(q), &req, &body_off, &clen), 400);
    PASS();
}

TEST(httpd_parse_rejects_raw_nul_in_head) {
    char raw[64] = "GET /a";
    size_t len = 6;
    raw[len++] = '\0';
    memcpy(raw + len, " HTTP/1.1\r\n\r\n", 13);
    len += 13;
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, len, &req, &body_off, &clen), 400);
    PASS();
}

TEST(httpd_parse_incomplete_head_needs_more) {
    const char *raw = "GET /api/logs HTTP/1.1\r\nHost: x\r\n"; /* no CRLFCRLF yet */
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), CBM_HTTP_NEED_MORE);
    PASS();
}

TEST(httpd_parse_rejects_missing_version) {
    const char *raw = "GET /\r\n\r\n";
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    ASSERT_EQ(cbm_http_parse_head(raw, strlen(raw), &req, &body_off, &clen), 400);

    const char *v2 = "GET / HTTP/2\r\n\r\n";
    ASSERT_EQ(cbm_http_parse_head(v2, strlen(v2), &req, &body_off, &clen), 400);
    PASS();
}

TEST(httpd_parse_rejects_oversized_head) {
    /* A head that exceeds CBM_HTTP_MAX_HEAD without terminating → 431 */
    size_t big = CBM_HTTP_MAX_HEAD + 1024;
    char *raw = malloc(big);
    ASSERT_NOT_NULL(raw);
    memcpy(raw, "GET / HTTP/1.1\r\nX-Junk: ", 24);
    memset(raw + 24, 'A', big - 24);
    cbm_http_req_t req;
    size_t body_off = 0, clen = 0;
    int rc = cbm_http_parse_head(raw, big, &req, &body_off, &clen);
    free(raw);
    ASSERT_EQ(rc, 431);
    PASS();
}

TEST(httpd_query_param_decode) {
    char buf[64];
    ASSERT_TRUE(cbm_http_query_param("a=hello+world&b=%2Ffoo%2F", "a", buf, (int)sizeof(buf)));
    ASSERT_STR_EQ(buf, "hello world");
    ASSERT_TRUE(cbm_http_query_param("a=hello+world&b=%2Ffoo%2F", "b", buf, (int)sizeof(buf)));
    ASSERT_STR_EQ(buf, "/foo/");
    /* uppercase + lowercase hex */
    ASSERT_TRUE(cbm_http_query_param("p=%2fTmp%2F", "p", buf, (int)sizeof(buf)));
    ASSERT_STR_EQ(buf, "/Tmp/");
    PASS();
}

TEST(httpd_query_param_edge_cases) {
    char buf[8];
    /* missing param */
    ASSERT_FALSE(cbm_http_query_param("a=1", "b", buf, (int)sizeof(buf)));
    /* empty value (current server treats it as absent) */
    ASSERT_FALSE(cbm_http_query_param("a=&b=2", "a", buf, (int)sizeof(buf)));
    /* value too large for buf */
    ASSERT_FALSE(cbm_http_query_param("a=123456789", "a", buf, (int)sizeof(buf)));
    /* decoded NUL rejected */
    char big[32];
    ASSERT_FALSE(cbm_http_query_param("a=x%00y", "a", big, (int)sizeof(big)));
    /* name is a prefix of another name — must not match */
    ASSERT_FALSE(cbm_http_query_param("abc=1", "ab", buf, (int)sizeof(buf)));
    /* truncated percent escape */
    ASSERT_FALSE(cbm_http_query_param("a=%2", "a", buf, (int)sizeof(buf)));
    PASS();
}

TEST(httpd_path_match_matrix) {
    /* exact */
    ASSERT_TRUE(cbm_http_path_match("/", "/"));
    ASSERT_FALSE(cbm_http_path_match("/x", "/"));
    ASSERT_TRUE(cbm_http_path_match("/rpc", "/rpc"));
    ASSERT_FALSE(cbm_http_path_match("/rpc2", "/rpc"));
    /* trailing-* prefix */
    ASSERT_TRUE(cbm_http_path_match("/api/layout", "/api/layout*"));
    ASSERT_TRUE(cbm_http_path_match("/assets/index-abc.js", "/assets/*"));
    ASSERT_FALSE(cbm_http_path_match("/api/browse", "/api/layout*"));
    /* raw path is matched — percent-encoded slash must NOT route */
    ASSERT_FALSE(cbm_http_path_match("/api%2Fbrowse", "/api/browse*"));
    ASSERT_FALSE(cbm_http_path_match("/api%2fbrowse", "/api/browse*"));
    /* CORS origin patterns */
    ASSERT_TRUE(cbm_http_path_match("http://localhost:5173", "http://localhost:*"));
    ASSERT_TRUE(cbm_http_path_match("http://127.0.0.1:9749", "http://127.0.0.1:*"));
    ASSERT_FALSE(cbm_http_path_match("http://evil.com", "http://localhost:*"));
    ASSERT_FALSE(cbm_http_path_match("https://localhost:5173", "http://localhost:*"));
    ASSERT_FALSE(cbm_http_path_match("http://localhost.evil.com:80", "http://localhost:*"));
    PASS();
}

TEST(httpd_resolves_bare_binary_path_from_path) {
#ifdef _WIN32
    PASS();
#else
    char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/cbm_httpd_bin_XXXXXX");
    char *td = cbm_mkdtemp(tmpdir);
    ASSERT_NOT_NULL(td);

    char exe[512];
    snprintf(exe, sizeof(exe), "%s/codebase-memory-mcp", td);
    FILE *f = fopen(exe, "w");
    ASSERT_NOT_NULL(f);
    fputs("#!/bin/sh\nexit 0\n", f);
    fclose(f);
    ASSERT_EQ(chmod(exe, 0755), 0);

    char *old_path = getenv("PATH") ? strdup(getenv("PATH")) : NULL;
    cbm_setenv("PATH", td, 1);

    char resolved[1024];
    ASSERT_TRUE(
        cbm_http_server_resolve_binary_path("codebase-memory-mcp", resolved, sizeof(resolved)));
    ASSERT_STR_EQ(resolved, exe);

    if (old_path) {
        cbm_setenv("PATH", old_path, 1);
        free(old_path);
    } else {
        cbm_unsetenv("PATH");
    }
    PASS();
#endif
}

/* ── Transport integration (listener only) ────────────────────── */

TEST(httpd_listen_ephemeral_port) {
    cbm_httpd_t *d = cbm_httpd_listen(0);
    ASSERT_NOT_NULL(d);
    int port = cbm_httpd_port(d);
    ASSERT_GT(port, 0);
    /* accept with a short timeout and no client → NULL, promptly */
    cbm_http_conn_t *c = cbm_httpd_accept(d, 50);
    ASSERT_NULL(c);
    ASSERT_TRUE(cbm_httpd_close(d));
    PASS();
}

TEST(httpd_listen_port_collision_returns_null) {
    cbm_httpd_t *d1 = cbm_httpd_listen(0);
    ASSERT_NOT_NULL(d1);
    cbm_httpd_t *d2 = cbm_httpd_listen(cbm_httpd_port(d1));
    ASSERT_NULL(d2);
    ASSERT_TRUE(cbm_httpd_close(d1));
    PASS();
}

TEST(httpd_close_refuses_while_connection_owns_listener) {
    cbm_httpd_t *listener = cbm_httpd_listen(0);
    ASSERT_NOT_NULL(listener);
    th_sock_t client = th_connect(cbm_httpd_port(listener));
    ASSERT_TRUE(client != TH_SOCK_BAD);
    cbm_http_conn_t *connection = cbm_httpd_accept(listener, 1000);
    ASSERT_NOT_NULL(connection);

    ASSERT_FALSE(cbm_httpd_close(listener));
    cbm_httpd_conn_close(connection);
    th_sock_close(client);
    ASSERT_TRUE(cbm_httpd_close(listener));
    PASS();
}

/* ── Full UI server integration ───────────────────────────────── */

typedef struct {
    cbm_http_server_t *srv;
    cbm_thread_t tid;
} th_server_t;

static void *th_server_thread(void *arg) {
    cbm_http_server_run((cbm_http_server_t *)arg);
    return NULL;
}

static int th_server_thread_start(cbm_thread_t *thread, cbm_http_server_t *server) {
    if (!cbm_http_server_schedule_run(server))
        return -1;
    int rc = cbm_thread_create(thread, 0, th_server_thread, server);
    if (rc != 0 && !cbm_http_server_cancel_scheduled_run(server))
        return -1;
    return rc;
}

static int th_server_start(th_server_t *ts) {
    ts->srv = cbm_http_server_new(0);
    if (!ts->srv)
        return -1;
    if (th_server_thread_start(&ts->tid, ts->srv) != 0) {
        (void)cbm_http_server_free(ts->srv);
        return -1;
    }
    return 0;
}

static int th_server_start_with_watcher(th_server_t *ts, cbm_watcher_t *watcher) {
    ts->srv = cbm_http_server_new(0);
    if (!ts->srv)
        return -1;
    cbm_http_server_set_watcher(ts->srv, watcher);
    if (th_server_thread_start(&ts->tid, ts->srv) != 0) {
        (void)cbm_http_server_free(ts->srv);
        return -1;
    }
    return 0;
}

typedef struct {
    atomic_int calls;
    char root_path[512];
    char project_name[256];
} th_ui_index_executor_t;

typedef struct {
    atomic_int calls;
    atomic_int release;
} th_ui_blocking_index_executor_t;

static int th_ui_index_executor(void *opaque, const char *root_path, const char *project_name) {
    th_ui_index_executor_t *executor = opaque;
    snprintf(executor->root_path, sizeof(executor->root_path), "%s", root_path);
    snprintf(executor->project_name, sizeof(executor->project_name), "%s",
             project_name ? project_name : "");
    atomic_fetch_add(&executor->calls, 1);
    return 0;
}

static int th_ui_blocking_index_executor(void *opaque, const char *root_path,
                                         const char *project_name) {
    (void)root_path;
    (void)project_name;
    th_ui_blocking_index_executor_t *executor = opaque;
    atomic_fetch_add(&executor->calls, 1);
    while (!atomic_load(&executor->release))
        cbm_usleep(1000);
    return 0;
}

static bool th_wait_atomic_int(atomic_int *value, int expected, uint32_t timeout_ms) {
    uint64_t deadline = cbm_now_ms() + timeout_ms;
    while (cbm_now_ms() < deadline) {
        if (atomic_load(value) == expected) {
            return true;
        }
        cbm_usleep(1000);
    }
    return atomic_load(value) == expected;
}

static bool th_wait_http_server_activity(cbm_http_server_t *server, cbm_httpd_activity_t expected,
                                         uint32_t timeout_ms) {
    uint64_t deadline = cbm_now_ms() + timeout_ms;
    while (cbm_now_ms() < deadline) {
        if (cbm_http_server_activity_for_test(server) == expected)
            return true;
        cbm_usleep(1000);
    }
    return cbm_http_server_activity_for_test(server) == expected;
}

static bool th_wait_httpd_activity(cbm_httpd_t *listener, cbm_httpd_activity_t expected,
                                   uint32_t timeout_ms) {
    uint64_t deadline = cbm_now_ms() + timeout_ms;
    while (cbm_now_ms() < deadline) {
        if (cbm_httpd_activity_for_test(listener) == expected)
            return true;
        cbm_usleep(1000);
    }
    return cbm_httpd_activity_for_test(listener) == expected;
}

typedef struct {
    atomic_int begin_calls;
    atomic_int end_calls;
    bool allow;
    char begin_project[256];
    char end_project[256];
} th_ui_mutation_guard_t;

static void th_ui_mutation_guard_init(th_ui_mutation_guard_t *guard, bool allow) {
    memset(guard, 0, sizeof(*guard));
    atomic_init(&guard->begin_calls, 0);
    atomic_init(&guard->end_calls, 0);
    guard->allow = allow;
}

static bool th_ui_mutation_begin(void *opaque, const char *project) {
    th_ui_mutation_guard_t *guard = opaque;
    snprintf(guard->begin_project, sizeof(guard->begin_project), "%s", project ? project : "");
    atomic_fetch_add(&guard->begin_calls, 1);
    return guard->allow;
}

static void th_ui_mutation_end(void *opaque, const char *project) {
    th_ui_mutation_guard_t *guard = opaque;
    snprintf(guard->end_project, sizeof(guard->end_project), "%s", project ? project : "");
    atomic_fetch_add(&guard->end_calls, 1);
}

static int th_server_start_with_mutation_guard(th_server_t *ts, cbm_watcher_t *watcher,
                                               th_ui_mutation_guard_t *guard) {
    ts->srv = cbm_http_server_new(0);
    if (!ts->srv)
        return -1;
    if (watcher)
        cbm_http_server_set_watcher(ts->srv, watcher);
    cbm_http_server_set_project_mutation_guard(ts->srv, th_ui_mutation_begin, th_ui_mutation_end,
                                               guard);
    if (th_server_thread_start(&ts->tid, ts->srv) != 0) {
        (void)cbm_http_server_free(ts->srv);
        return -1;
    }
    return 0;
}

static void th_server_stop(th_server_t *ts) {
    cbm_http_server_stop(ts->srv);
    (void)cbm_thread_join(&ts->tid);
    (void)cbm_http_server_free(ts->srv);
}

typedef struct {
    char tmpdir[256];
    char cache_dir[512];
    char root_dir[512];
    char *saved_cache_dir;
    cbm_store_t *store;
    cbm_watcher_t *watcher;
} ui_delete_fixture_t;

static int ui_delete_fixture_init(ui_delete_fixture_t *fx) {
    memset(fx, 0, sizeof(*fx));
    char *tmp = th_mktempdir("cbm_httpd_delete");
    if (!tmp)
        return -1;
    snprintf(fx->tmpdir, sizeof(fx->tmpdir), "%s", tmp);
    snprintf(fx->cache_dir, sizeof(fx->cache_dir), "%s/cache", fx->tmpdir);
    snprintf(fx->root_dir, sizeof(fx->root_dir), "%s/root", fx->tmpdir);

    const char *saved = getenv("CBM_CACHE_DIR");
    fx->saved_cache_dir = saved ? strdup(saved) : NULL;
    if (th_mkdir_p(fx->cache_dir) != 0 || th_mkdir_p(fx->root_dir) != 0) {
        return -1;
    }
    cbm_setenv("CBM_CACHE_DIR", fx->cache_dir, 1);

    fx->store = cbm_store_open_memory();
    fx->watcher = cbm_watcher_new(fx->store, NULL, NULL);
    return fx->store && fx->watcher ? 0 : -1;
}

static void ui_delete_fixture_cleanup(ui_delete_fixture_t *fx) {
    if (fx->watcher)
        cbm_watcher_free(fx->watcher);
    if (fx->store)
        cbm_store_close(fx->store);
    if (fx->saved_cache_dir) {
        cbm_setenv("CBM_CACHE_DIR", fx->saved_cache_dir, 1);
        free(fx->saved_cache_dir);
    } else {
        cbm_unsetenv("CBM_CACHE_DIR");
    }
    th_cleanup(fx->tmpdir);
}

static void ui_delete_db_path(const ui_delete_fixture_t *fx, const char *project, char *out,
                              size_t outsz) {
    snprintf(out, outsz, "%s/%s.db", fx->cache_dir, project);
}

static int ui_delete_make_db_file(const ui_delete_fixture_t *fx, const char *project) {
    char path[1024];
    ui_delete_db_path(fx, project, path, sizeof(path));
    return th_write_file(path, "test db");
}

static int ui_delete_make_sidecars(const ui_delete_fixture_t *fx, const char *project) {
    char path[1024];
    ui_delete_db_path(fx, project, path, sizeof(path));
    char wal[1040], shm[1040];
    snprintf(wal, sizeof(wal), "%s-wal", path);
    snprintf(shm, sizeof(shm), "%s-shm", path);
    return th_write_file(wal, "wal") == 0 && th_write_file(shm, "shm") == 0 ? 0 : -1;
}

static int ui_delete_request(th_server_t *ts, const char *target, char *resp, size_t respsz) {
    char req[512];
    snprintf(req, sizeof(req), "DELETE %s HTTP/1.1\r\n\r\n", target);
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

static int ui_adr_post_request(th_server_t *ts, const char *project, const char *content,
                               char *resp, size_t respsz) {
    char body[2048];
    int body_len =
        snprintf(body, sizeof(body), "{\"project\":\"%s\",\"content\":\"%s\"}", project, content);
    if (body_len < 0 || (size_t)body_len >= sizeof(body))
        return 0;

    char req[2304];
    int req_len = snprintf(req, sizeof(req),
                           "POST /api/adr HTTP/1.1\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n\r\n%s",
                           body_len, body);
    if (req_len < 0 || (size_t)req_len >= sizeof(req))
        return 0;
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

static int ui_adr_get_request(th_server_t *ts, const char *project, char *resp, size_t respsz) {
    char req[512];
    int req_len = snprintf(req, sizeof(req), "GET /api/adr?project=%s HTTP/1.1\r\n\r\n", project);
    if (req_len < 0 || (size_t)req_len >= sizeof(req))
        return 0;
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

static int ui_adr_seed(const ui_delete_fixture_t *fx, const char *project, const char *content) {
    char db_path[1024];
    ui_delete_db_path(fx, project, db_path, sizeof(db_path));
    cbm_store_t *store = cbm_store_open_path(db_path);
    if (!store)
        return CBM_STORE_ERR;
    int rc = cbm_store_adr_store(store, project, content);
    cbm_store_close(store);
    return rc;
}

static bool ui_adr_equals(const ui_delete_fixture_t *fx, const char *project,
                          const char *expected) {
    char db_path[1024];
    ui_delete_db_path(fx, project, db_path, sizeof(db_path));
    cbm_store_t *store = cbm_store_open_path_query(db_path);
    if (!store)
        return false;

    cbm_adr_t adr = {0};
    int rc = cbm_store_adr_get(store, project, &adr);
    bool equal = rc == CBM_STORE_OK && adr.content && strcmp(adr.content, expected) == 0;
    if (rc == CBM_STORE_OK)
        cbm_store_adr_free(&adr);
    cbm_store_close(store);
    return equal;
}

TEST(ui_server_readiness_proof_is_exact_and_generation_bound) {
    static const char challenge[] =
        "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f";
    static const char expected[] =
        "62215de7bddcea7e2c4047ff6bb94f8d18262fc8b3f3648134bb7d44158ff84d";
    uint8_t secret[CBM_SHA256_DIGEST_LEN];
    for (size_t index = 0; index < sizeof(secret); index++) {
        secret[index] = (uint8_t)index;
    }

    th_server_t without_secret;
    ASSERT_EQ(th_server_start(&without_secret), 0);
    char request[512];
    ASSERT_GT(snprintf(request, sizeof(request),
                       "GET /__cbm/ui-readiness?challenge=%s HTTP/1.1\r\n\r\n", challenge),
              0);
    char response[4096];
    int response_length =
        th_http(cbm_http_server_port(without_secret.srv), request, response, sizeof(response));
    int missing_secret_status = response_length > 0 ? th_status(response) : -1;
    th_server_stop(&without_secret);

    th_server_t server = {0};
    server.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(server.srv);
    cbm_http_server_set_readiness_secret(server.srv, secret);
    ASSERT_EQ(th_server_thread_start(&server.tid, server.srv), 0);
    int port = cbm_http_server_port(server.srv);
    response_length = th_http(port, request, response, sizeof(response));
    char *body = response_length > 0 ? strstr(response, "\r\n\r\n") : NULL;
    body = body ? body + 4 : NULL;
    bool exact_proof = response_length > 0 && th_status(response) == 200 && body &&
                       strcmp(body, expected) == 0 &&
                       strstr(response, "Content-Type: text/plain; charset=utf-8") != NULL &&
                       strstr(response, "Cache-Control: no-store") != NULL;

    ASSERT_GT(snprintf(request, sizeof(request),
                       "GET /__cbm/ui-readiness?challenge="
                       "202122232425262728292A2b2c2d2e2f303132333435363738393a3b3c3d3e3f "
                       "HTTP/1.1\r\n\r\n"),
              0);
    response_length = th_http(port, request, response, sizeof(response));
    int uppercase_status = response_length > 0 ? th_status(response) : -1;

    ASSERT_GT(snprintf(request, sizeof(request),
                       "GET /__cbm/ui-readiness?challenge=%s&extra=1 HTTP/1.1\r\n\r\n", challenge),
              0);
    response_length = th_http(port, request, response, sizeof(response));
    int extra_parameter_status = response_length > 0 ? th_status(response) : -1;

    response_length =
        th_http(port, "GET /__cbm/ui-readiness HTTP/1.1\r\n\r\n", response, sizeof(response));
    int missing_challenge_status = response_length > 0 ? th_status(response) : -1;
    th_server_stop(&server);

    ASSERT_EQ(missing_secret_status, 503);
    ASSERT_TRUE(exact_proof);
    ASSERT_EQ(uppercase_status, 400);
    ASSERT_EQ(extra_parameter_status, 400);
    ASSERT_EQ(missing_challenge_status, 400);
    PASS();
}

TEST(ui_server_unknown_path_404) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    int port = cbm_http_server_port(ts.srv);

    char resp[4096];
    int n = th_http(port, "GET /definitely/not/here HTTP/1.1\r\n\r\n", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 404);
    /* every response is explicit-length + close */
    ASSERT_NOT_NULL(strstr(resp, "Connection: close"));
    ASSERT_NOT_NULL(strstr(resp, "Content-Length:"));

    th_server_stop(&ts);
    PASS();
}

/* Security regression: process IDs are not cancellation capabilities. The UI
 * must never expose an endpoint that accepts an arbitrary PID and reaches an
 * OS process-termination API (the former Windows path accepted every PID but
 * self). Daemon-owned jobs are cancelled by opaque, owner-bound subscription
 * handles instead, so this legacy route must be completely absent. */
TEST(ui_server_process_kill_route_is_unavailable) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    int port = cbm_http_server_port(ts.srv);

    static const char body[] = "{\"pid\":2147483646}";
    char request[512];
    snprintf(request, sizeof(request),
             "POST /api/process-kill HTTP/1.1\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             strlen(body), body);
    char resp[4096];
    int n = th_http(port, request, resp, sizeof(resp));

    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 404);
    ASSERT_NULL(strstr(resp, "\"killed\""));
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_routes_indexing_through_joinable_daemon_executor) {
    char *root = th_mktempdir("cbm_httpd_daemon_index");
    ASSERT_NOT_NULL(root);
    th_ui_index_executor_t executor = {0};
    atomic_init(&executor.calls, 0);
    th_server_t ts;
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    char body[1024];
    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    char request[1400];
    snprintf(request, sizeof(request),
             "POST /api/index HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             strlen(body), body);
    char response[4096];
    int response_length =
        th_http(cbm_http_server_port(ts.srv), request, response, sizeof(response));
    bool called = th_wait_atomic_int(&executor.calls, 1, 2000);

    th_server_stop(&ts);
    ASSERT_GT(response_length, 0);
    ASSERT_EQ(th_status(response), 202);
    ASSERT_TRUE(called);
    ASSERT_STR_EQ(executor.root_path, root);
    ASSERT(executor.project_name[0] != '\0');
    th_cleanup(root);
    PASS();
}

TEST(ui_server_free_never_joins_active_index_worker) {
    char *root = th_mktempdir("cbm_httpd_active_index");
    ASSERT_NOT_NULL(root);
    th_ui_blocking_index_executor_t executor = {0};
    atomic_init(&executor.calls, 0);
    atomic_init(&executor.release, 0);

    th_server_t ts;
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_blocking_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    char body[1024];
    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    char request[1400];
    snprintf(request, sizeof(request),
             "POST /api/index HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             strlen(body), body);
    char response[4096];
    ASSERT_GT(th_http(cbm_http_server_port(ts.srv), request, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    ASSERT_TRUE(th_wait_atomic_int(&executor.calls, 1, 2000));

    cbm_http_server_stop(ts.srv);
    ASSERT_EQ(cbm_thread_join(&ts.tid), 0);
    uint64_t started = cbm_now_ms();
    ASSERT_FALSE(cbm_http_server_free(ts.srv));
    ASSERT_LT(cbm_now_ms() - started, 1000);

    atomic_store(&executor.release, 1);
    bool freed = false;
    uint64_t deadline = cbm_now_ms() + 2000;
    while (!freed && cbm_now_ms() < deadline) {
        freed = cbm_http_server_free(ts.srv);
        if (!freed)
            cbm_usleep(1000);
    }
    ASSERT_TRUE(freed);
    th_cleanup(root);
    PASS();
}

TEST(ui_server_root_without_embedded_assets_is_not_found) {
    /* The frontend is linked into the image, so its availability is decided at
     * build time, not warmed at runtime: a binary built without --with-ui has
     * no index.html and never will. That is a permanent 404, NOT a retryable
     * 503 -- promising a retry for a condition that cannot change would make
     * every client poll forever. */
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char resp[4096];
    int n = th_http(cbm_http_server_port(ts.srv), "GET / HTTP/1.1\r\n\r\n", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 404);
    ASSERT_NOT_NULL(strstr(resp, "no frontend embedded"));
    ASSERT_NULL(strstr(resp, "Retry-After"));
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_same_origin_request_is_allowed) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    int port = cbm_http_server_port(ts.srv);
    char req[512];
    snprintf(req, sizeof(req),
             "OPTIONS /rpc HTTP/1.1\r\n"
             "Host: 127.0.0.1:%d\r\n"
             "Origin: http://127.0.0.1:%d\r\n\r\n",
             port, port);
    char resp[4096];
    int n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 204);
    char expected_origin[128];
    snprintf(expected_origin, sizeof(expected_origin),
             "Access-Control-Allow-Origin: http://127.0.0.1:%d", port);
    ASSERT_NOT_NULL(strstr(resp, expected_origin));
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_rejects_foreign_and_null_origins) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    int port = cbm_http_server_port(ts.srv);
    char resp[4096];
    char req[768];
    snprintf(req, sizeof(req),
             "OPTIONS /rpc HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
             "Origin: http://evil.example.com\r\n\r\n",
             port);
    int n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);
    ASSERT_NULL(strstr(resp, "Access-Control-Allow-Origin"));

    snprintf(req, sizeof(req), "GET / HTTP/1.1\r\nHost: localhost:%d\r\nOrigin: null\r\n\r\n",
             port);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);

    snprintf(req, sizeof(req),
             "GET / HTTP/1.1\r\nHost: localhost:%d\r\n"
             "Origin: http://127.0.0.1:%d\r\n\r\n",
             port, port);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);

    static const char body[] = "{\"project\":\"victim\",\"content\":\"poison\"}";
    snprintf(req, sizeof(req),
             "POST /api/adr HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
             "Origin: http://evil.example.com\r\nContent-Type: text/plain\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             port, strlen(body), body);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_mutations_require_json_content_type) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    int port = cbm_http_server_port(ts.srv);
    const char *body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\","
                       "\"params\":{\"name\":\"list_projects\",\"arguments\":{}}}";
    char req[1024];
    char resp[8192];

    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             port, strlen(body), body);
    int n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 415);

    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             port, strlen(body), body);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 415);

    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n"
             "Content-Type: application/json; charset=utf-8\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             port, strlen(body), body);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);

    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_rpc_allows_only_ui_read_tools) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    const char *body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\","
                       "\"params\":{\"name\":\"list_projects\",\"arguments\":{}}}";
    char req[1024];
    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n\r\n%s",
             (int)strlen(body), body);
    char resp[8192];
    int n = th_http(cbm_http_server_port(ts.srv), req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "\"jsonrpc\""));

    static const char *blocked_tools[] = {"delete_project", "manage_adr", "ingest_traces",
                                          "index_repository"};
    for (size_t i = 0; i < sizeof(blocked_tools) / sizeof(blocked_tools[0]); i++) {
        char blocked_body[512];
        snprintf(blocked_body, sizeof(blocked_body),
                 "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\","
                 "\"params\":{\"name\":\"%s\",\"arguments\":{}}}",
                 blocked_tools[i]);
        snprintf(req, sizeof(req),
                 "POST /rpc HTTP/1.1\r\nContent-Type: application/json\r\n"
                 "Content-Length: %zu\r\n\r\n%s",
                 strlen(blocked_body), blocked_body);
        n = th_http(cbm_http_server_port(ts.srv), req, resp, sizeof(resp));
        ASSERT_GT(n, 0);
        ASSERT_EQ(th_status(resp), 403);
    }

    const char *initialize = "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"initialize\","
                             "\"params\":{}}";
    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             strlen(initialize), initialize);
    n = th_http(cbm_http_server_port(ts.srv), req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);

    const char *ambiguous = "{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"tools/call\","
                            "\"params\":{\"name\":\"list_projects\",\"name\":\"delete_project\","
                            "\"arguments\":{}}}";
    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             strlen(ambiguous), ambiguous);
    n = th_http(cbm_http_server_port(ts.srv), req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);

    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_oversized_body_rejected) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char req[256];
    snprintf(req, sizeof(req), "POST /rpc HTTP/1.1\r\nContent-Length: %d\r\n\r\n",
             CBM_HTTP_MAX_BODY + 1);
    char resp[4096];
    int n = th_http(cbm_http_server_port(ts.srv), req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 413);
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_encoded_slash_not_routed) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char resp[4096];
    int n = th_http(cbm_http_server_port(ts.srv), "GET /api%2Fbrowse?path=/tmp HTTP/1.1\r\n\r\n",
                    resp, sizeof(resp));
    ASSERT_GT(n, 0);
    /* must fall through to 404 — NOT the browse handler */
    ASSERT_EQ(th_status(resp), 404);
    ASSERT_NULL(strstr(resp, "\"dirs\""));
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_nul_in_target_rejected) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char resp[4096];
    int n =
        th_http(cbm_http_server_port(ts.srv), "GET /a%00b HTTP/1.1\r\n\r\n", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 400);
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_browse_traversal_probe) {
    /* Percent-encoded traversal in the QUERY VALUE is decoded (that is the
     * documented contract) and then hits the same directory checks as any
     * other path. The server must answer with a well-formed JSON error or
     * listing — never crash, never echo raw unescaped input. */
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char resp[65536];
    int n = th_http(cbm_http_server_port(ts.srv),
                    "GET /api/browse?path=%2Ftmp%2F..%2F..%2Fprivate HTTP/1.1\r\n\r\n", resp,
                    sizeof(resp));
    ASSERT_GT(n, 0);
    int st = th_status(resp);
    ASSERT_TRUE(st == 200 || st == 400 || st == 403);
    const char *json = strstr(resp, "\r\n\r\n");
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(json[4], '{');
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_adr_mutation_guard_busy_preserves_existing_adr) {
    static const char *project = "ui-guard-adr-busy";
    static const char *original = "original architecture";
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(ui_adr_seed(&fx, project, original), CBM_STORE_OK);

    th_ui_mutation_guard_t guard;
    th_ui_mutation_guard_init(&guard, false);
    th_server_t ts;
    ASSERT_EQ(th_server_start_with_mutation_guard(&ts, NULL, &guard), 0);

    char resp[4096];
    int n = ui_adr_post_request(&ts, project, "replacement architecture", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 423);
    ASSERT_NOT_NULL(strstr(resp, "project is busy; retry after indexing"));
    ASSERT_EQ(atomic_load(&guard.begin_calls), 1);
    ASSERT_EQ(atomic_load(&guard.end_calls), 0);
    ASSERT_STR_EQ(guard.begin_project, project);

    /* A rejected mutation must leave the published DB queryable and unchanged.
     * GET is a query operation and therefore must not enter the mutation guard. */
    n = ui_adr_get_request(&ts, project, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "\"has_adr\":true"));
    ASSERT_NOT_NULL(strstr(resp, original));
    ASSERT_EQ(atomic_load(&guard.begin_calls), 1);
    ASSERT_EQ(atomic_load(&guard.end_calls), 0);
    ASSERT_TRUE(ui_adr_equals(&fx, project, original));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_adr_mutation_guard_balances_success) {
    static const char *project = "ui-guard-adr-success";
    static const char *content = "coordinated architecture";
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);

    th_ui_mutation_guard_t guard;
    th_ui_mutation_guard_init(&guard, true);
    th_server_t ts;
    ASSERT_EQ(th_server_start_with_mutation_guard(&ts, NULL, &guard), 0);

    char resp[4096];
    int n = ui_adr_post_request(&ts, project, content, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "{\"saved\":true}"));
    ASSERT_EQ(atomic_load(&guard.begin_calls), 1);
    ASSERT_EQ(atomic_load(&guard.end_calls), 1);
    ASSERT_STR_EQ(guard.begin_project, project);
    ASSERT_STR_EQ(guard.end_project, project);
    ASSERT_TRUE(ui_adr_equals(&fx, project, content));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_mutation_guard_busy_preserves_project) {
    static const char *project = "ui-guard-delete-busy";
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(ui_delete_make_db_file(&fx, project), 0);
    ASSERT_EQ(ui_delete_make_sidecars(&fx, project), 0);
    cbm_watcher_watch(fx.watcher, project, fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_ui_mutation_guard_t guard;
    th_ui_mutation_guard_init(&guard, false);
    th_server_t ts;
    ASSERT_EQ(th_server_start_with_mutation_guard(&ts, fx.watcher, &guard), 0);

    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=ui-guard-delete-busy", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 423);
    ASSERT_NOT_NULL(strstr(resp, "project is busy; retry after indexing"));
    ASSERT_EQ(atomic_load(&guard.begin_calls), 1);
    ASSERT_EQ(atomic_load(&guard.end_calls), 0);
    ASSERT_STR_EQ(guard.begin_project, project);

    char db_path[1024], wal_path[1040], shm_path[1040];
    ui_delete_db_path(&fx, project, db_path, sizeof(db_path));
    snprintf(wal_path, sizeof(wal_path), "%s-wal", db_path);
    snprintf(shm_path, sizeof(shm_path), "%s-shm", db_path);
    ASSERT_TRUE(cbm_file_exists(db_path));
    ASSERT_TRUE(cbm_file_exists(wal_path));
    ASSERT_TRUE(cbm_file_exists(shm_path));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_mutation_guard_balances_success) {
    static const char *project = "ui-guard-delete-success";
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(ui_delete_make_db_file(&fx, project), 0);
    cbm_watcher_watch(fx.watcher, project, fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_ui_mutation_guard_t guard;
    th_ui_mutation_guard_init(&guard, true);
    th_server_t ts;
    ASSERT_EQ(th_server_start_with_mutation_guard(&ts, fx.watcher, &guard), 0);

    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=ui-guard-delete-success", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "{\"deleted\":true}"));
    ASSERT_EQ(atomic_load(&guard.begin_calls), 1);
    ASSERT_EQ(atomic_load(&guard.end_calls), 1);
    ASSERT_STR_EQ(guard.begin_project, project);
    ASSERT_STR_EQ(guard.end_project, project);

    char db_path[1024];
    ui_delete_db_path(&fx, project, db_path, sizeof(db_path));
    ASSERT_FALSE(cbm_file_exists(db_path));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 0);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_project_unwatches_after_delete) {
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(ui_delete_make_db_file(&fx, "ui-delete-watch"), 0);
    ASSERT_EQ(ui_delete_make_sidecars(&fx, "ui-delete-watch"), 0);
    cbm_watcher_watch(fx.watcher, "ui-delete-watch", fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_t ts;
    ASSERT_EQ(th_server_start_with_watcher(&ts, fx.watcher), 0);
    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=ui-delete-watch", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "{\"deleted\":true}"));

    char db[1024], wal[1040], shm[1040];
    ui_delete_db_path(&fx, "ui-delete-watch", db, sizeof(db));
    snprintf(wal, sizeof(wal), "%s-wal", db);
    snprintf(shm, sizeof(shm), "%s-shm", db);
    ASSERT_FALSE(cbm_file_exists(db));
    ASSERT_FALSE(cbm_file_exists(wal));
    ASSERT_FALSE(cbm_file_exists(shm));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 0);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_project_unwatches_missing_db) {
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    cbm_watcher_watch(fx.watcher, "ui-delete-missing", fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_t ts;
    ASSERT_EQ(th_server_start_with_watcher(&ts, fx.watcher), 0);
    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=ui-delete-missing", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 404);
    ASSERT_NOT_NULL(strstr(resp, "{\"error\":\"project not found\"}"));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 0);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_project_no_watcher_still_deletes) {
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(ui_delete_make_db_file(&fx, "ui-delete-no-watcher"), 0);

    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=ui-delete-no-watcher", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 200);

    char db[1024];
    ui_delete_db_path(&fx, "ui-delete-no-watcher", db, sizeof(db));
    ASSERT_FALSE(cbm_file_exists(db));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_project_missing_name_keeps_watch) {
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    cbm_watcher_watch(fx.watcher, "ui-delete-still-watched", fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_t ts;
    ASSERT_EQ(th_server_start_with_watcher(&ts, fx.watcher), 0);
    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 400);
    ASSERT_NOT_NULL(strstr(resp, "{\"error\":\"missing name\"}"));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_project_invalid_name_keeps_watch) {
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    cbm_watcher_watch(fx.watcher, "bad/name", fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_t ts;
    ASSERT_EQ(th_server_start_with_watcher(&ts, fx.watcher), 0);
    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=bad%2Fname", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 404);
    ASSERT_NOT_NULL(strstr(resp, "{\"error\":\"project not found\"}"));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_delete_project_unlink_failure_keeps_watch) {
    ui_delete_fixture_t fx;
    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    char db[1024];
    ui_delete_db_path(&fx, "ui-delete-unlink-fails", db, sizeof(db));
    ASSERT_EQ(th_mkdir_p(db), 0);
    cbm_watcher_watch(fx.watcher, "ui-delete-unlink-fails", fx.root_dir);
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_t ts;
    ASSERT_EQ(th_server_start_with_watcher(&ts, fx.watcher), 0);
    char resp[4096];
    int n = ui_delete_request(&ts, "/api/project?name=ui-delete-unlink-fails", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 500);
    ASSERT_NOT_NULL(strstr(resp, "{\"error\":\"failed to delete\"}"));
    ASSERT_TRUE(cbm_file_exists(db));
    ASSERT_EQ(cbm_watcher_watch_count(fx.watcher), 1);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_server_ui_config_detects_zh_accept_language) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);

    char resp[4096];
    int n = th_http(cbm_http_server_port(ts.srv),
                    "GET /api/ui-config HTTP/1.1\r\n"
                    "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n"
                    "\r\n",
                    resp, sizeof(resp));
    ASSERT_TRUE(n > 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "\"lang\":\"zh\""));

    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_ui_config_prefers_config_lang) {
    char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/cbm_httpd_cfg_XXXXXX");
    char *td = cbm_mkdtemp(tmpdir);
    ASSERT_NOT_NULL(td);

    char *old_home = getenv("HOME") ? strdup(getenv("HOME")) : NULL;
    cbm_setenv("HOME", td, 1);

    char cache_dir[1024];
    snprintf(cache_dir, sizeof(cache_dir), "%s", cbm_resolve_cache_dir());
    cbm_config_t *cfg = cbm_config_open(cache_dir);
    ASSERT_NOT_NULL(cfg);
    ASSERT_EQ(cbm_config_set(cfg, CBM_CONFIG_UI_LANG, "zh"), 0);
    cbm_config_close(cfg);

    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);

    char resp[4096];
    int n = th_http(cbm_http_server_port(ts.srv),
                    "GET /api/ui-config HTTP/1.1\r\n"
                    "Accept-Language: en-US,en;q=0.9\r\n"
                    "\r\n",
                    resp, sizeof(resp));
    ASSERT_TRUE(n > 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "\"lang\":\"zh\""));

    th_server_stop(&ts);
    if (old_home) {
        cbm_setenv("HOME", old_home, 1);
        free(old_home);
    }
    PASS();
}

TEST(ui_server_slow_request_hits_deadline) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    /* Shorten the deadline so the test is fast */
    cbm_http_server_set_recv_deadline_ms(ts.srv, 300);
    int port = cbm_http_server_port(ts.srv);

    th_sock_t s = th_connect(port);
    ASSERT_TRUE(s != TH_SOCK_BAD);
    ASSERT_EQ(th_send_all(s, "GET /api", 8), 0); /* partial request, then stall */
    char resp[1024];
    int n = th_recv_until_close(s, resp, sizeof(resp)); /* server must give up */
    th_sock_close(s);
    /* Either a 408 or a bare close is acceptable — the loop must move on */
    if (n > 0) {
        ASSERT_EQ(th_status(resp), 408);
    }

    /* …and the server must still answer the next request */
    char resp2[4096];
    int n2 = th_http(port, "GET /definitely/not/here HTTP/1.1\r\n\r\n", resp2, sizeof(resp2));
    ASSERT_GT(n2, 0);
    ASSERT_EQ(th_status(resp2), 404);

    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_access_log_redacts_query) {
    httpd_log_buf[0] = '\0';
    CBMLogLevel prev_level = cbm_log_get_level();
    cbm_log_set_level(CBM_LOG_DEBUG);
    cbm_log_set_format(CBM_LOG_FORMAT_TEXT);
    cbm_log_set_sink_ex(httpd_capture_log, CBM_LOG_SINK_REPLACE);

    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    char resp[4096];
    int n = th_http(cbm_http_server_port(ts.srv),
                    "GET /definitely/not/here?token=secret HTTP/1.1\r\n\r\n", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 404);
    th_server_stop(&ts);

    cbm_log_set_sink(NULL);
    cbm_log_set_level(prev_level);

    ASSERT_NOT_NULL(strstr(httpd_log_buf, "msg=http.request"));
    ASSERT_NOT_NULL(strstr(httpd_log_buf, "component=graph_ui"));
    ASSERT_NOT_NULL(strstr(httpd_log_buf, "method=GET"));
    ASSERT_NOT_NULL(strstr(httpd_log_buf, "path=/definitely/not/here"));
    ASSERT_NOT_NULL(strstr(httpd_log_buf, "status=404"));
    ASSERT_NULL(strstr(httpd_log_buf, "token"));
    ASSERT_NULL(strstr(httpd_log_buf, "secret"));
    PASS();
}

TEST(ui_server_stop_joins_cleanly) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    /* no requests at all — stop must unblock the accept wait promptly */
    th_server_stop(&ts);
    PASS();
}

TEST(ui_server_free_refuses_active_loop) {
    cbm_http_server_t *server = cbm_http_server_new(0);
    ASSERT_NOT_NULL(server);

    cbm_thread_t thread;
    ASSERT_EQ(th_server_thread_start(&thread, server), 0);
    char response[512];
    ASSERT_GT(
        th_http(cbm_http_server_port(server), "GET / HTTP/1.1\r\n\r\n", response, sizeof(response)),
        0);
    ASSERT_FALSE(cbm_http_server_free(server));
    cbm_http_server_stop(server);
    ASSERT_EQ(cbm_thread_join(&thread), 0);
    ASSERT_TRUE(cbm_http_server_free(server));
    PASS();
}

TEST(ui_server_free_refuses_scheduled_run_before_child_starts) {
    cbm_http_server_t *server = cbm_http_server_new(0);
    ASSERT_NOT_NULL(server);

    ASSERT_TRUE(cbm_http_server_schedule_run(server));
    ASSERT_FALSE(cbm_http_server_free(server));
    ASSERT_TRUE(cbm_http_server_cancel_scheduled_run(server));
    ASSERT_TRUE(cbm_http_server_free(server));
    PASS();
}

TEST(daemon_host_http_thread_create_failure_cancels_scheduled_run) {
    ASSERT_TRUE(cbm_daemon_host_http_thread_create_failure_lifecycle_for_test());
    PASS();
}

typedef struct {
    th_sock_t socket;
    atomic_int *operation_finished;
    atomic_int stop_finished;
    atomic_int watchdog_fired;
} th_http_stop_watchdog_t;

static void *th_http_stop_watchdog(void *opaque) {
    th_http_stop_watchdog_t *watchdog = opaque;
    /* Hang detector, not a latency assertion: sized for the slowest
     * sanitizer/loaded-runner tail (see AGENTS.md, CI determinism). */
    for (int elapsed_ms = 0; elapsed_ms < 10000; elapsed_ms += 10) {
        if (atomic_load(&watchdog->stop_finished) ||
            (watchdog->operation_finished && atomic_load(watchdog->operation_finished)))
            return NULL;
        cbm_usleep(10 * 1000);
    }
    atomic_store(&watchdog->watchdog_fired, 1);
    (void)th_sock_shutdown(watchdog->socket);
    return NULL;
}

typedef struct {
    cbm_httpd_t *listener;
    atomic_int accepted;
    atomic_int finished;
} th_httpd_large_reply_t;

static void *th_httpd_large_reply(void *opaque) {
    th_httpd_large_reply_t *reply = opaque;
    cbm_http_conn_t *connection = cbm_httpd_accept(reply->listener, 3000);
    if (!connection) {
        atomic_store(&reply->finished, 1);
        return NULL;
    }
    atomic_store(&reply->accepted, 1);
    size_t response_size = 8U * 1024U * 1024U;
    char *response = malloc(response_size);
    if (response) {
        memset(response, 'R', response_size);
        cbm_http_reply_buf(connection, 200, "Content-Type: application/octet-stream\r\n", response,
                           response_size);
        free(response);
    }
    cbm_httpd_conn_close(connection);
    atomic_store(&reply->finished, 1);
    return NULL;
}

TEST(httpd_interrupt_unblocks_nonreading_large_response_within_one_second) {
    cbm_httpd_t *listener = cbm_httpd_listen(0);
    ASSERT_NOT_NULL(listener);
    cbm_httpd_set_send_buffer_for_test(listener, 64 * 1024);
    /* Deadline pinned far out of reach: the only remaining way the blocked
     * 8 MB send can end is the interrupt, so the join itself proves
     * interrupt causality — no wall-clock discrimination needed (the old
     * elapsed<500ms check was a lottery under sanitizer slowdown). */
    cbm_httpd_set_send_deadline_for_test(listener, 60 * 1000);
    th_httpd_large_reply_t reply = {.listener = listener};
    atomic_init(&reply.accepted, 0);
    atomic_init(&reply.finished, 0);

    th_sock_t socket = th_connect_with_recv_buffer(cbm_httpd_port(listener), 1024);
    ASSERT_TRUE(socket != TH_SOCK_BAD);
    cbm_thread_t reply_thread;
    ASSERT_EQ(cbm_thread_create(&reply_thread, 0, th_httpd_large_reply, &reply), 0);

    ASSERT_TRUE(th_wait_httpd_activity(listener, CBM_HTTPD_ACTIVITY_RESPONDING, 1000));
    ASSERT_EQ(atomic_load(&reply.accepted), 1);
    ASSERT_EQ(atomic_load(&reply.finished), 0);

    th_http_stop_watchdog_t watchdog = {
        .socket = socket,
        .operation_finished = &reply.finished,
    };
    atomic_init(&watchdog.stop_finished, 0);
    atomic_init(&watchdog.watchdog_fired, 0);
    cbm_thread_t watchdog_thread;
    ASSERT_EQ(cbm_thread_create(&watchdog_thread, 0, th_http_stop_watchdog, &watchdog), 0);

    cbm_httpd_interrupt(listener);
    ASSERT_EQ(cbm_thread_join(&reply_thread), 0);
    atomic_store(&watchdog.stop_finished, 1);
    ASSERT_EQ(cbm_thread_join(&watchdog_thread), 0);
    (void)th_sock_shutdown(socket);
    th_sock_close(socket);
    ASSERT_TRUE(cbm_httpd_close(listener));

    /* The 60 s deadline cannot have fired, so the join above is the proof
     * of interrupt delivery; the watchdog is purely a hang detector. */
    ASSERT_EQ(atomic_load(&watchdog.watchdog_fired), 0);
    PASS();
}

TEST(httpd_nonreading_large_response_hits_send_deadline_without_interrupt) {
    cbm_httpd_t *listener = cbm_httpd_listen(0);
    ASSERT_NOT_NULL(listener);
    cbm_httpd_set_send_buffer_for_test(listener, 64 * 1024);
    th_httpd_large_reply_t reply = {.listener = listener};
    atomic_init(&reply.accepted, 0);
    atomic_init(&reply.finished, 0);

    th_sock_t socket = th_connect_with_recv_buffer(cbm_httpd_port(listener), 1024);
    ASSERT_TRUE(socket != TH_SOCK_BAD);
    cbm_thread_t reply_thread;
    ASSERT_EQ(cbm_thread_create(&reply_thread, 0, th_httpd_large_reply, &reply), 0);
    ASSERT_TRUE(th_wait_httpd_activity(listener, CBM_HTTPD_ACTIVITY_RESPONDING, 1000));
    ASSERT_EQ(atomic_load(&reply.accepted), 1);
    ASSERT_EQ(atomic_load(&reply.finished), 0);
    uint64_t started = cbm_now_ms();

    th_http_stop_watchdog_t watchdog = {
        .socket = socket,
        .operation_finished = &reply.finished,
    };
    atomic_init(&watchdog.stop_finished, 0);
    atomic_init(&watchdog.watchdog_fired, 0);
    cbm_thread_t watchdog_thread;
    ASSERT_EQ(cbm_thread_create(&watchdog_thread, 0, th_http_stop_watchdog, &watchdog), 0);

    ASSERT_EQ(cbm_thread_join(&reply_thread), 0);
    uint64_t elapsed = cbm_now_ms() - started;
    atomic_store(&watchdog.stop_finished, 1);
    ASSERT_EQ(cbm_thread_join(&watchdog_thread), 0);
    th_sock_close(socket);
    ASSERT_TRUE(cbm_httpd_close(listener));

    ASSERT_GTE(elapsed, 500);
    ASSERT_EQ(atomic_load(&watchdog.watchdog_fired), 0);
    PASS();
}

TEST(ui_server_stop_interrupts_partial_request_within_one_second) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    cbm_http_server_set_recv_deadline_ms(ts.srv, 3000);
    int port = cbm_http_server_port(ts.srv);
    th_sock_t socket = th_connect(port);
    ASSERT_TRUE(socket != TH_SOCK_BAD);

    char partial[256];
    snprintf(partial, sizeof(partial), "GET /api/logs HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n", port);
    ASSERT_EQ(th_send_all(socket, partial, strlen(partial)), 0);
    ASSERT_TRUE(th_wait_http_server_activity(ts.srv, CBM_HTTPD_ACTIVITY_READING_REQUEST, 1000));

    th_http_stop_watchdog_t watchdog = {.socket = socket};
    atomic_init(&watchdog.stop_finished, 0);
    atomic_init(&watchdog.watchdog_fired, 0);
    cbm_thread_t watchdog_thread;
    ASSERT_EQ(cbm_thread_create(&watchdog_thread, 0, th_http_stop_watchdog, &watchdog), 0);

    uint64_t started = cbm_now_ms();
    cbm_http_server_stop(ts.srv);
    ASSERT_EQ(cbm_thread_join(&ts.tid), 0);
    uint64_t elapsed = cbm_now_ms() - started;
    atomic_store(&watchdog.stop_finished, 1);
    ASSERT_EQ(cbm_thread_join(&watchdog_thread), 0);
    (void)th_sock_shutdown(socket);
    th_sock_close(socket);
    ASSERT_TRUE(cbm_http_server_free(ts.srv));

    ASSERT_LT(elapsed, 1000);
    ASSERT_EQ(atomic_load(&watchdog.watchdog_fired), 0);
    PASS();
}

/* ── /api/repo-info git-remote URL helpers (distilled from PR #789) ── */

/* The web base must always be https (deep-links can't be downgraded) and must
 * never carry embedded credentials, across scp / ssh / https remote shapes. */
TEST(repo_info_web_base_normalizes_to_https) {
    struct {
        const char *in;
        const char *want;
    } cases[] = {
        {"git@github.com:org/repo.git", "https://github.com/org/repo"},
        {"git@github.com:org/repo", "https://github.com/org/repo"},
        {"https://github.com/org/repo.git", "https://github.com/org/repo"},
        {"ssh://git@github.com/org/repo.git", "https://github.com/org/repo"},
        {"https://user:token@github.com/org/repo.git", "https://github.com/org/repo"},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char *got = cbm_ui_git_web_base(cases[i].in);
        ASSERT_NOT_NULL(got);
        ASSERT_STR_EQ(got, cases[i].want);
        /* Never leak credentials into the web base. */
        ASSERT_NULL(strstr(got, "token"));
        ASSERT_NULL(strstr(got, "@"));
        free(got);
    }
    /* Unrecognized shapes yield NULL, not a bogus link. */
    ASSERT_NULL(cbm_ui_git_web_base(""));
    ASSERT_NULL(cbm_ui_git_web_base("not-a-url"));
    PASS();
}

/* The remote_url field echoed to the client must have any user:pass@ stripped. */
TEST(repo_info_strips_credentials_from_remote) {
    char *safe = cbm_ui_git_strip_credentials("https://alice:s3cr3t@github.com/org/repo.git");
    ASSERT_NOT_NULL(safe);
    ASSERT_STR_EQ(safe, "https://github.com/org/repo.git");
    ASSERT_NULL(strstr(safe, "s3cr3t"));
    ASSERT_NULL(strstr(safe, "alice"));
    free(safe);

    /* Credential-free URLs pass through unchanged. */
    char *plain = cbm_ui_git_strip_credentials("https://github.com/org/repo.git");
    ASSERT_NOT_NULL(plain);
    ASSERT_STR_EQ(plain, "https://github.com/org/repo.git");
    free(plain);

    /* An '@' in the path (not the authority) must not be treated as creds. */
    char *pathat = cbm_ui_git_strip_credentials("https://github.com/org/repo/@scope");
    ASSERT_NOT_NULL(pathat);
    ASSERT_STR_EQ(pathat, "https://github.com/org/repo/@scope");
    free(pathat);

    /* scp-style carries no secret and is left intact. */
    char *scp = cbm_ui_git_strip_credentials("git@github.com:org/repo.git");
    ASSERT_NOT_NULL(scp);
    ASSERT_STR_EQ(scp, "git@github.com:org/repo.git");
    free(scp);

    ASSERT_NULL(cbm_ui_git_strip_credentials(NULL));
    PASS();
}

/* ── #798 follow-up: full UI-mode hang repro (live sockets) ───── */

/* Like th_http but arms a client-side receive-timeout watchdog. If the
 * single-threaded server wedges, recv() returns instead of blocking forever, so
 * the test FAILs deterministically rather than hanging CI. 0 on connect/timeout. */
static int th_http_deadline(int port, const char *request, char *resp, size_t respsz,
                            int timeout_ms) {
    char *prepared = th_request_with_ui_headers(port, request);
    if (!prepared)
        return 0;
    th_sock_t s = th_connect(port);
    if (s == TH_SOCK_BAD) {
        free(prepared);
        return 0;
    }
#ifdef _WIN32
    DWORD tv = (DWORD)timeout_ms;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    if (th_send_all(s, prepared, strlen(prepared)) != 0) {
        free(prepared);
        th_sock_close(s);
        return 0;
    }
    free(prepared);
    int n = th_recv_until_close(s, resp, respsz);
    th_sock_close(s);
    return n;
}

/* #798 was a single-threaded-server wedge: list_projects never returned and the
 * whole UI stopped answering. Assert the running server answers list_projects
 * within a hard deadline while it holds live listening sockets. The client
 * receive-timeout is the watchdog: a wedge → no 200 → FAIL, never a CI hang. */
TEST(ui_server_list_projects_responds_under_watchdog) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    const char *body = "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\","
                       "\"params\":{\"name\":\"list_projects\",\"arguments\":{}}}";
    char req[512];
    snprintf(req, sizeof(req),
             "POST /rpc HTTP/1.1\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n\r\n%s",
             (int)strlen(body), body);
    char resp[8192];
    int n = th_http_deadline(cbm_http_server_port(ts.srv), req, resp, sizeof(resp), 15000);
    th_server_stop(&ts);
    ASSERT_GT(n, 0); /* a response arrived before the watchdog fired */
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "\"jsonrpc\""));
    PASS();
}

#ifdef _WIN32
typedef struct {
    char path[512];
    int resolved_ok;
} th_gitctx_probe_t;

static DWORD WINAPI th_gitctx_probe_thread(LPVOID arg) {
    th_gitctx_probe_t *p = (th_gitctx_probe_t *)arg;
    cbm_git_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    int rc = cbm_git_context_resolve(p->path, &ctx);
    p->resolved_ok = (rc == 0 && ctx.is_git) ? 1 : 0;
    cbm_git_context_free(&ctx);
    return 0;
}
#endif

/* The load-bearing end-to-end repro of #798: while the single-threaded UI server
 * holds LIVE listening/AFD socket handles in this process, cbm_git_context_resolve
 * — the exact path list_projects runs (add_git_context_json → resolve →
 * cbm_popen(git)) — must not hang. Under a raw-_popen regression git inherits
 * those sockets and its MSYS2 runtime deadlocks in NtQueryObject; the watchdog
 * turns that into a hard FAIL instead of an infinite hang. */
TEST(git_context_resolve_no_hang_under_live_ui_sockets) {
#ifndef _WIN32
    SKIP_PLATFORM("Windows-only: #798 UI listening-socket handle inheritance");
#else
    char *tmp = th_mktempdir("cbm_798repro");
    if (!tmp)
        FAIL("th_mktempdir returned NULL");

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "git -C \"%s\" init -q && git -C \"%s\" -c user.email=t@t -c user.name=t "
             "commit -q --allow-empty -m init",
             tmp, tmp);
    if (system(cmd) != 0) {
        th_rmtree(tmp);
        SKIP_PLATFORM("git not available to init a repo");
    }

    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);

    th_gitctx_probe_t *probe = (th_gitctx_probe_t *)calloc(1, sizeof(*probe));
    ASSERT_NOT_NULL(probe);
    snprintf(probe->path, sizeof(probe->path), "%s", tmp);

    HANDLE h = CreateThread(NULL, 0, th_gitctx_probe_thread, probe, 0, NULL);
    ASSERT_NOT_NULL(h);
    DWORD w = WaitForSingleObject(h, 30000);
    if (w != WAIT_OBJECT_0) {
        /* Wedged on the inherited-socket NtQueryObject walk. Deliberately leak
         * the heap probe + thread (a late wake must not touch freed memory);
         * process exit reaps them. Fail loudly rather than hang CI. */
        th_server_stop(&ts);
        FAIL("cbm_git_context_resolve hung under live UI sockets (#798 regression)");
    }
    CloseHandle(h);
    th_server_stop(&ts);
    int ok = probe->resolved_ok;
    free(probe);
    th_rmtree(tmp);
    ASSERT_EQ(ok, 1);
    PASS();
#endif
}

/* Host is part of the authority boundary: HTTP/1.1 requires exactly one, and
 * the optional port must be the actual bound port. This blocks DNS rebinding
 * and prevents a foreign localhost service from manufacturing same-origin
 * requests for this daemon. */
TEST(ui_server_rejects_non_loopback_host) {
    th_server_t ts;
    ASSERT_EQ(th_server_start(&ts), 0);
    int port = cbm_http_server_port(ts.srv);
    char resp[4096];

    int n = th_http(port, "GET / HTTP/1.1\r\nHost: evil.example.com\r\n\r\n", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);

    n = th_http_raw(port, "GET / HTTP/1.1\r\n\r\n", resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 400);

    static const char *bare_loopback_authorities[] = {
        "127.0.0.1",
        "localhost",
        "[::1]",
    };
    char req[256];
    for (size_t i = 0; i < sizeof(bare_loopback_authorities) / sizeof(bare_loopback_authorities[0]);
         i++) {
        snprintf(req, sizeof(req), "GET / HTTP/1.1\r\nHost: %s\r\n\r\n",
                 bare_loopback_authorities[i]);
        n = th_http_raw(port, req, resp, sizeof(resp));
        ASSERT_GT(n, 0);
        ASSERT_EQ(th_status(resp), 403);
    }

    snprintf(req, sizeof(req), "GET / HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n\r\n", port + 1);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_EQ(th_status(resp), 403);

    snprintf(req, sizeof(req), "GET / HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n\r\n", port);
    n = th_http_raw(port, req, resp, sizeof(resp));
    ASSERT_GT(n, 0);
    ASSERT_NEQ(th_status(resp), 400);
    ASSERT_NEQ(th_status(resp), 403);

    static const char *bare_origins[] = {
        "http://127.0.0.1",
        "http://localhost",
        "http://[::1]",
    };
    for (size_t i = 0; i < sizeof(bare_origins) / sizeof(bare_origins[0]); i++) {
        snprintf(req, sizeof(req), "GET / HTTP/1.1\r\nHost: 127.0.0.1:%d\r\nOrigin: %s\r\n\r\n",
                 port, bare_origins[i]);
        n = th_http_raw(port, req, resp, sizeof(resp));
        ASSERT_GT(n, 0);
        ASSERT_EQ(th_status(resp), 403);
    }

    th_server_stop(&ts);
    PASS();
}

/* The directory browser formats readdir() entries into a fixed 32 KB response
 * buffer. The per-entry loop is clamped, but the trailing "parent"/"roots"
 * appends were not — once the entries filled the buffer, pos ran past the end
 * and the next size argument wrapped, writing out of bounds. Fill the buffer
 * with many long-named subdirectories and browse it in a forked child so an
 * overflow surfaces as a killing signal (ASan abort) rather than a clean run. */
TEST(ui_server_browse_wide_dir_no_overflow) {
#ifdef _WIN32
    SKIP_PLATFORM("fork crash-isolation is POSIX-only; the clamp is platform-agnostic");
#else
    char *dir = th_mktempdir("cbm_browse");
    if (!dir) {
        FAIL("mktempdir");
    }
    char longname[240];
    memset(longname, 'a', sizeof(longname) - 1);
    longname[sizeof(longname) - 1] = '\0';
    for (int i = 0; i < 250; i++) { /* 250 * ~220 chars overflows the 32 KB buffer */
        char sub[600];
        snprintf(sub, sizeof(sub), "%s/%s%03d", dir, longname, i);
        th_mkdir_p(sub);
    }
    fflush(NULL);
    pid_t pid = fork();
    if (pid == 0) {
        th_server_t ts;
        if (th_server_start(&ts) != 0) {
            _exit(2);
        }
        char req[512];
        int port = cbm_http_server_port(ts.srv);
        snprintf(req, sizeof(req), "GET /api/browse?path=%s HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n\r\n",
                 dir, port);
        char *resp = malloc(262144);
        int n = resp ? th_http(port, req, resp, 262144) : 0;
        int ok = (n > 0 && strstr(resp, "HTTP/1.1 200") != NULL);
        free(resp);
        th_server_stop(&ts);
        _exit(ok ? 0 : 3);
    }
    ASSERT_TRUE(pid > 0);
    int status = 0;
    (void)waitpid(pid, &status, 0);
    char rm[600];
    snprintf(rm, sizeof(rm), "rm -rf '%s'", dir);
    (void)system(rm);
    if (WIFSIGNALED(status)) {
        char m[96];
        snprintf(m, sizeof(m), "browse killed by signal %d — response buffer overflow",
                 WTERMSIG(status));
        FAIL(m);
    }
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), 0);
    PASS();
#endif
}

/* The log endpoint serialises the ring into one heap buffer budgeted at
 * LOG_LINE_MAX + 10 bytes per line. JSON escaping doubles every '"' and '\\',
 * so an escape-dense line needs ~2x LOG_LINE_MAX. The inner escape loop stops
 * at buf_size - 10, but the per-line framing writes (separator comma, opening
 * quote, closing quote) were raw indexes, so every line past saturation wrote
 * three bytes beyond the allocation. Fill the ring with escape-dense lines and
 * read it back in a forked child, so the overflow surfaces as a killing signal
 * (ASan abort) instead of silent heap corruption. */
TEST(ui_server_logs_escape_dense_no_overflow) {
#ifdef _WIN32
    SKIP_PLATFORM("fork crash-isolation is POSIX-only; the clamp is platform-agnostic");
#else
    fflush(NULL);
    pid_t pid = fork();
    if (pid == 0) {
        /* Worst case the ring can hold: every byte escapes to two bytes. */
        char dense[512];
        for (size_t i = 0; i < sizeof(dense) - 1; i++) {
            dense[i] = (i % 2 == 0) ? '"' : '\\';
        }
        dense[sizeof(dense) - 1] = '\0';
        for (int i = 0; i < 500; i++) { /* fills the whole 500-entry ring */
            cbm_ui_log_append(dense);
        }
        th_server_t ts;
        if (th_server_start(&ts) != 0) {
            _exit(2);
        }
        char req[256];
        int port = cbm_http_server_port(ts.srv);
        snprintf(req, sizeof(req), "GET /api/logs?lines=500 HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n\r\n",
                 port);
        size_t cap = 4u * 1024u * 1024u;
        char *resp = malloc(cap);
        int n = resp ? th_http(port, req, resp, (int)cap) : 0;
        int ok = (n > 0 && strstr(resp, "HTTP/1.1 200") != NULL);
        free(resp);
        th_server_stop(&ts);
        _exit(ok ? 0 : 3);
    }
    ASSERT_TRUE(pid > 0);
    int status = 0;
    (void)waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
        char m[96];
        snprintf(m, sizeof(m), "logs killed by signal %d — response buffer overflow",
                 WTERMSIG(status));
        FAIL(m);
    }
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), 0);
    PASS();
#endif
}

/* The index-status endpoint renders every active job into a fixed 2 KB stack
 * buffer. http_appendf clamps its own writes, but the separator and the closing
 * bracket were raw indexes, so two jobs holding ~1 KB root paths (the field is
 * 1024 bytes and the value comes straight from POST /api/index) pushed pos to
 * the clamp and the close then wrote past the buffer. Drive it through the real
 * endpoint with the index executor stubbed out, in a forked child so the
 * overflow surfaces as a killing signal. */
#define MAX_TEST_INDEX_JOBS 4
TEST(ui_server_index_status_long_paths_no_overflow) {
#ifdef _WIN32
    SKIP_PLATFORM("fork crash-isolation is POSIX-only; the clamp is platform-agnostic");
#else
    char *base = th_mktempdir("cbm_status");
    if (!base) {
        FAIL("mktempdir");
    }
    /* Two real directories whose paths are long enough that two rendered job
     * entries exceed the 2 KB response buffer. */
    /* Four slots x ~520 chars overflows the 2 KB buffer while every path stays
     * well inside PATH_MAX — a single pair of ~1 KB paths would reach the
     * buffer too, but mkdir refuses them once the temp-dir prefix is added. */
    char comp[200];
    memset(comp, 'd', sizeof(comp) - 1);
    comp[sizeof(comp) - 1] = '\0';
    char deep[MAX_TEST_INDEX_JOBS][800];
    for (int j = 0; j < MAX_TEST_INDEX_JOBS; j++) {
        int n = snprintf(deep[j], sizeof(deep[j]), "%s/%d", base, j);
        while (n < 520) {
            n += snprintf(deep[j] + n, sizeof(deep[j]) - (size_t)n, "/%s", comp);
        }
        th_mkdir_p(deep[j]);
    }
    fflush(NULL);
    pid_t pid = fork();
    if (pid == 0) {
        /* The blocking executor holds every job open so all four slots are
         * occupied at once. With the non-blocking stub each job finishes
         * immediately and handle_index_start recycles the same slot, leaving a
         * single entry to render — far short of the buffer. */
        th_ui_blocking_index_executor_t executor = {0};
        atomic_init(&executor.calls, 0);
        atomic_init(&executor.release, 0);
        th_server_t ts;
        ts.srv = cbm_http_server_new(0);
        if (!ts.srv) {
            _exit(2);
        }
        cbm_http_server_set_index_executor(ts.srv, th_ui_blocking_index_executor, &executor);
        if (th_server_thread_start(&ts.tid, ts.srv) != 0) {
            _exit(2);
        }
        int port = cbm_http_server_port(ts.srv);
        for (int j = 0; j < MAX_TEST_INDEX_JOBS; j++) {
            char body[1200];
            snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", deep[j]);
            char request[1500];
            snprintf(request, sizeof(request),
                     "POST /api/index HTTP/1.1\r\nContent-Type: application/json\r\n"
                     "Content-Length: %zu\r\n\r\n%s",
                     strlen(body), body);
            char response[4096];
            int rn = th_http(port, request, response, sizeof(response));
            /* A rejected POST would leave the slot empty and the endpoint would
             * render nothing — a vacuous pass. Fail loudly instead. */
            if (rn <= 0 || th_status(response) != 202) {
                _exit(4);
            }
        }
        /* Wait for the state the assertion depends on — all four jobs actually
         * running — rather than for a duration. */
        if (!th_wait_atomic_int(&executor.calls, MAX_TEST_INDEX_JOBS, 5000)) {
            atomic_store(&executor.release, 1);
            _exit(5);
        }
        char req[256];
        snprintf(req, sizeof(req), "GET /api/index-status HTTP/1.1\r\nHost: 127.0.0.1:%d\r\n\r\n",
                 port);
        char resp[8192];
        int n = th_http(port, req, resp, sizeof(resp));
        int ok = (n > 0 && strstr(resp, "HTTP/1.1 200") != NULL);
        atomic_store(&executor.release, 1);
        th_server_stop(&ts);
        _exit(ok ? 0 : 3);
    }
    ASSERT_TRUE(pid > 0);
    int status = 0;
    (void)waitpid(pid, &status, 0);
    th_cleanup(base);
    if (WIFSIGNALED(status)) {
        char m[96];
        snprintf(m, sizeof(m), "index-status killed by signal %d — response buffer overflow",
                 WTERMSIG(status));
        FAIL(m);
    }
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), 0);
    PASS();
#endif
}

static void http_restore_cache(char *saved) {
    if (saved) {
        (void)cbm_setenv("CBM_CACHE_DIR", saved, 1);
        free(saved);
    } else {
        (void)cbm_unsetenv("CBM_CACHE_DIR");
    }
}

static bool http_write_project_db(const char *cache, const char *name, const char *root,
                                  const char *indexed_at) {
    char path[CBM_SZ_2K];
    cbm_store_t *st;
    bool ok;
    snprintf(path, sizeof(path), "%s/%s.db", cache, name);
    st = cbm_store_open_path(path);
    if (!st) {
        return false;
    }
    ok = cbm_store_upsert_project(st, name, root) == CBM_STORE_OK;
    if (ok && indexed_at && indexed_at[0]) {
        char sql[CBM_SZ_512];
        snprintf(sql, sizeof(sql), "UPDATE projects SET indexed_at='%s' WHERE name='%s';",
                 indexed_at, name);
        ok = sqlite3_exec(cbm_store_get_db(st), sql, NULL, NULL, NULL) == SQLITE_OK;
    }
    cbm_store_close(st);
    return ok;
}

static int http_post_index(int port, const char *body, char *response, size_t response_sz) {
    char request[1600];
    snprintf(request, sizeof(request),
             "POST /api/index HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             strlen(body), body);
    return th_http(port, request, response, response_sz);
}

/**
 * @sdd-task: Task #3 - HTTP + MCP + watcher Gherkin
 * @sdd-spec: specs/spec-004-j8k-adr-parse-on-reindex/spec.md
 * @sdd-decision: SDD-ADR-019 user-triggered fill; SDD-ADR-020 body 32768
 * @sdd-why: POST /api/index create/reindex must fill the same store GET /api/adr reads
 * @human-debug: Job stays indexing → executor/index_repository failed; see last_resp
 */
typedef struct {
    atomic_int calls;
    atomic_int rc;
    char last_resp[2048];
} th_ui_real_index_executor_t;

static int th_ui_real_index_executor(void *opaque, const char *root_path,
                                     const char *project_name) {
    th_ui_real_index_executor_t *ex = opaque;
    cbm_mcp_server_t *srv;
    char args[CBM_SZ_2K];
    char *resp;
    int ok;

    srv = cbm_mcp_server_new(NULL);
    if (!srv || !root_path) {
        if (ex)
            atomic_store(&ex->rc, -1);
        if (srv)
            cbm_mcp_server_free(srv);
        return -1;
    }
    snprintf(args, sizeof(args), "{\"repo_path\":\"%s\",\"name\":\"%s\",\"mode\":\"fast\"}",
             root_path, project_name ? project_name : "");
    resp = cbm_mcp_handle_tool(srv, "index_repository", args);
    ok = resp && strstr(resp, "\"status\":\"indexed\"") != NULL;
    if (ex) {
        atomic_fetch_add(&ex->calls, 1);
        if (resp)
            snprintf(ex->last_resp, sizeof(ex->last_resp), "%s", resp);
        atomic_store(&ex->rc, ok ? 0 : -1);
    }
    free(resp);
    cbm_mcp_server_free(srv);
    return ok ? 0 : -1;
}

static int ui_adr_fill_tree(const char *root, const char *purpose, const char *stack,
                            const char *decisions, const char *devlog) {
    if (th_write_file(TH_PATH(root, "main.py"), "def main():\n    return 1\n") != 0)
        return -1;
    if (purpose && th_write_file(TH_PATH(root, ".sdd-skill/context_ai.md"), purpose) != 0)
        return -1;
    if (stack && th_write_file(TH_PATH(root, ".sdd-skill/baseline/TECH_STACK.md"), stack) != 0)
        return -1;
    if (decisions &&
        th_write_file(TH_PATH(root, ".sdd-skill/baseline/ARCHITECTURE_ADR.md"), decisions) != 0)
        return -1;
    if (devlog && th_write_file(TH_PATH(root, ".sdd-skill/baseline/DEV_LOG.md"), devlog) != 0)
        return -1;
    return 0;
}

static void ui_make_unreadable(const char *path) {
#ifndef _WIN32
    if (chmod(path, 0) == 0) {
        FILE *f = fopen(path, "rb");
        if (!f)
            return;
        fclose(f);
        (void)chmod(path, 0644);
    }
#endif
    (void)cbm_unlink(path);
    (void)th_mkdir_p(path);
}

static int http_seed_adr(const char *cache, const char *project, const char *content) {
    char path[CBM_SZ_2K];
    cbm_store_t *store;
    int rc;

    snprintf(path, sizeof(path), "%s/%s.db", cache, project);
    store = cbm_store_open_path(path);
    if (!store)
        return -1;
    rc = cbm_store_adr_store(store, project, content);
    cbm_store_close(store);
    return rc;
}

static char *http_adr_load(const char *cache, const char *project) {
    char path[CBM_SZ_2K];
    cbm_store_t *store;
    cbm_adr_t adr;
    char *out = NULL;

    snprintf(path, sizeof(path), "%s/%s.db", cache, project);
    store = cbm_store_open_path_query(path);
    if (!store)
        return NULL;
    memset(&adr, 0, sizeof(adr));
    if (cbm_store_adr_get(store, project, &adr) == CBM_STORE_OK && adr.content)
        out = strdup(adr.content);
    if (adr.content)
        cbm_store_adr_free(&adr);
    cbm_store_close(store);
    return out;
}

static char *http_between(const char *doc, const char *start_m, const char *end_m) {
    const char *s;
    const char *e;
    size_t n;
    char *out;

    if (!doc)
        return NULL;
    s = strstr(doc, start_m);
    if (!s)
        return NULL;
    s += strlen(start_m);
    e = strstr(s, end_m);
    if (!e)
        return NULL;
    n = (size_t)(e - s);
    out = malloc(n + 1);
    if (!out)
        return NULL;
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

static bool http_ws_only(const char *s) {
    if (!s)
        return true;
    while (*s) {
        if (!isspace((unsigned char)*s))
            return false;
        s++;
    }
    return true;
}

static int ui_adr_fill_server_start(th_server_t *ts, th_ui_real_index_executor_t *exec) {
    memset(exec, 0, sizeof(*exec));
    atomic_init(&exec->calls, 0);
    atomic_init(&exec->rc, 0);
    ts->srv = cbm_http_server_new(0);
    if (!ts->srv)
        return -1;
    cbm_http_server_set_index_executor(ts->srv, th_ui_real_index_executor, exec);
    if (th_server_thread_start(&ts->tid, ts->srv) != 0) {
        (void)cbm_http_server_free(ts->srv);
        ts->srv = NULL;
        return -1;
    }
    return 0;
}

static int http_wait_index_done(int port, uint32_t timeout_ms) {
    uint64_t deadline = cbm_now_ms() + timeout_ms;
    char resp[8192];

    while (cbm_now_ms() < deadline) {
        if (th_http(port, "GET /api/index-status HTTP/1.1\r\n\r\n", resp, sizeof(resp)) > 0 &&
            th_status(resp) == 200) {
            if (strstr(resp, "\"status\":\"done\""))
                return 2;
            if (strstr(resp, "\"status\":\"error\""))
                return 3;
        }
        cbm_usleep(10000);
    }
    return -1;
}

static char *http_json_escape(const char *s) {
    size_t n = 0;
    size_t i;
    char *out;
    char *p;

    if (!s)
        s = "";
    for (i = 0; s[i]; i++) {
        char ch = s[i];
        if (ch == '"' || ch == '\\' || ch == '\n' || ch == '\r' || ch == '\t')
            n += 2;
        else
            n++;
    }
    out = malloc(n + 1);
    if (!out)
        return NULL;
    p = out;
    for (i = 0; s[i]; i++) {
        char ch = s[i];
        if (ch == '"') {
            *p++ = '\\';
            *p++ = '"';
        } else if (ch == '\\') {
            *p++ = '\\';
            *p++ = '\\';
        } else if (ch == '\n') {
            *p++ = '\\';
            *p++ = 'n';
        } else if (ch == '\r') {
            *p++ = '\\';
            *p++ = 'r';
        } else if (ch == '\t') {
            *p++ = '\\';
            *p++ = 't';
        } else {
            *p++ = ch;
        }
    }
    *p = '\0';
    return out;
}

static int ui_adr_post_escaped(th_server_t *ts, const char *project, const char *content,
                               char *resp, size_t respsz) {
    char *esc;
    char *body;
    char *req;
    size_t body_cap;
    size_t req_cap;
    int body_len;
    int n;

    esc = http_json_escape(content);
    if (!esc)
        return 0;
    body_cap = strlen(esc) + strlen(project) + 64;
    body = malloc(body_cap);
    if (!body) {
        free(esc);
        return 0;
    }
    body_len = snprintf(body, body_cap, "{\"project\":\"%s\",\"content\":\"%s\"}", project, esc);
    free(esc);
    if (body_len < 0 || (size_t)body_len >= body_cap) {
        free(body);
        return 0;
    }
    req_cap = (size_t)body_len + 256;
    req = malloc(req_cap);
    if (!req) {
        free(body);
        return 0;
    }
    snprintf(req, req_cap,
             "POST /api/adr HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %d\r\n\r\n%s",
             body_len, body);
    n = th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
    free(req);
    free(body);
    return n;
}

static char *http_manage_adr_get(const char *project) {
    cbm_mcp_server_t *srv = cbm_mcp_server_new(NULL);
    char args[256];
    char *resp;

    if (!srv)
        return NULL;
    snprintf(args, sizeof(args), "{\"project\":\"%s\",\"mode\":\"get\"}", project);
    resp = cbm_mcp_handle_tool(srv, "manage_adr", args);
    cbm_mcp_server_free(srv);
    return resp;
}

TEST(ui_index_owned_path_is_409_path_exists) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_owned");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char response[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-ownedc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 409);
    ASSERT_NOT_NULL(strstr(response, "path_exists"));
    ASSERT_NOT_NULL(strstr(response, "alpha"));
    ASSERT_EQ(atomic_load(&executor.calls), 0);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_trailing_slash_is_409_path_exists) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_slash");
    char slashed[300];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char response[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-slashc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    snprintf(slashed, sizeof(slashed), "%s/", root);
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", slashed);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 409);
    ASSERT_NOT_NULL(strstr(response, "path_exists"));
    ASSERT_EQ(atomic_load(&executor.calls), 0);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_derived_name_other_path_is_409_name_exists) {
    char cache[256];
    char owned[256];
    char request[256];
    char *derived;
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char response[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-nmc-XXXXXX");
    snprintf(owned, sizeof(owned), "/tmp/cbm-http-nmo-XXXXXX");
    snprintf(request, sizeof(request), "/tmp/cbm-http-nmr-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT(cbm_mkdtemp(owned) != NULL);
    ASSERT(cbm_mkdtemp(request) != NULL);
    derived = cbm_project_name_from_path(request);
    ASSERT(http_write_project_db(cache, derived, owned, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", request);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 409);
    ASSERT_NOT_NULL(strstr(response, "name_exists"));
    ASSERT_NOT_NULL(strstr(response, derived));
    ASSERT_EQ(atomic_load(&executor.calls), 0);

    free(derived);
    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(owned);
    th_cleanup(request);
    PASS();
}

TEST(ui_index_reindex_project_is_202) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_rx");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char response[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-rxc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT(http_write_project_db(cache, "custom", root, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"custom\"}", root);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, response, sizeof(response)), 0);
    bool called = th_wait_atomic_int(&executor.calls, 1, 2000);
    ASSERT_EQ(th_status(response), 202);
    ASSERT_TRUE(called);
    ASSERT_STR_EQ(executor.project_name, "custom");

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_reindex_project_name_alias_is_202) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_pna");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char response[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-pnac-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT(http_write_project_db(cache, "custom", root, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project_name\":\"custom\"}", root);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, response, sizeof(response)), 0);
    bool called = th_wait_atomic_int(&executor.calls, 1, 2000);
    ASSERT_EQ(th_status(response), 202);
    ASSERT_TRUE(called);
    ASSERT_STR_EQ(executor.project_name, "custom");

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_tie_existing_project_is_greater_name) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_tie");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char response[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-tiec-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-29T10:00:00Z"));
    ASSERT(http_write_project_db(cache, "beta", root, "2026-08-29T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 409);
    ASSERT_NOT_NULL(strstr(response, "path_exists"));
    ASSERT_NOT_NULL(strstr(response, "\"existing_project\":\"beta\""));
    ASSERT_EQ(atomic_load(&executor.calls), 0);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_inflight_second_create_is_409) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_if");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_blocking_index_executor_t executor = {0};
    th_server_t ts;
    char body[1024];
    char first[4096];
    char second[4096];

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-ifc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);

    atomic_init(&executor.calls, 0);
    atomic_init(&executor.release, 0);
    ts.srv = cbm_http_server_new(0);
    ASSERT_NOT_NULL(ts.srv);
    cbm_http_server_set_index_executor(ts.srv, th_ui_blocking_index_executor, &executor);
    ASSERT_EQ(th_server_thread_start(&ts.tid, ts.srv), 0);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, first, sizeof(first)), 0);
    ASSERT_EQ(th_status(first), 202);
    ASSERT_TRUE(th_wait_atomic_int(&executor.calls, 1, 2000));

    ASSERT_GT(http_post_index(cbm_http_server_port(ts.srv), body, second, sizeof(second)), 0);
    ASSERT_EQ(th_status(second), 409);
    ASSERT_NOT_NULL(strstr(second, "path_exists"));
    ASSERT_EQ(atomic_load(&executor.calls), 1);

    atomic_store(&executor.release, 1);
    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_reindex_fills_adr_and_migrates_unmarked) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_adr_rx");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char *stored;
    char *manual;
    char *mcp_get;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-rxc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_tree(root, "PURPOSE-ALPHA-GRAPH\n", "STACK-ALPHA-C11\n",
                               "DECISION-ALPHA-PATH\n", "SECRET-DEVLOG-ALPHA\n"),
              0);
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-30T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "alpha", "# Existing ADR\n"), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"alpha\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "alpha", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_END));
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_MANUAL_START));
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-ALPHA-GRAPH"));
    ASSERT_NOT_NULL(strstr(response, "STACK-ALPHA-C11"));
    ASSERT_NOT_NULL(strstr(response, "DECISION-ALPHA-PATH"));
    ASSERT_NULL(strstr(response, "SECRET-DEVLOG-ALPHA"));

    stored = http_adr_load(cache, "alpha");
    ASSERT_NOT_NULL(stored);
    manual = http_between(stored, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_NOT_NULL(strstr(manual, "# Existing ADR"));
    free(manual);
    free(stored);

    mcp_get = http_manage_adr_get("alpha");
    ASSERT_NOT_NULL(mcp_get);
    ASSERT_NOT_NULL(strstr(mcp_get, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(mcp_get, "PURPOSE-ALPHA-GRAPH"));
    ASSERT_NULL(strstr(mcp_get, "SECRET-DEVLOG-ALPHA"));
    free(mcp_get);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_create_fills_generated_empty_manual) {
    char cache[256];
    char *tmp = th_mktempdir("cbm_http_adr_new");
    char root[512];
    char *project;
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char *stored;
    char *manual;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-newc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(tmp);
    snprintf(root, sizeof(root), "%s/beta", tmp);
    ASSERT_EQ(th_mkdir_p(root), 0);
    ASSERT_EQ(ui_adr_fill_tree(root, "PURPOSE-BETA-NEW\n", "STACK-BETA-NEW\n",
                               "DECISION-BETA-NEW\n", NULL),
              0);
    project = cbm_project_name_from_path(root);
    ASSERT_NOT_NULL(project);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, project, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-BETA-NEW"));
    ASSERT_NOT_NULL(strstr(response, "STACK-BETA-NEW"));
    ASSERT_NOT_NULL(strstr(response, "DECISION-BETA-NEW"));

    stored = http_adr_load(cache, project);
    ASSERT_NOT_NULL(stored);
    manual = http_between(stored, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_TRUE(http_ws_only(manual));
    free(manual);
    free(stored);
    free(project);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(tmp);
    PASS();
}

TEST(ui_index_no_sdd_skill_leaves_adr_unmarked) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_adr_gamma");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char *stored;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-gammac-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, "main.py"), "def main():\n    return 1\n"), 0);
    ASSERT(http_write_project_db(cache, "gamma", root, "2026-08-30T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "gamma", "# Hand only\n"), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"gamma\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "gamma", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NULL(strstr(response, "CBM-GENERATED"));
    stored = http_adr_load(cache, "gamma");
    ASSERT_NOT_NULL(stored);
    ASSERT_STR_EQ(stored, "# Hand only\n");
    free(stored);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_partial_context_ai_job_succeeds) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_adr_part");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-partc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_tree(root, "PURPOSE-PARTIAL-ONLY\n", NULL, NULL, NULL), 0);
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-30T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"alpha\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "alpha", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-PARTIAL-ONLY"));
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_START));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_unreadable_architecture_omits_extract) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_adr_unr");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-unrc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_tree(root, "PURPOSE-READABLE\n", "STACK-READABLE\n",
                               "DECISION-UNREADABLE-FULL-COPY\n", NULL),
              0);
    ui_make_unreadable(TH_PATH(root, ".sdd-skill/baseline/ARCHITECTURE_ADR.md"));
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-30T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"alpha\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "alpha", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-READABLE"));
    ASSERT_NOT_NULL(strstr(response, "STACK-READABLE"));
    ASSERT_NULL(strstr(response, "DECISION-UNREADABLE-FULL-COPY"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_adr_generated_hand_edit_replaced_on_reindex) {
    static const char *hand =
        "<!-- CBM-GENERATED-START -->\n# Purpose\nPURPOSE-HAND-EDIT\n"
        "<!-- CBM-GENERATED-END -->\n<!-- CBM-MANUAL-START -->\n# notes\n"
        "<!-- CBM-MANUAL-END -->\n";
    char cache[256];
    char *root = th_mktempdir("cbm_http_adr_canon");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-canon-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_tree(root, "PURPOSE-CANONICAL\n", "STACK-CANON\n", "DECISION-CANON\n",
                               NULL),
              0);
    ASSERT(http_write_project_db(cache, "alpha", root, "2026-08-30T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "alpha", hand), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    ASSERT_GT(ui_adr_post_escaped(&ts, "alpha", hand, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"alpha\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "alpha", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-CANONICAL"));
    ASSERT_NULL(strstr(response, "PURPOSE-HAND-EDIT"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_adr_post_body_max_32768) {
    enum { CONTENT_16K = CBM_SZ_16K };
    char cache[256];
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_server_t ts;
    char *content;
    char resp[4096];
    char *over;
    char *req;
    size_t over_len = (size_t)CBM_SZ_32K + 1;
    size_t req_cap;
    int i;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-adr-cap-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(th_server_start(&ts), 0);
    content = malloc(CONTENT_16K + 1);
    ASSERT_NOT_NULL(content);
    memcpy(content, CBM_ADR_GENERATED_START, strlen(CBM_ADR_GENERATED_START));
    for (i = (int)strlen(CBM_ADR_GENERATED_START); i < CONTENT_16K; i++)
        content[i] = (char)('A' + (i % 26));
    content[CONTENT_16K] = '\0';
    ASSERT_GT(ui_adr_post_escaped(&ts, "adr-cap", content, resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_NOT_NULL(strstr(resp, "{\"saved\":true}"));
    free(content);

    over = malloc(over_len + 1);
    ASSERT_NOT_NULL(over);
    memset(over, 'x', over_len);
    over[over_len] = '\0';
    req_cap = over_len + 256;
    req = malloc(req_cap);
    ASSERT_NOT_NULL(req);
    snprintf(req, req_cap,
             "POST /api/adr HTTP/1.1\r\nContent-Type: application/json\r\n"
             "Content-Length: %zu\r\n\r\n%s",
             over_len, over);
    ASSERT_GT(th_http(cbm_http_server_port(ts.srv), req, resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 400);
    ASSERT_NOT_NULL(strstr(resp, "invalid body"));
    free(req);
    free(over);
    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    PASS();
}

/**
 * @sdd-task: Task #2 - HTTP + MCP + watcher Gherkin
 * @sdd-spec: specs/spec-013-r9w-adr-fill-gamedev-trio/spec.md
 * @sdd-decision: SDD-ADR-058 XOR; SDD-ADR-060 NULL=neither dir
 * @sdd-why: Job Gherkin: gamedev XOR fill on POST /api/index; leftover sdd omitted
 * @human-debug: Dual-tree leftover PURPOSE-SDD-* in GET /api/adr → XOR lost or a spec-004
 * fixture grew .gamedev/. Empty .gamedev/ with leftover sdd extract → presence treated as missing
 */
static int ui_adr_fill_gamedev_tree(const char *root, const char *purpose, const char *stack,
                                    const char *decisions, const char *gdd) {
    if (th_write_file(TH_PATH(root, "main.py"), "def main():\n    return 1\n") != 0)
        return -1;
    if (th_mkdir_p(TH_PATH(root, ".gamedev")) != 0)
        return -1;
    if (purpose && th_write_file(TH_PATH(root, ".gamedev/game_context.md"), purpose) != 0)
        return -1;
    if (stack && th_write_file(TH_PATH(root, ".gamedev/baseline/TECH_STACK.md"), stack) != 0)
        return -1;
    if (decisions &&
        th_write_file(TH_PATH(root, ".gamedev/baseline/ARCHITECTURE_ADR.md"), decisions) != 0)
        return -1;
    if (gdd &&
        th_write_file(TH_PATH(root, ".gamedev/phases/01-preproduction/gdd.md"), gdd) != 0)
        return -1;
    return 0;
}

static int ui_adr_fill_leftover_sdd(const char *root, const char *purpose, const char *stack,
                                    const char *decisions, const char *devlog) {
    if (purpose && th_write_file(TH_PATH(root, ".sdd-skill/context_ai.md"), purpose) != 0)
        return -1;
    if (stack && th_write_file(TH_PATH(root, ".sdd-skill/baseline/TECH_STACK.md"), stack) != 0)
        return -1;
    if (decisions &&
        th_write_file(TH_PATH(root, ".sdd-skill/baseline/ARCHITECTURE_ADR.md"), decisions) != 0)
        return -1;
    if (devlog && th_write_file(TH_PATH(root, ".sdd-skill/baseline/DEV_LOG.md"), devlog) != 0)
        return -1;
    return 0;
}

static char *ui_adr_slurp(const char *path) {
    FILE *f = cbm_fopen(path, "rb");
    long sz;
    char *buf;
    size_t n;

    if (!f)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = '\0';
    return buf;
}

TEST(ui_index_reindex_fills_gamedev_omits_leftover_sdd) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_rx");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char purpose_path[512];
    char stack_path[512];
    char dec_path[512];
    char *before_p;
    char *before_s;
    char *before_d;
    char *after_p;
    char *after_s;
    char *after_d;
    char *stored;
    char *manual;
    char *mcp_get;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-rxc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, "PURPOSE-GAME-BEVY\n", "STACK-GAME-BEVY\n",
                                       "DECISION-GAME-BEVY\n", "SECRET-GDD-BEVY\n"),
              0);
    ASSERT_EQ(ui_adr_fill_leftover_sdd(root, "PURPOSE-SDD-MVP1\n", "STACK-SDD-MVP1\n",
                                       "DECISION-SDD-MVP1\n", "SECRET-DEVLOG-BEVY\n"),
              0);
    snprintf(purpose_path, sizeof(purpose_path), "%s",
             TH_PATH(root, ".gamedev/game_context.md"));
    snprintf(stack_path, sizeof(stack_path), "%s",
             TH_PATH(root, ".gamedev/baseline/TECH_STACK.md"));
    snprintf(dec_path, sizeof(dec_path), "%s",
             TH_PATH(root, ".gamedev/baseline/ARCHITECTURE_ADR.md"));
    before_p = ui_adr_slurp(purpose_path);
    before_s = ui_adr_slurp(stack_path);
    before_d = ui_adr_slurp(dec_path);
    ASSERT_NOT_NULL(before_p);
    ASSERT_NOT_NULL(before_s);
    ASSERT_NOT_NULL(before_d);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "bevy", "# Existing ADR\n"), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_END));
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_MANUAL_START));
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-GAME-BEVY"));
    ASSERT_NOT_NULL(strstr(response, "STACK-GAME-BEVY"));
    ASSERT_NOT_NULL(strstr(response, "DECISION-GAME-BEVY"));
    ASSERT_NOT_NULL(strstr(response, "# Purpose"));
    ASSERT_NOT_NULL(strstr(response, "# Stack"));
    ASSERT_NOT_NULL(strstr(response, "# Decisions"));
    ASSERT_NULL(strstr(response, "PURPOSE-SDD-MVP1"));
    ASSERT_NULL(strstr(response, "STACK-SDD-MVP1"));
    ASSERT_NULL(strstr(response, "DECISION-SDD-MVP1"));
    ASSERT_NULL(strstr(response, "SECRET-GDD-BEVY"));
    ASSERT_NULL(strstr(response, "SECRET-DEVLOG-BEVY"));

    stored = http_adr_load(cache, "bevy");
    ASSERT_NOT_NULL(stored);
    manual = http_between(stored, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_NOT_NULL(strstr(manual, "# Existing ADR"));
    free(manual);
    free(stored);

    mcp_get = http_manage_adr_get("bevy");
    ASSERT_NOT_NULL(mcp_get);
    ASSERT_NOT_NULL(strstr(mcp_get, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(mcp_get, "PURPOSE-GAME-BEVY"));
    ASSERT_NULL(strstr(mcp_get, "PURPOSE-SDD-MVP1"));
    free(mcp_get);

    after_p = ui_adr_slurp(purpose_path);
    after_s = ui_adr_slurp(stack_path);
    after_d = ui_adr_slurp(dec_path);
    ASSERT_NOT_NULL(after_p);
    ASSERT_NOT_NULL(after_s);
    ASSERT_NOT_NULL(after_d);
    ASSERT_STR_EQ(after_p, before_p);
    ASSERT_STR_EQ(after_s, before_s);
    ASSERT_STR_EQ(after_d, before_d);
    free(before_p);
    free(before_s);
    free(before_d);
    free(after_p);
    free(after_s);
    free(after_d);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_create_gamedev_fills_generated_empty_manual) {
    char cache[256];
    char *tmp = th_mktempdir("cbm_http_gd_new");
    char root[512];
    char *project;
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char *stored;
    char *manual;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-newc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(tmp);
    snprintf(root, sizeof(root), "%s/gamma", tmp);
    ASSERT_EQ(th_mkdir_p(root), 0);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, "PURPOSE-GAMMA-NEW\n", "STACK-GAMMA-NEW\n",
                                       "DECISION-GAMMA-NEW\n", NULL),
              0);
    project = cbm_project_name_from_path(root);
    ASSERT_NOT_NULL(project);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, project, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_START));
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-GAMMA-NEW"));
    ASSERT_NOT_NULL(strstr(response, "STACK-GAMMA-NEW"));
    ASSERT_NOT_NULL(strstr(response, "DECISION-GAMMA-NEW"));

    stored = http_adr_load(cache, project);
    ASSERT_NOT_NULL(stored);
    manual = http_between(stored, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_TRUE(http_ws_only(manual));
    free(manual);
    free(stored);
    free(project);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(tmp);
    PASS();
}

TEST(ui_index_add_gamedev_overwrites_sdd_keeps_manual) {
    static const char *former =
        "<!-- CBM-GENERATED-START -->\n# Purpose\nPURPOSE-SDD-MVP1\n"
        "<!-- CBM-GENERATED-END -->\n<!-- CBM-MANUAL-START -->\n# Keep notes\n"
        "<!-- CBM-MANUAL-END -->\n";
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_sw");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char *stored;
    char *manual;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-swc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, "main.py"), "def main():\n    return 1\n"), 0);
    ASSERT_EQ(ui_adr_fill_leftover_sdd(root, "PURPOSE-SDD-MVP1\n", NULL, NULL, NULL), 0);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "bevy", former), CBM_STORE_OK);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, "PURPOSE-GAME-SWITCH\n", NULL, NULL, NULL), 0);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-GAME-SWITCH"));
    ASSERT_NULL(strstr(response, "PURPOSE-SDD-MVP1"));

    stored = http_adr_load(cache, "bevy");
    ASSERT_NOT_NULL(stored);
    manual = http_between(stored, CBM_ADR_MANUAL_START, CBM_ADR_MANUAL_END);
    ASSERT_NOT_NULL(manual);
    ASSERT_NOT_NULL(strstr(manual, "# Keep notes"));
    free(manual);
    free(stored);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_gamedev_only_tech_stack_no_sdd_fallback) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_part");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-partc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, NULL, "STACK-PARTIAL-ONLY\n", NULL, NULL), 0);
    ASSERT_EQ(ui_adr_fill_leftover_sdd(root, "PURPOSE-SDD-FALLBACK\n", NULL, NULL, NULL), 0);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "STACK-PARTIAL-ONLY"));
    ASSERT_NOT_NULL(strstr(response, "# Stack"));
    ASSERT_NULL(strstr(response, "# Purpose"));
    ASSERT_NULL(strstr(response, "# Decisions"));
    ASSERT_NULL(strstr(response, "PURPOSE-SDD-FALLBACK"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_empty_gamedev_dir_no_sdd_fallback) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_empty");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-emptyc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, NULL, NULL, NULL, NULL), 0);
    ASSERT_EQ(ui_adr_fill_leftover_sdd(root, "PURPOSE-SDD-EMPTYDIR\n", NULL, NULL, NULL), 0);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, CBM_ADR_GENERATED_START));
    ASSERT_NULL(strstr(response, "PURPOSE-SDD-EMPTYDIR"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_remove_gamedev_restores_sdd) {
    static const char *former =
        "<!-- CBM-GENERATED-START -->\n# Purpose\nPURPOSE-GAME-BEVY\n"
        "<!-- CBM-GENERATED-END -->\n<!-- CBM-MANUAL-START -->\n# notes\n"
        "<!-- CBM-MANUAL-END -->\n";
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_rm");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-rmc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, "main.py"), "def main():\n    return 1\n"), 0);
    ASSERT_EQ(ui_adr_fill_leftover_sdd(root, "PURPOSE-SDD-RESTORED\n", NULL, NULL, NULL), 0);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "bevy", former), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-SDD-RESTORED"));
    ASSERT_NULL(strstr(response, "PURPOSE-GAME-BEVY"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_both_skill_dirs_gone_leaves_last_blob) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_gone");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    char *stored;
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-gonec-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(th_write_file(TH_PATH(root, "main.py"), "def main():\n    return 1\n"), 0);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "bevy", "# Last gamedev generated leftover\n"), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NULL(strstr(response, "CBM-GENERATED"));
    stored = http_adr_load(cache, "bevy");
    ASSERT_NOT_NULL(stored);
    ASSERT_STR_EQ(stored, "# Last gamedev generated leftover\n");
    free(stored);

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_index_unreadable_game_context_omits_purpose) {
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_unr");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-unrc-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, "PURPOSE-GAME-UNREADABLE\n", "STACK-READABLE-GAME\n",
                                       "DECISION-READABLE-GAME\n", NULL),
              0);
    ASSERT_EQ(ui_adr_fill_leftover_sdd(root, "PURPOSE-SDD-UNREADABLE\n", NULL, NULL, NULL), 0);
    /* chmod 0 is SDD-ADR-022, but .gamedev is not ALWAYS_SKIP — semantic_manifest
     * hashes the trio as graph source and a mode-000 file fails the job. Directory
     * at the trio path is the same fopen-fail fixture ui_make_unreadable uses when
     * chmod still allows read. Fill omits Purpose; the index job can succeed. */
    {
        char purpose_path[512];
        snprintf(purpose_path, sizeof(purpose_path), "%s",
                 TH_PATH(root, ".gamedev/game_context.md"));
        (void)cbm_unlink(purpose_path);
        ASSERT_EQ(th_mkdir_p(purpose_path), 0);
    }
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "STACK-READABLE-GAME"));
    ASSERT_NOT_NULL(strstr(response, "DECISION-READABLE-GAME"));
    ASSERT_NULL(strstr(response, "# Purpose"));
    ASSERT_NULL(strstr(response, "PURPOSE-SDD-UNREADABLE"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

TEST(ui_adr_gamedev_generated_hand_edit_replaced_on_reindex) {
    static const char *hand =
        "<!-- CBM-GENERATED-START -->\n# Purpose\nPURPOSE-HAND-EDIT-GAME\n"
        "<!-- CBM-GENERATED-END -->\n<!-- CBM-MANUAL-START -->\n# notes\n"
        "<!-- CBM-MANUAL-END -->\n";
    char cache[256];
    char *root = th_mktempdir("cbm_http_gd_canon");
    const char *saved = getenv("CBM_CACHE_DIR");
    char *saved_copy = saved ? strdup(saved) : NULL;
    th_ui_real_index_executor_t exec;
    th_server_t ts;
    char body[1024];
    char response[8192];
    int port;
    int job;

    snprintf(cache, sizeof(cache), "/tmp/cbm-http-gd-canon-XXXXXX");
    ASSERT(cbm_mkdtemp(cache) != NULL);
    ASSERT_NOT_NULL(root);
    ASSERT_EQ(ui_adr_fill_gamedev_tree(root, "PURPOSE-CANONICAL-GAME\n", NULL, NULL, NULL), 0);
    ASSERT(http_write_project_db(cache, "bevy", root, "2026-08-31T10:00:00Z"));
    ASSERT_EQ(http_seed_adr(cache, "bevy", hand), CBM_STORE_OK);
    ASSERT_EQ(cbm_setenv("CBM_CACHE_DIR", cache, 1), 0);
    ASSERT_EQ(ui_adr_fill_server_start(&ts, &exec), 0);
    port = cbm_http_server_port(ts.srv);

    ASSERT_GT(ui_adr_post_escaped(&ts, "bevy", hand, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);

    snprintf(body, sizeof(body), "{\"root_path\":\"%s\",\"project\":\"bevy\"}", root);
    ASSERT_GT(http_post_index(port, body, response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 202);
    job = http_wait_index_done(port, 30000);
    ASSERT_EQ(job, 2);

    ASSERT_GT(ui_adr_get_request(&ts, "bevy", response, sizeof(response)), 0);
    ASSERT_EQ(th_status(response), 200);
    ASSERT_NOT_NULL(strstr(response, "PURPOSE-CANONICAL-GAME"));
    ASSERT_NULL(strstr(response, "PURPOSE-HAND-EDIT-GAME"));

    th_server_stop(&ts);
    http_restore_cache(saved_copy);
    th_cleanup(cache);
    th_cleanup(root);
    PASS();
}

/**
 * @sdd-task: Task #2 - HTTP POST + GET merge + C Gherkin + publish copy
 * @sdd-spec: specs/spec-006-k3n-spec-archive/spec.md
 * @sdd-decision: SDD-ADR-030 POST /api/spec-board; SDD-ADR-031 flag object
 * @sdd-why: C owns merge/409/404/400/idempotent/orphan; fixtures stay under /tmp
 * @human-debug: 409 still persisted → POST wrote before the done-column check
 */
static const char *SB_HTTP_ACTIVE =
    "{\n"
    "  \"active_spec\": null,\n"
    "  \"planned_specs\": [\"spec-010-aaa-planned\"],\n"
    "  \"draft_specs\": [],\n"
    "  \"completed_specs\": [\"spec-012-ccc-closed\"]\n"
    "}\n";

static char *th_read_file_alloc(const char *path) {
    FILE *f = cbm_fopen(path, "rb");
    long sz;
    char *buf;
    size_t n;
    if (!f) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[n] = '\0';
    return buf;
}

static const char *th_http_json(const char *resp) {
    const char *body = strstr(resp, "\r\n\r\n");
    return body ? body + 4 : NULL;
}

static bool sb_json_spec_has(const char *json, const char *id, const char *needle) {
    char key[256];
    const char *p;
    const char *next;
    size_t span;
    char buf[2048];

    snprintf(key, sizeof(key), "\"id\":\"%s\"", id);
    p = strstr(json, key);
    if (!p) {
        return false;
    }
    next = strstr(p + 1, "\"id\":");
    span = next ? (size_t)(next - p) : strlen(p);
    if (span >= sizeof(buf)) {
        span = sizeof(buf) - 1;
    }
    memcpy(buf, p, span);
    buf[span] = '\0';
    return strstr(buf, needle) != NULL;
}

static int sb_http_seed_board(ui_delete_fixture_t *fx) {
    if (th_write_file(TH_PATH(fx->root_dir, ".sdd-skill/specs/active.json"), SB_HTTP_ACTIVE) != 0) {
        return -1;
    }
    if (!http_write_project_db(fx->cache_dir, "alpha", fx->root_dir, NULL)) {
        return -1;
    }
    return 0;
}

static int sb_http_seed_flag(const ui_delete_fixture_t *fx, const char *spec_id, int archived) {
    char db_path[1024];
    cbm_store_t *st;
    int rc;

    ui_delete_db_path(fx, "alpha", db_path, sizeof(db_path));
    st = cbm_store_open_path(db_path);
    if (!st) {
        return CBM_STORE_ERR;
    }
    rc = cbm_store_spec_archive_set(st, spec_id, archived);
    cbm_store_close(st);
    return rc;
}

static int ui_spec_board_get(th_server_t *ts, const char *project, char *resp, size_t respsz) {
    char req[512];
    int n = snprintf(req, sizeof(req), "GET /api/spec-board?project=%s HTTP/1.1\r\n\r\n", project);
    if (n < 0 || (size_t)n >= sizeof(req)) {
        return 0;
    }
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

static int ui_spec_board_post(th_server_t *ts, const char *body, char *resp, size_t respsz) {
    char req[4608];
    int n = snprintf(req, sizeof(req),
                     "POST /api/spec-board HTTP/1.1\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %zu\r\n\r\n%s",
                     strlen(body), body);
    if (n < 0 || (size_t)n >= sizeof(req)) {
        return 0;
    }
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

TEST(ui_spec_board_get_merges_done_orphan_leftover_todo) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_flag(&fx, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(sb_http_seed_flag(&fx, "spec-099-zzz-gone", 1), CBM_STORE_OK);
    ASSERT_EQ(sb_http_seed_flag(&fx, "spec-010-aaa-planned", 1), CBM_STORE_OK);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_spec_has(json, "spec-012-ccc-closed", "\"column\":\"done\""));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-012-ccc-closed", "\"archived\":true"));
    ASSERT_NULL(strstr(json, "\"column\":\"archived\""));
    ASSERT_NULL(strstr(json, "spec-099-zzz-gone"));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-010-aaa-planned", "\"column\":\"todo\""));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-010-aaa-planned", "\"archived\":true"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_200_flag_object_and_idempotent) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[8192];
    const char *json;
    char *before;
    char *after;
    char active_path[1024];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    before = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(before);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"alpha\",\"spec_id\":\"spec-012-ccc-closed\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"spec_id\":\"spec-012-ccc-closed\""));
    ASSERT_NOT_NULL(strstr(json, "\"archived\":true"));
    ASSERT_NULL(strstr(json, "sdd_skill_present"));
    ASSERT_NULL(strstr(json, "\"specs\""));

    after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(after);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"alpha\",\"spec_id\":\"spec-012-ccc-closed\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"archived\":true"));

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_spec_has(json, "spec-012-ccc-closed", "\"archived\":true"));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-012-ccc-closed", "\"column\":\"done\""));

    after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(after);
    free(before);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_409_todo_writes_nothing) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[8192];
    const char *json;
    char *before;
    char *after;
    char active_path[1024];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    before = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(before);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"alpha\",\"spec_id\":\"spec-010-aaa-planned\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 409);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"spec not done\"}");

    after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(before);
    free(after);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_spec_has(json, "spec-010-aaa-planned", "\"archived\":false"));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-010-aaa-planned", "\"column\":\"todo\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_404_unknown_spec) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"alpha\",\"spec_id\":\"spec-099-zzz-gone\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"spec not found\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_400_missing_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_post(&ts, "{\"spec_id\":\"spec-012-ccc-closed\",\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 400);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"error\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_400_invalid_archived) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"alpha\",\"spec_id\":\"spec-012-ccc-closed\","
                                 "\"archived\":\"yes\"}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 400);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"invalid archived\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_404_unknown_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"missing-proj\",\"spec_id\":\"spec-012-ccc-closed\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"project not found\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_423_busy) {
    ui_delete_fixture_t fx;
    th_ui_mutation_guard_t guard;
    th_server_t ts;
    char resp[4096];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    th_ui_mutation_guard_init(&guard, false);
    ASSERT_EQ(th_server_start_with_mutation_guard(&ts, NULL, &guard), 0);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"alpha\",\"spec_id\":\"spec-012-ccc-closed\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 423);
    ASSERT_NOT_NULL(strstr(resp, "project is busy; retry after indexing"));
    ASSERT_EQ(atomic_load(&guard.begin_calls), 1);
    ASSERT_EQ(atomic_load(&guard.end_calls), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    ASSERT_TRUE(sb_json_spec_has(th_http_json(resp), "spec-012-ccc-closed", "\"archived\":false"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

/**
 * @sdd-task: Task #2 - HTTP GET additive + POST epic-id 404
 * @sdd-spec: specs/spec-008-g8r-grill-epic-todo/spec.md
 * @sdd-decision: SDD-ADR-035 additive GET; SDD-ADR-036 grill parse not in HTTP
 * @sdd-why: C HTTP Gherkin: mixed GET JSON, POST epic 404, unknown project, zero skill writes
 * @human-debug: POST epic 200 → spec_board_find walked epics[] or store ran before 404
 */
static const char *SB_HTTP_GRILL_EPIC_ID =
    ".grill/plans/inbox-plan/epics/epic-001-inbox.md";

static int sb_http_seed_grill(ui_delete_fixture_t *fx) {
    if (th_write_file(TH_PATH(fx->root_dir, ".grill/index.md"),
                      "# .grill/\n\n"
                      "| slug | title | status |\n"
                      "|------|-------|--------|\n"
                      "| inbox-plan | Inbox Plan | draft |\n") != 0) {
        return -1;
    }
    if (th_write_file(TH_PATH(fx->root_dir, ".grill/plans/inbox-plan/epics/epic-001-inbox.md"),
                      "name: inbox\nstatus: pending\n\nsummary: Filter unread first.\n") != 0) {
        return -1;
    }
    return th_write_file(TH_PATH(fx->root_dir, ".sdd-skill/specs/spec-010-aaa-planned/spec.md"),
                         "# Spec-010-aaa: Planned\n\nNo Companion-to grill path.\n");
}

/**
 * @sdd-task: Task #2 - HTTP GET additive + POST leftover locks
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-065 same GET always-emit debt; POST stays spec-only
 * @sdd-why: fixture + JSON helpers for HTTP debt Gherkin; HTTP does not fopen TECH_DEBT.md
 * @human-debug: GET 200 missing debt → to_json not Task #1; POST epic 200 → find walked epics[]
 */
static int sb_http_seed_tech_debt(ui_delete_fixture_t *fx, const char *body) {
    return th_write_file(TH_PATH(fx->root_dir, ".sdd-skill/baseline/TECH_DEBT.md"), body);
}

static int sb_json_debt_len(const char *json) {
    const char *p;
    const char *end;
    int n = 0;

    if (!json) {
        return -1;
    }
    p = strstr(json, "\"debt\":");
    if (!p) {
        return -1;
    }
    p = strchr(p, '[');
    if (!p) {
        return -1;
    }
    end = strchr(p, ']');
    if (!end) {
        return -1;
    }
    for (;;) {
        p = strstr(p, "\"id\":");
        if (!p || p >= end) {
            break;
        }
        n++;
        p += 5;
    }
    return n;
}

static int sb_json_debt_copy(const char *json, char *buf, size_t bufsz) {
    const char *p;
    const char *end;
    size_t n;

    if (!json || !buf || bufsz == 0) {
        return -1;
    }
    p = strstr(json, "\"debt\":");
    if (!p) {
        return -1;
    }
    p = strchr(p, '[');
    if (!p) {
        return -1;
    }
    end = strchr(p, ']');
    if (!end) {
        return -1;
    }
    n = (size_t)(end - p + 1);
    if (n >= bufsz) {
        n = bufsz - 1;
    }
    memcpy(buf, p, n);
    buf[n] = '\0';
    return 0;
}

static int sb_http_archive_has_id(const ui_delete_fixture_t *fx, const char *spec_id) {
    char db_path[1024];
    cbm_store_t *st;
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = 0;
    int i;

    ui_delete_db_path(fx, "alpha", db_path, sizeof(db_path));
    st = cbm_store_open_path_query(db_path);
    if (!st) {
        return -1;
    }
    if (cbm_store_spec_archive_load(st, rows, CBM_SPEC_ARCHIVE_CAP, &n) != CBM_STORE_OK) {
        cbm_store_close(st);
        return -1;
    }
    cbm_store_close(st);
    for (i = 0; i < n; i++) {
        if (strcmp(rows[i].spec_id, spec_id) == 0) {
            return 1;
        }
    }
    return 0;
}

static bool sb_json_obj_has(const char *json, const char *id, const char *needle) {
    char key[320];
    const char *p;
    const char *start;
    const char *next;
    size_t span;
    char buf[2048];

    snprintf(key, sizeof(key), "\"id\":\"%s\"", id);
    p = strstr(json, key);
    if (!p) {
        return false;
    }
    start = p;
    while (start > json && *start != '{') {
        start--;
    }
    next = strstr(p + 1, "\"id\":");
    span = next ? (size_t)(next - start) : strlen(start);
    if (span >= sizeof(buf)) {
        span = sizeof(buf) - 1;
    }
    memcpy(buf, start, span);
    buf[span] = '\0';
    return strstr(buf, needle) != NULL;
}

static bool sb_json_specs_contain_id(const char *json, const char *id) {
    const char *specs;
    const char *epics;
    char key[320];
    size_t span;
    char buf[8192];

    specs = strstr(json, "\"specs\":");
    epics = strstr(json, "\"epics\":");
    if (!specs || !epics || epics <= specs) {
        return false;
    }
    snprintf(key, sizeof(key), "\"id\":\"%s\"", id);
    span = (size_t)(epics - specs);
    if (span >= sizeof(buf)) {
        span = sizeof(buf) - 1;
    }
    memcpy(buf, specs, span);
    buf[span] = '\0';
    return strstr(buf, key) != NULL;
}

TEST(ui_spec_board_get_200_mixed_todo_grill_epics) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char index_path[1024];
    char epic_path[1024];
    char active_path[1024];
    char *index_before;
    char *epic_before;
    char *active_before;
    char *index_after;
    char *epic_after;
    char *active_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_grill(&fx), 0);
    ASSERT_EQ(sb_http_seed_flag(&fx, SB_HTTP_GRILL_EPIC_ID, 1), CBM_STORE_OK);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", fx.root_dir);
    snprintf(epic_path, sizeof(epic_path), "%s/.grill/plans/inbox-plan/epics/epic-001-inbox.md",
             fx.root_dir);
    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    index_before = th_read_file_alloc(index_path);
    epic_before = th_read_file_alloc(epic_path);
    active_before = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(index_before);
    ASSERT_NOT_NULL(epic_before);
    ASSERT_NOT_NULL(active_before);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"grill_skill_present\":true"));
    ASSERT_TRUE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"kind\":\"epic\""));
    ASSERT_TRUE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"column\":\"todo\""));
    ASSERT_TRUE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"title\":\"inbox\""));
    ASSERT_TRUE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"summary\":\"Filter unread first.\""));
    ASSERT_TRUE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"plan_title\":\"Inbox Plan\""));
    ASSERT_FALSE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"archived\""));
    ASSERT_FALSE(sb_json_specs_contain_id(json, SB_HTTP_GRILL_EPIC_ID));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-010-aaa-planned", "\"column\":\"todo\""));
    ASSERT_FALSE(sb_json_obj_has(json, "spec-010-aaa-planned", "\"kind\""));
    ASSERT_NULL(strstr(json, "\"has_more\""));
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));

    index_after = th_read_file_alloc(index_path);
    epic_after = th_read_file_alloc(epic_path);
    active_after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(index_after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_NOT_NULL(active_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_STR_EQ(epic_before, epic_after);
    ASSERT_STR_EQ(active_before, active_after);
    free(index_before);
    free(epic_before);
    free(active_before);
    free(index_after);
    free(epic_after);
    free(active_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_404_epic_id_writes_nothing) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char epic_path[1024];
    char active_path[1024];
    char *epic_before;
    char *active_before;
    char *epic_after;
    char *active_after;
    char post_body[512];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_grill(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"kind\":\"epic\""));
    ASSERT_FALSE(sb_json_obj_has(json, SB_HTTP_GRILL_EPIC_ID, "\"archived\""));
    ASSERT_FALSE(sb_json_specs_contain_id(json, SB_HTTP_GRILL_EPIC_ID));

    snprintf(epic_path, sizeof(epic_path), "%s/.grill/plans/inbox-plan/epics/epic-001-inbox.md",
             fx.root_dir);
    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    epic_before = th_read_file_alloc(epic_path);
    active_before = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(epic_before);
    ASSERT_NOT_NULL(active_before);

    snprintf(post_body, sizeof(post_body),
             "{\"project\":\"alpha\",\"spec_id\":\"%s\",\"archived\":true}", SB_HTTP_GRILL_EPIC_ID);
    ASSERT_GT(ui_spec_board_post(&ts, post_body, resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"spec not found\"}");
    ASSERT_EQ(sb_http_archive_has_id(&fx, SB_HTTP_GRILL_EPIC_ID), 0);

    epic_after = th_read_file_alloc(epic_path);
    active_after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_NOT_NULL(active_after);
    ASSERT_STR_EQ(epic_before, epic_after);
    ASSERT_STR_EQ(active_before, active_after);
    free(epic_before);
    free(active_before);
    free(epic_after);
    free(active_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_404_unknown_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "missing-proj", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"project not found\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_200_leaves_skill_trees) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    char index_path[1024];
    char epic_path[1024];
    char active_path[1024];
    char debt_path[1024];
    char *index_before;
    char *epic_before;
    char *active_before;
    char *debt_before;
    char *index_after;
    char *epic_after;
    char *active_after;
    char *debt_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_grill(&fx), 0);
    ASSERT_EQ(sb_http_seed_tech_debt(&fx, "## TD-005: leftover cache\nStatus: identified\n"), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", fx.root_dir);
    snprintf(epic_path, sizeof(epic_path), "%s/.grill/plans/inbox-plan/epics/epic-001-inbox.md",
             fx.root_dir);
    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    snprintf(debt_path, sizeof(debt_path), "%s/.sdd-skill/baseline/TECH_DEBT.md", fx.root_dir);
    index_before = th_read_file_alloc(index_path);
    epic_before = th_read_file_alloc(epic_path);
    active_before = th_read_file_alloc(active_path);
    debt_before = th_read_file_alloc(debt_path);
    ASSERT_NOT_NULL(index_before);
    ASSERT_NOT_NULL(epic_before);
    ASSERT_NOT_NULL(active_before);
    ASSERT_NOT_NULL(debt_before);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);

    index_after = th_read_file_alloc(index_path);
    epic_after = th_read_file_alloc(epic_path);
    active_after = th_read_file_alloc(active_path);
    debt_after = th_read_file_alloc(debt_path);
    ASSERT_NOT_NULL(index_after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_NOT_NULL(active_after);
    ASSERT_NOT_NULL(debt_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_STR_EQ(epic_before, epic_after);
    ASSERT_STR_EQ(active_before, active_after);
    ASSERT_STR_EQ(debt_before, debt_after);
    free(index_before);
    free(epic_before);
    free(active_before);
    free(debt_before);
    free(index_after);
    free(epic_after);
    free(active_after);
    free(debt_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

/**
 * @sdd-task: Task #2 - HTTP GET additive + POST leftover locks
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-065 same GET always-emit debt; POST stays spec-only
 * @sdd-why: prove existing handle_spec_board_get dispatch (read → specs-only merge → to_json)
 * @human-debug: GET 200 missing debt → to_json not Task #1; POST epic 200 → find walked epics[]
 */
TEST(ui_spec_board_get_200_open_heading_debt) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char debt_buf[1024];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_flag(&fx, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    ASSERT_EQ(sb_http_seed_tech_debt(&fx,
                                     "# Tech Debt\n\n"
                                     "## TD-005: leftover cache\n"
                                     "Title: wrong title from field\n"
                                     "Status: identified\n"
                                     "Description: leftover cache body.\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 1);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[{\"id\":\"TD-005\",\"title\":\"leftover cache\"}]"));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_TRUE(sb_json_spec_has(json, "spec-012-ccc-closed", "\"archived\":true"));
    ASSERT_EQ(sb_json_debt_copy(json, debt_buf, sizeof(debt_buf)), 0);
    ASSERT_NULL(strstr(debt_buf, "archived"));
    ASSERT_NULL(strstr(debt_buf, "severity"));
    ASSERT_NULL(strstr(debt_buf, "category"));
    ASSERT_NULL(strstr(debt_buf, "\"status\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_200_17th_debt_omitted) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char body[4096];
    int n = 0;
    int i;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    for (i = 1; i <= 17; i++) {
        n += snprintf(body + n, sizeof(body) - (size_t)n,
                      "## TD-%03d: item %d\nStatus: identified\n\n", i, i);
    }
    ASSERT_EQ(sb_http_seed_tech_debt(&fx, body), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 16);
    ASSERT_NOT_NULL(strstr(json, "TD-016"));
    ASSERT_NULL(strstr(json, "TD-017"));
    ASSERT_NULL(strstr(json, "has_more"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

/**
 * @sdd-task: Task #2 - C Inbox walk + conversion + HTTP bytes
 * @sdd-spec: specs/spec-011-q5n-game-phase-board/spec.md
 * @sdd-decision: SDD-ADR-046, SDD-ADR-047
 * @sdd-why: GET inbox objects + conversion + zero skill writes; 400/404 stay
 * @human-debug: If GET 500 → calloc/to_json; if Inbox empty with leftover grill → conversion
 */
static int ui_game_board_get(th_server_t *ts, const char *project, char *resp, size_t respsz) {
    char req[512];
    int n;
    if (project && project[0]) {
        n = snprintf(req, sizeof(req), "GET /api/game-board?project=%s HTTP/1.1\r\n\r\n", project);
    } else {
        n = snprintf(req, sizeof(req), "GET /api/game-board HTTP/1.1\r\n\r\n");
    }
    if (n < 0 || (size_t)n >= sizeof(req)) {
        return 0;
    }
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

static const char *GB_HTTP_INBOX_EPIC_ID =
    ".grill/plans/inbox-plan/epics/epic-001-inbox.md";

static int gb_http_seed_bevy(ui_delete_fixture_t *fx) {
    if (th_mkdir_p(TH_PATH(fx->root_dir, ".gamedev")) != 0) {
        return -1;
    }
    if (!http_write_project_db(fx->cache_dir, "bevy", fx->root_dir, NULL)) {
        return -1;
    }
    return 0;
}

static int gb_http_seed_inbox_grill(ui_delete_fixture_t *fx) {
    if (th_write_file(TH_PATH(fx->root_dir, ".grill/index.md"),
                      "# .grill/\n\n"
                      "| slug | title | status |\n"
                      "|------|-------|--------|\n"
                      "| inbox-plan | Inbox Plan | draft |\n") != 0) {
        return -1;
    }
    return th_write_file(TH_PATH(fx->root_dir, ".grill/plans/inbox-plan/epics/epic-001-inbox.md"),
                         "name: inbox\nstatus: pending\n\nsummary: Filter unread first.\n");
}

/**
 * @sdd-task: Task #2 - HTTP GET leftover locks
 * @sdd-spec: specs/spec-016-d9v-game-inbox-registry/spec.md
 * @sdd-decision: SDD-ADR-069 same GET omit; SDD-ADR-070 parse stays in game_board.c
 * @sdd-why: HTTP proves registry omit + 404 + bytes + no create; spec-board must not read registry
 * @human-debug: GET still lists a hide-set leftover → fill_inbox not reached; spec-board omit → spec_board opened the registry
 */
static int gb_http_seed_registry(ui_delete_fixture_t *fx, const char *rows) {
    char body[8192];
    snprintf(body, sizeof(body),
             "| Epic | Plan | Name | Origin | Status |\n"
             "|------|------|------|--------|--------|\n"
             "%s",
             rows ? rows : "");
    return th_write_file(TH_PATH(fx->root_dir, ".gamedev/epics_registry.md"), body);
}

static int gb_http_seed_active_json(ui_delete_fixture_t *fx) {
    return th_write_file(TH_PATH(fx->root_dir, ".sdd-skill/specs/active.json"), SB_HTTP_ACTIVE);
}

/**
 * @sdd-task: Task #3 - HTTP POST + GET merge + C Gherkin + publish copy
 * @sdd-spec: specs/spec-012-m2k-game-expand-archive-deps/spec.md
 * @sdd-decision: SDD-ADR-053 POST /api/game-board; HTTP merge; publish copy
 * @sdd-why: C owns merge/409/404/400/idempotent/orphan/zero writes; fixtures stay under /tmp
 * @human-debug: 409 still persisted → POST wrote before the done-after-overlay check
 */
static const char *GB_HTTP_AUDIO_ID = ".gamedev/phases/01-preproduction/audio-direction.md";
static const char *GB_HTTP_GDD_ID = ".gamedev/phases/01-preproduction/gdd.md";
static const char *GB_HTTP_SYS_ID = ".gamedev/phases/02-production/systems/SYS-001-movement";
static const char *GB_HTTP_ORPHAN_ID = ".gamedev/phases/01-preproduction/missing-doc.md";

static int ui_game_board_post(th_server_t *ts, const char *body, char *resp, size_t respsz) {
    char req[4608];
    int n = snprintf(req, sizeof(req),
                     "POST /api/game-board HTTP/1.1\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %zu\r\n\r\n%s",
                     strlen(body), body);
    if (n < 0 || (size_t)n >= sizeof(req)) {
        return 0;
    }
    return th_http(cbm_http_server_port(ts->srv), req, resp, respsz);
}

static int gb_http_seed_flag(const ui_delete_fixture_t *fx, const char *card_id, int archived) {
    char db_path[1024];
    cbm_store_t *st;
    int rc;

    ui_delete_db_path(fx, "bevy", db_path, sizeof(db_path));
    st = cbm_store_open_path(db_path);
    if (!st) {
        return CBM_STORE_ERR;
    }
    rc = cbm_store_game_archive_set(st, card_id, archived);
    cbm_store_close(st);
    return rc;
}

static int gb_http_archive_has_id(const ui_delete_fixture_t *fx, const char *card_id) {
    char db_path[1024];
    cbm_store_t *st;
    cbm_game_archive_row_t rows[16];
    int n = 0;
    int i;

    ui_delete_db_path(fx, "bevy", db_path, sizeof(db_path));
    st = cbm_store_open_path_query(db_path);
    if (!st) {
        return -1;
    }
    if (cbm_store_game_archive_load(st, rows, 16, &n) != CBM_STORE_OK) {
        cbm_store_close(st);
        return -1;
    }
    cbm_store_close(st);
    for (i = 0; i < n; i++) {
        if (strcmp(rows[i].card_id, card_id) == 0) {
            return 1;
        }
    }
    return 0;
}

static int gb_http_seed_done_audio(ui_delete_fixture_t *fx) {
    return th_write_file(TH_PATH(fx->root_dir, ".gamedev/phases/01-preproduction/audio-direction.md"),
                         "status: approved\n");
}

static int gb_http_seed_pending_gdd(ui_delete_fixture_t *fx) {
    return th_write_file(TH_PATH(fx->root_dir, ".gamedev/phases/01-preproduction/gdd.md"),
                         "status: draft\n");
}

static int gb_http_seed_sys_done(ui_delete_fixture_t *fx) {
    return th_write_file(
        TH_PATH(fx->root_dir, ".gamedev/phases/02-production/systems/SYS-001-movement/spec.md"),
        "status: approved\n");
}

static int gb_http_seed_blocked_state(ui_delete_fixture_t *fx) {
    return th_write_file(TH_PATH(fx->root_dir, ".gamedev/state.md"),
                         "phase=02-production focus=\"x\"\n"
                         "gameplay-engineer:blocked:\"Combat system v2\":\"Waiting on final boss design\"\n");
}

/**
 * @sdd-task: Task #2 - HTTP GET additive + POST leftover locks
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 same GET always-emit debt; SDD-ADR-073 HTTP does not fopen backlog.md
 * @sdd-why: Prove GET debt JSON + no has_more + 404 + bytes + no create; spec-board stays TECH_DEBT.md
 * @human-debug: GET 200 missing debt → to_json not Task #1; GET created backlog.md → HTTP wrote; spec-board has debt:gate → spec_board opened backlog.md
 */
static int gb_http_seed_backlog(ui_delete_fixture_t *fx, const char *body) {
    return th_write_file(TH_PATH(fx->root_dir, ".gamedev/backlog.md"), body);
}

TEST(ui_game_board_get_200_present_true_empty_arrays) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[8192];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"gamedev_skill_present\":true"));
    ASSERT_NOT_NULL(strstr(json, "\"inbox\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"preproduction\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"production\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"postproduction\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"continue\":\"/gamedev-skill continue\""));
    ASSERT_NULL(strstr(json, "has_more"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_present_false_empty_arrays) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[8192];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT(http_write_project_db(fx.cache_dir, "alpha", fx.root_dir, NULL));
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"gamedev_skill_present\":false"));
    ASSERT_NOT_NULL(strstr(json, "\"inbox\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"preproduction\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"production\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"postproduction\":[]"));
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_NULL(strstr(json, "has_more"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_200_no_gamedev_field_when_gamedev_dir) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_404_unknown_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "missing", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"project not found\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_400_missing_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, NULL, resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 400);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"missing project parameter\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_unconverted_inbox_epic) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".gamedev/roadmap.md"), "no 001 cell\n"), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"kind\":\"epic\""));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"title\":\"inbox\""));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"summary\":\"Filter unread first.\""));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"plan_title\":\"Inbox Plan\""));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"track\":null"));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"work_state\":null"));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"owner\":\"\""));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"continue\":\"/gamedev-skill continue\""));
    ASSERT_NULL(strstr(json, "\"has_more\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_companion_to_omits_epic_bytes) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;
    char epic_path[1024];
    char *before;
    char *after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".gamedev/phases/01-preproduction/gdd.md"),
                            "Companion to: .grill/plans/inbox-plan/epics/epic-001-inbox.md\n"),
              0);
    snprintf(epic_path, sizeof(epic_path), "%s/.grill/plans/inbox-plan/epics/epic-001-inbox.md",
             fx.root_dir);
    before = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(before);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_FALSE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"kind\":\"epic\""));

    after = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(before);
    free(after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_leaves_skill_trees) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    char state_path[1024];
    char index_path[1024];
    char epic_path[1024];
    char *state_before;
    char *index_before;
    char *epic_before;
    char *state_after;
    char *index_after;
    char *epic_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".gamedev/state.md"),
                            "phase=02-production focus=\"keep me\"\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", fx.root_dir);
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", fx.root_dir);
    snprintf(epic_path, sizeof(epic_path), "%s/.grill/plans/inbox-plan/epics/epic-001-inbox.md",
             fx.root_dir);
    state_before = th_read_file_alloc(state_path);
    index_before = th_read_file_alloc(index_path);
    epic_before = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(state_before);
    ASSERT_NOT_NULL(index_before);
    ASSERT_NOT_NULL(epic_before);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);

    state_after = th_read_file_alloc(state_path);
    index_after = th_read_file_alloc(index_path);
    epic_after = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(state_after);
    ASSERT_NOT_NULL(index_after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_STR_EQ(epic_before, epic_after);
    ASSERT_FALSE(cbm_is_dir(TH_PATH(fx.root_dir, ".sdd-skill")));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".sdd-skill")));
    free(state_before);
    free(index_before);
    free(epic_before);
    free(state_after);
    free(index_after);
    free(epic_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_registry_in_progress_omits) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;
    char reg_path[1024];
    char state_path[1024];
    char index_path[1024];
    char active_path[1024];
    char *reg_before;
    char *state_before;
    char *index_before;
    char *active_before;
    char *reg_after;
    char *state_after;
    char *index_after;
    char *active_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(gb_http_seed_registry(&fx, "| 001 | inbox-plan | Inbox | o | in_progress |\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".gamedev/state.md"),
                            "phase=02-production focus=\"keep me\"\n"),
              0);
    ASSERT_EQ(gb_http_seed_active_json(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(reg_path, sizeof(reg_path), "%s/.gamedev/epics_registry.md", fx.root_dir);
    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", fx.root_dir);
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", fx.root_dir);
    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    reg_before = th_read_file_alloc(reg_path);
    state_before = th_read_file_alloc(state_path);
    index_before = th_read_file_alloc(index_path);
    active_before = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(reg_before);
    ASSERT_NOT_NULL(state_before);
    ASSERT_NOT_NULL(index_before);
    ASSERT_NOT_NULL(active_before);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_FALSE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"kind\":\"epic\""));
    ASSERT_NULL(strstr(json, GB_HTTP_INBOX_EPIC_ID));
    ASSERT_NULL(strstr(json, "\"has_more\""));
    ASSERT_NULL(strstr(json, "epics_registry"));
    ASSERT_NULL(strstr(json, "\"registry\""));

    reg_after = th_read_file_alloc(reg_path);
    state_after = th_read_file_alloc(state_path);
    index_after = th_read_file_alloc(index_path);
    active_after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(reg_after);
    ASSERT_NOT_NULL(state_after);
    ASSERT_NOT_NULL(index_after);
    ASSERT_NOT_NULL(active_after);
    ASSERT_STR_EQ(reg_before, reg_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_STR_EQ(active_before, active_after);
    free(reg_before);
    free(state_before);
    free(index_before);
    free(active_before);
    free(reg_after);
    free(state_after);
    free(index_after);
    free(active_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_absent_registry_does_not_create) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;
    char *state_before;
    char *index_before;
    char *active_before;
    char *state_after;
    char *index_after;
    char *active_after;
    char state_path[1024];
    char index_path[1024];
    char active_path[1024];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".gamedev/state.md"),
                            "phase=02-production focus=\"keep me\"\n"),
              0);
    ASSERT_EQ(gb_http_seed_active_json(&fx), 0);
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/epics_registry.md")));
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", fx.root_dir);
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", fx.root_dir);
    snprintf(active_path, sizeof(active_path), "%s/.sdd-skill/specs/active.json", fx.root_dir);
    state_before = th_read_file_alloc(state_path);
    index_before = th_read_file_alloc(index_path);
    active_before = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(state_before);
    ASSERT_NOT_NULL(index_before);
    ASSERT_NOT_NULL(active_before);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"kind\":\"epic\""));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/epics_registry.md")));
    ASSERT_NULL(strstr(json, "\"has_more\""));
    ASSERT_NULL(strstr(json, "epics_registry"));
    ASSERT_NULL(strstr(json, "\"registry\""));

    state_after = th_read_file_alloc(state_path);
    index_after = th_read_file_alloc(index_path);
    active_after = th_read_file_alloc(active_path);
    ASSERT_NOT_NULL(state_after);
    ASSERT_NOT_NULL(index_after);
    ASSERT_NOT_NULL(active_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_STR_EQ(active_before, active_after);
    free(state_before);
    free(index_before);
    free(active_before);
    free(state_after);
    free(index_after);
    free(active_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_200_closed_registry_epic_still_listed) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(gb_http_seed_registry(&fx, "| 001 | inbox-plan | Inbox | o | closed |\n"), 0);
    ASSERT_EQ(gb_http_seed_active_json(&fx), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".sdd-skill/specs/spec-010-aaa-planned/spec.md"),
                            "# Spec-010-aaa: Planned\n\nNo Companion-to grill path.\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_INBOX_EPIC_ID, "\"kind\":\"epic\""));
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_200_flag_object_and_idempotent) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char *before;
    char *after;
    char *epic_before;
    char *epic_after;
    char audio_path[1024];
    char epic_path[1024];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_done_audio(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(audio_path, sizeof(audio_path), "%s/%s", fx.root_dir, GB_HTTP_AUDIO_ID);
    snprintf(epic_path, sizeof(epic_path), "%s/%s", fx.root_dir, GB_HTTP_INBOX_EPIC_ID);
    before = th_read_file_alloc(audio_path);
    epic_before = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(before);
    ASSERT_NOT_NULL(epic_before);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/audio-direction.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"card_id\":\".gamedev/phases/01-preproduction/audio-direction.md\""));
    ASSERT_NOT_NULL(strstr(json, "\"archived\":true"));
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));
    ASSERT_NULL(strstr(json, "\"preproduction\""));

    after = th_read_file_alloc(audio_path);
    epic_after = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_STR_EQ(before, after);
    ASSERT_STR_EQ(epic_before, epic_after);
    free(after);
    free(epic_after);
    ASSERT_FALSE(cbm_is_dir(TH_PATH(fx.root_dir, ".sdd-skill")));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".sdd-skill")));

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/audio-direction.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"archived\":true"));

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_AUDIO_ID, "\"archived\":true"));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_AUDIO_ID, "\"work_state\":\"done\""));

    after = th_read_file_alloc(audio_path);
    epic_after = th_read_file_alloc(epic_path);
    ASSERT_NOT_NULL(after);
    ASSERT_NOT_NULL(epic_after);
    ASSERT_STR_EQ(before, after);
    ASSERT_STR_EQ(epic_before, epic_after);
    free(after);
    free(epic_after);
    free(before);
    free(epic_before);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_404_inbox_epic) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".grill/plans/inbox-plan/epics/epic-001-inbox.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"card not found\"}");
    ASSERT_EQ(gb_http_archive_has_id(&fx, GB_HTTP_INBOX_EPIC_ID), 0);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_merges_leftover_orphan) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_pending_gdd(&fx), 0);
    ASSERT_EQ(gb_http_seed_flag(&fx, GB_HTTP_GDD_ID, 1), CBM_STORE_OK);
    ASSERT_EQ(gb_http_seed_flag(&fx, GB_HTTP_ORPHAN_ID, 1), CBM_STORE_OK);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_GDD_ID, "\"work_state\":\"pending\""));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_GDD_ID, "\"archived\":true"));
    ASSERT_NULL(strstr(json, GB_HTTP_ORPHAN_ID));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_409_pending_writes_nothing) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char *before;
    char *after;
    char gdd_path[1024];

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_pending_gdd(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(gdd_path, sizeof(gdd_path), "%s/%s", fx.root_dir, GB_HTTP_GDD_ID);
    before = th_read_file_alloc(gdd_path);
    ASSERT_NOT_NULL(before);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/gdd.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 409);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"card not done\"}");

    after = th_read_file_alloc(gdd_path);
    ASSERT_NOT_NULL(after);
    ASSERT_STR_EQ(before, after);
    free(before);
    free(after);
    ASSERT_EQ(gb_http_archive_has_id(&fx, GB_HTTP_GDD_ID), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_GDD_ID, "\"archived\":false"));
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_GDD_ID, "\"work_state\":\"pending\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_409_blocked_overlay) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_sys_done(&fx), 0);
    ASSERT_EQ(gb_http_seed_blocked_state(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/02-production/systems/SYS-001-movement\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 409);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"card not done\"}");
    ASSERT_EQ(gb_http_archive_has_id(&fx, GB_HTTP_SYS_ID), 0);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_404_unknown_card) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/nope.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"card not found\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_400_missing_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"card_id\":\".gamedev/phases/01-preproduction/"
                                 "audio-direction.md\",\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 400);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"error\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_400_invalid_archived) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/audio-direction.md\","
                                 "\"archived\":\"yes\"}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 400);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"invalid archived\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_404_unknown_project) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"missing-proj\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/audio-direction.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"project not found\"}");

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_post_game_card_id_does_not_archive) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_done_audio(&fx), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_post(&ts,
                                 "{\"project\":\"bevy\",\"spec_id\":"
                                 "\".gamedev/phases/01-preproduction/audio-direction.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"spec not found\"}");
    ASSERT_EQ(gb_http_archive_has_id(&fx, GB_HTTP_AUDIO_ID), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(sb_json_obj_has(json, GB_HTTP_AUDIO_ID, "\"archived\":false"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(pipeline_publish_staged_copies_spec_archive) {
    char *td = th_mktempdir("cbm_pub_arch");
    char live[512];
    char stage[512];
    cbm_store_t *st;
    cbm_pipeline_generation_t gen;
    char *stage_owned;
    cbm_spec_archive_row_t rows[CBM_SPEC_ARCHIVE_CAP];
    int n = 0;

    ASSERT_NOT_NULL(td);
    snprintf(live, sizeof(live), "%s/final.db", td);
    snprintf(stage, sizeof(stage), "%s/stage.db", td);

    st = cbm_store_open_path(live);
    ASSERT_NOT_NULL(st);
    ASSERT_EQ(cbm_store_upsert_project(st, "pubarch", td), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_spec_archive_set(st, "spec-012-ccc-closed", 1), CBM_STORE_OK);
    cbm_store_close(st);

    st = cbm_store_open_path(stage);
    ASSERT_NOT_NULL(st);
    ASSERT_EQ(cbm_store_upsert_project(st, "pubarch", td), CBM_STORE_OK);
    cbm_store_close(st);

    memset(&gen, 0, sizeof(gen));
    gen.final_db_path = live;
    gen.project = "pubarch";
    gen.surfaces_in_place = true;

    stage_owned = strdup(stage);
    ASSERT_NOT_NULL(stage_owned);
    ASSERT_EQ(cbm_pipeline_publish_staged(stage_owned, &gen, false, true), 0);

    st = cbm_store_open_path_query(live);
    ASSERT_NOT_NULL(st);
    ASSERT_EQ(cbm_store_spec_archive_load(st, rows, CBM_SPEC_ARCHIVE_CAP, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    ASSERT_STR_EQ(rows[0].spec_id, "spec-012-ccc-closed");
    ASSERT_EQ(rows[0].archived, 1);
    cbm_store_close(st);
    th_rmtree(td);
    PASS();
}

TEST(pipeline_publish_staged_copies_game_archive) {
    char *td = th_mktempdir("cbm_pub_game_arch");
    char live[512];
    char stage[512];
    cbm_store_t *st;
    cbm_pipeline_generation_t gen;
    char *stage_owned;
    cbm_game_archive_row_t rows[16];
    int n = 0;

    ASSERT_NOT_NULL(td);
    snprintf(live, sizeof(live), "%s/final.db", td);
    snprintf(stage, sizeof(stage), "%s/stage.db", td);

    st = cbm_store_open_path(live);
    ASSERT_NOT_NULL(st);
    ASSERT_EQ(cbm_store_upsert_project(st, "pubgarch", td), CBM_STORE_OK);
    ASSERT_EQ(cbm_store_game_archive_set(st, GB_HTTP_AUDIO_ID, 1), CBM_STORE_OK);
    cbm_store_close(st);

    st = cbm_store_open_path(stage);
    ASSERT_NOT_NULL(st);
    ASSERT_EQ(cbm_store_upsert_project(st, "pubgarch", td), CBM_STORE_OK);
    cbm_store_close(st);

    memset(&gen, 0, sizeof(gen));
    gen.final_db_path = live;
    gen.project = "pubgarch";
    gen.surfaces_in_place = true;

    stage_owned = strdup(stage);
    ASSERT_NOT_NULL(stage_owned);
    ASSERT_EQ(cbm_pipeline_publish_staged(stage_owned, &gen, false, true), 0);

    st = cbm_store_open_path_query(live);
    ASSERT_NOT_NULL(st);
    ASSERT_EQ(cbm_store_game_archive_load(st, rows, 16, &n), CBM_STORE_OK);
    ASSERT_EQ(n, 1);
    ASSERT_STR_EQ(rows[0].card_id, GB_HTTP_AUDIO_ID);
    ASSERT_EQ(rows[0].archived, 1);
    cbm_store_close(st);
    th_rmtree(td);
    PASS();
}

TEST(ui_game_board_get_200_open_comment_debt) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_backlog(&fx, "<!-- debt:gate-preproduction missing GDD lock "
                                       "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 1);
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"debt:gate-preproduction\""));
    ASSERT_NOT_NULL(strstr(json, "\"title\":\"missing GDD lock\""));
    ASSERT_NULL(strstr(json, "has_more"));
    ASSERT_NULL(strstr(json, "\"severity\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_17th_debt_omitted) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[32768];
    const char *json;
    char body[2048];
    int i;
    int n = 0;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    body[0] = '\0';
    for (i = 1; i <= 17; i++) {
        n += snprintf(body + n, sizeof(body) - (size_t)n, "- debt:d%02d item %d\n", i, i);
        ASSERT_TRUE(n > 0 && n < (int)sizeof(body));
    }
    ASSERT_EQ(gb_http_seed_backlog(&fx, body), 0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 16);
    ASSERT_NULL(strstr(json, "debt:d17"));
    ASSERT_NULL(strstr(json, "has_more"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_debt_registry_hide_unchanged) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(gb_http_seed_registry(&fx, "| 001 | inbox-plan | Inbox | o | in_progress |\n"), 0);
    ASSERT_EQ(gb_http_seed_backlog(&fx, "<!-- debt:gate-preproduction missing GDD lock "
                                       "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NULL(strstr(json, GB_HTTP_INBOX_EPIC_ID));
    ASSERT_EQ(sb_json_debt_len(json), 1);
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"debt:gate-preproduction\""));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_200_debt_from_tech_debt_not_backlog) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_tech_debt(&fx, "## TD-005: leftover cache\nStatus: identified\n"), 0);
    ASSERT_EQ(th_mkdir_p(TH_PATH(fx.root_dir, ".gamedev")), 0);
    ASSERT_EQ(gb_http_seed_backlog(&fx, "<!-- debt:gate-preproduction missing GDD lock "
                                       "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    ASSERT_TRUE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/backlog.md")));
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 1);
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"TD-005\""));
    ASSERT_NULL(strstr(json, "debt:gate-preproduction"));
    ASSERT_NULL(strstr(json, "gamedev_skill_present"));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_spec_board_get_200_without_backlog_still_tech_debt) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(sb_http_seed_board(&fx), 0);
    ASSERT_EQ(sb_http_seed_tech_debt(&fx, "## TD-005: leftover cache\nStatus: identified\n"), 0);
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/backlog.md")));
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_spec_board_get(&ts, "alpha", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 1);
    ASSERT_NOT_NULL(strstr(json, "\"id\":\"TD-005\""));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/backlog.md")));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_leaves_backlog_does_not_create) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;
    char backlog_path[1024];
    char state_path[1024];
    char reg_path[1024];
    char index_path[1024];
    char *backlog_before;
    char *state_before;
    char *reg_before;
    char *index_before;
    char *backlog_after;
    char *state_after;
    char *reg_after;
    char *index_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_inbox_grill(&fx), 0);
    ASSERT_EQ(gb_http_seed_registry(&fx, "| 001 | inbox-plan | Inbox | o | not_started |\n"), 0);
    ASSERT_EQ(th_write_file(TH_PATH(fx.root_dir, ".gamedev/state.md"),
                            "phase=02-production focus=\"keep me\"\n"),
              0);
    ASSERT_EQ(gb_http_seed_backlog(&fx, "<!-- debt:gate-preproduction missing GDD lock "
                                       "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(backlog_path, sizeof(backlog_path), "%s/.gamedev/backlog.md", fx.root_dir);
    snprintf(state_path, sizeof(state_path), "%s/.gamedev/state.md", fx.root_dir);
    snprintf(reg_path, sizeof(reg_path), "%s/.gamedev/epics_registry.md", fx.root_dir);
    snprintf(index_path, sizeof(index_path), "%s/.grill/index.md", fx.root_dir);
    backlog_before = th_read_file_alloc(backlog_path);
    state_before = th_read_file_alloc(state_path);
    reg_before = th_read_file_alloc(reg_path);
    index_before = th_read_file_alloc(index_path);
    ASSERT_NOT_NULL(backlog_before);
    ASSERT_NOT_NULL(state_before);
    ASSERT_NOT_NULL(reg_before);
    ASSERT_NOT_NULL(index_before);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_EQ(sb_json_debt_len(json), 1);

    backlog_after = th_read_file_alloc(backlog_path);
    state_after = th_read_file_alloc(state_path);
    reg_after = th_read_file_alloc(reg_path);
    index_after = th_read_file_alloc(index_path);
    ASSERT_NOT_NULL(backlog_after);
    ASSERT_NOT_NULL(state_after);
    ASSERT_NOT_NULL(reg_after);
    ASSERT_NOT_NULL(index_after);
    ASSERT_STR_EQ(backlog_before, backlog_after);
    ASSERT_STR_EQ(state_before, state_after);
    ASSERT_STR_EQ(reg_before, reg_after);
    ASSERT_STR_EQ(index_before, index_after);
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".sdd-skill")));
    free(backlog_before);
    free(state_before);
    free(reg_before);
    free(index_before);
    free(backlog_after);
    free(state_after);
    free(reg_after);
    free(index_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_get_200_absent_backlog_does_not_create) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[16384];
    const char *json;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/backlog.md")));
    ASSERT_EQ(th_server_start(&ts), 0);

    ASSERT_GT(ui_game_board_get(&ts, "bevy", resp, sizeof(resp)), 0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"debt\":[]"));
    ASSERT_FALSE(cbm_file_exists(TH_PATH(fx.root_dir, ".gamedev/backlog.md")));

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_200_archive_leaves_backlog) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[65536];
    const char *json;
    char backlog_path[1024];
    char *backlog_before;
    char *backlog_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_done_audio(&fx), 0);
    ASSERT_EQ(gb_http_seed_backlog(&fx, "<!-- debt:gate-preproduction missing GDD lock "
                                       "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(backlog_path, sizeof(backlog_path), "%s/.gamedev/backlog.md", fx.root_dir);
    backlog_before = th_read_file_alloc(backlog_path);
    ASSERT_NOT_NULL(backlog_before);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":"
                                 "\".gamedev/phases/01-preproduction/audio-direction.md\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 200);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_NOT_NULL(strstr(json, "\"archived\":true"));

    backlog_after = th_read_file_alloc(backlog_path);
    ASSERT_NOT_NULL(backlog_after);
    ASSERT_STR_EQ(backlog_before, backlog_after);
    free(backlog_before);
    free(backlog_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

TEST(ui_game_board_post_404_debt_id_not_archive_target) {
    ui_delete_fixture_t fx;
    th_server_t ts;
    char resp[4096];
    const char *json;
    char backlog_path[1024];
    char *backlog_before;
    char *backlog_after;

    ASSERT_EQ(ui_delete_fixture_init(&fx), 0);
    ASSERT_EQ(gb_http_seed_bevy(&fx), 0);
    ASSERT_EQ(gb_http_seed_backlog(&fx, "<!-- debt:gate-preproduction missing GDD lock "
                                       "\xe2\x80\x94 director \xe2\x80\x94 M1 -->\n"),
              0);
    ASSERT_EQ(th_server_start(&ts), 0);

    snprintf(backlog_path, sizeof(backlog_path), "%s/.gamedev/backlog.md", fx.root_dir);
    backlog_before = th_read_file_alloc(backlog_path);
    ASSERT_NOT_NULL(backlog_before);

    ASSERT_GT(ui_game_board_post(&ts,
                                 "{\"project\":\"bevy\",\"card_id\":\"debt:gate-preproduction\","
                                 "\"archived\":true}",
                                 resp, sizeof(resp)),
              0);
    ASSERT_EQ(th_status(resp), 404);
    json = th_http_json(resp);
    ASSERT_NOT_NULL(json);
    ASSERT_STR_EQ(json, "{\"error\":\"card not found\"}");
    ASSERT_EQ(gb_http_archive_has_id(&fx, "debt:gate-preproduction"), 0);

    backlog_after = th_read_file_alloc(backlog_path);
    ASSERT_NOT_NULL(backlog_after);
    ASSERT_STR_EQ(backlog_before, backlog_after);
    free(backlog_before);
    free(backlog_after);

    th_server_stop(&ts);
    ui_delete_fixture_cleanup(&fx);
    PASS();
}

/* ── Suite ────────────────────────────────────────────────────── */

SUITE(httpd) {
    RUN_TEST(ui_server_browse_wide_dir_no_overflow);
    RUN_TEST(ui_server_logs_escape_dense_no_overflow);
    RUN_TEST(ui_server_index_status_long_paths_no_overflow);
    /* Parser / helpers */
    RUN_TEST(httpd_parse_simple_get);
    RUN_TEST(httpd_parse_security_headers_and_rejects_duplicates);
    RUN_TEST(httpd_parse_post_with_body_offset);
    RUN_TEST(httpd_parse_origin_case_insensitive);
    RUN_TEST(httpd_parse_rejects_bare_lf);
    RUN_TEST(httpd_parse_rejects_chunked);
    RUN_TEST(httpd_parse_rejects_oversized_content_length);
    RUN_TEST(httpd_parse_rejects_garbage_content_length);
    RUN_TEST(httpd_parse_rejects_percent00_in_target);
    RUN_TEST(httpd_parse_rejects_raw_nul_in_head);
    RUN_TEST(httpd_parse_incomplete_head_needs_more);
    RUN_TEST(httpd_parse_rejects_missing_version);
    RUN_TEST(httpd_parse_rejects_oversized_head);
    RUN_TEST(httpd_query_param_decode);
    RUN_TEST(httpd_query_param_edge_cases);
    RUN_TEST(httpd_path_match_matrix);
    RUN_TEST(httpd_resolves_bare_binary_path_from_path);
    RUN_TEST(repo_info_web_base_normalizes_to_https);
    RUN_TEST(repo_info_strips_credentials_from_remote);

    /* Transport */
    RUN_TEST(httpd_listen_ephemeral_port);
    RUN_TEST(httpd_listen_port_collision_returns_null);
    RUN_TEST(httpd_close_refuses_while_connection_owns_listener);

    /* Full UI server */
    RUN_TEST(ui_server_readiness_proof_is_exact_and_generation_bound);
    RUN_TEST(ui_server_rejects_non_loopback_host);
    RUN_TEST(ui_server_unknown_path_404);
    RUN_TEST(ui_server_process_kill_route_is_unavailable);
    RUN_TEST(ui_server_routes_indexing_through_joinable_daemon_executor);
    RUN_TEST(ui_index_owned_path_is_409_path_exists);
    RUN_TEST(ui_index_trailing_slash_is_409_path_exists);
    RUN_TEST(ui_index_derived_name_other_path_is_409_name_exists);
    RUN_TEST(ui_index_reindex_project_is_202);
    RUN_TEST(ui_index_reindex_project_name_alias_is_202);
    RUN_TEST(ui_index_tie_existing_project_is_greater_name);
    RUN_TEST(ui_index_inflight_second_create_is_409);
    RUN_TEST(ui_server_free_never_joins_active_index_worker);
    RUN_TEST(ui_server_root_without_embedded_assets_is_not_found);
    RUN_TEST(ui_server_same_origin_request_is_allowed);
    RUN_TEST(ui_server_rejects_foreign_and_null_origins);
    RUN_TEST(ui_server_mutations_require_json_content_type);
    RUN_TEST(ui_server_rpc_allows_only_ui_read_tools);
    RUN_TEST(ui_server_oversized_body_rejected);
    RUN_TEST(ui_server_encoded_slash_not_routed);
    RUN_TEST(ui_server_nul_in_target_rejected);
    RUN_TEST(ui_server_browse_traversal_probe);
    RUN_TEST(ui_server_adr_mutation_guard_busy_preserves_existing_adr);
    RUN_TEST(ui_server_adr_mutation_guard_balances_success);
    RUN_TEST(ui_index_reindex_fills_adr_and_migrates_unmarked);
    RUN_TEST(ui_index_create_fills_generated_empty_manual);
    RUN_TEST(ui_index_no_sdd_skill_leaves_adr_unmarked);
    RUN_TEST(ui_index_partial_context_ai_job_succeeds);
    RUN_TEST(ui_index_unreadable_architecture_omits_extract);
    RUN_TEST(ui_adr_generated_hand_edit_replaced_on_reindex);
    RUN_TEST(ui_adr_post_body_max_32768);
    RUN_TEST(ui_index_reindex_fills_gamedev_omits_leftover_sdd);
    RUN_TEST(ui_index_create_gamedev_fills_generated_empty_manual);
    RUN_TEST(ui_index_add_gamedev_overwrites_sdd_keeps_manual);
    RUN_TEST(ui_index_gamedev_only_tech_stack_no_sdd_fallback);
    RUN_TEST(ui_index_empty_gamedev_dir_no_sdd_fallback);
    RUN_TEST(ui_index_remove_gamedev_restores_sdd);
    RUN_TEST(ui_index_both_skill_dirs_gone_leaves_last_blob);
    RUN_TEST(ui_index_unreadable_game_context_omits_purpose);
    RUN_TEST(ui_adr_gamedev_generated_hand_edit_replaced_on_reindex);
    RUN_TEST(ui_spec_board_get_merges_done_orphan_leftover_todo);
    RUN_TEST(ui_spec_board_post_200_flag_object_and_idempotent);
    RUN_TEST(ui_spec_board_post_409_todo_writes_nothing);
    RUN_TEST(ui_spec_board_post_404_unknown_spec);
    RUN_TEST(ui_spec_board_post_400_missing_project);
    RUN_TEST(ui_spec_board_post_400_invalid_archived);
    RUN_TEST(ui_spec_board_post_404_unknown_project);
    RUN_TEST(ui_spec_board_post_423_busy);
    RUN_TEST(ui_spec_board_get_200_mixed_todo_grill_epics);
    RUN_TEST(ui_spec_board_post_404_epic_id_writes_nothing);
    RUN_TEST(ui_spec_board_get_404_unknown_project);
    RUN_TEST(ui_spec_board_get_200_leaves_skill_trees);
    RUN_TEST(ui_spec_board_get_200_open_heading_debt);
    RUN_TEST(ui_spec_board_get_200_17th_debt_omitted);
    RUN_TEST(ui_game_board_get_200_present_true_empty_arrays);
    RUN_TEST(ui_game_board_get_200_present_false_empty_arrays);
    RUN_TEST(ui_spec_board_get_200_no_gamedev_field_when_gamedev_dir);
    RUN_TEST(ui_game_board_get_404_unknown_project);
    RUN_TEST(ui_game_board_get_400_missing_project);
    RUN_TEST(ui_game_board_get_200_unconverted_inbox_epic);
    RUN_TEST(ui_game_board_get_200_companion_to_omits_epic_bytes);
    RUN_TEST(ui_game_board_get_200_leaves_skill_trees);
    RUN_TEST(ui_game_board_get_200_registry_in_progress_omits);
    RUN_TEST(ui_game_board_get_200_absent_registry_does_not_create);
    RUN_TEST(ui_spec_board_get_200_closed_registry_epic_still_listed);
    RUN_TEST(ui_game_board_post_200_flag_object_and_idempotent);
    RUN_TEST(ui_game_board_post_404_inbox_epic);
    RUN_TEST(ui_game_board_get_merges_leftover_orphan);
    RUN_TEST(ui_game_board_post_409_pending_writes_nothing);
    RUN_TEST(ui_game_board_post_409_blocked_overlay);
    RUN_TEST(ui_game_board_post_404_unknown_card);
    RUN_TEST(ui_game_board_post_400_missing_project);
    RUN_TEST(ui_game_board_post_400_invalid_archived);
    RUN_TEST(ui_game_board_post_404_unknown_project);
    RUN_TEST(ui_game_board_get_200_open_comment_debt);
    RUN_TEST(ui_game_board_get_200_17th_debt_omitted);
    RUN_TEST(ui_game_board_get_200_debt_registry_hide_unchanged);
    RUN_TEST(ui_spec_board_get_200_debt_from_tech_debt_not_backlog);
    RUN_TEST(ui_spec_board_get_200_without_backlog_still_tech_debt);
    RUN_TEST(ui_game_board_get_200_leaves_backlog_does_not_create);
    RUN_TEST(ui_game_board_get_200_absent_backlog_does_not_create);
    RUN_TEST(ui_game_board_post_200_archive_leaves_backlog);
    RUN_TEST(ui_game_board_post_404_debt_id_not_archive_target);
    RUN_TEST(ui_spec_board_post_game_card_id_does_not_archive);
    RUN_TEST(pipeline_publish_staged_copies_spec_archive);
    RUN_TEST(pipeline_publish_staged_copies_game_archive);
    RUN_TEST(ui_server_delete_mutation_guard_busy_preserves_project);
    RUN_TEST(ui_server_delete_mutation_guard_balances_success);
    RUN_TEST(ui_server_delete_project_unwatches_after_delete);
    RUN_TEST(ui_server_delete_project_unwatches_missing_db);
    RUN_TEST(ui_server_delete_project_no_watcher_still_deletes);
    RUN_TEST(ui_server_delete_project_missing_name_keeps_watch);
    RUN_TEST(ui_server_delete_project_invalid_name_keeps_watch);
    RUN_TEST(ui_server_delete_project_unlink_failure_keeps_watch);
    RUN_TEST(ui_server_ui_config_detects_zh_accept_language);
    RUN_TEST(ui_server_ui_config_prefers_config_lang);
    RUN_TEST(ui_server_slow_request_hits_deadline);
    RUN_TEST(ui_server_access_log_redacts_query);
    RUN_TEST(ui_server_stop_joins_cleanly);
    RUN_TEST(ui_server_free_refuses_active_loop);
    RUN_TEST(ui_server_free_refuses_scheduled_run_before_child_starts);
    RUN_TEST(daemon_host_http_thread_create_failure_cancels_scheduled_run);
    RUN_TEST(httpd_interrupt_unblocks_nonreading_large_response_within_one_second);
    RUN_TEST(httpd_nonreading_large_response_hits_send_deadline_without_interrupt);
    RUN_TEST(ui_server_stop_interrupts_partial_request_within_one_second);
    /* #798 follow-up: full UI-mode hang repro under live sockets */
    RUN_TEST(ui_server_list_projects_responds_under_watchdog);
    RUN_TEST(git_context_resolve_no_hang_under_live_ui_sockets);
}
