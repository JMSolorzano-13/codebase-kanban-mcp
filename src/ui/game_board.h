/**
 * @sdd-task: Task #1 - game_board debt parse + additive JSON
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 always-emit debt[{id,title}] cap 16; SDD-ADR-073 parse backlog.md here
 * @sdd-why: Same GET; open debt:* only; do not call spec_board TECH_DEBT helper
 * @human-debug: If Game debt stays [] with an open comment → tag charset / resolved-by in body / path not regular file; if closed item appears → body ended at a blank line; if 17th shows → cap not applied
 *
 * game_board.h — Heap-only gamedev board for GET /api/game-board.
 *
 * Presence reuses cbm_spec_board_gamedev_skill_present. state.md is fopen
 * "rb" only. Phase arrays are exist-only artifact cards. Inbox is unconverted
 * grill epics (game_grill_* in game_board.c). Expand parse is local (do not
 * call cbm_spec_board_extract_blurb). archived is HTTP-merge only. Never
 * stack cbm_game_board_t.
 */
#ifndef CBM_UI_GAME_BOARD_H
#define CBM_UI_GAME_BOARD_H

#include <stdbool.h>
#include <stddef.h>

#define CBM_GAME_BOARD_MAX_CARDS 64
#define CBM_GAME_BOARD_LIST_MAX 256
#define CBM_GAME_BOARD_MAX_TASKS 48
#define CBM_GAME_BOARD_MAX_BLOCKED 16
#define CBM_GAME_BOARD_MAX_DEBT 16
#define CBM_GAME_BOARD_BLURB_MAX 512
#define CBM_GAME_BOARD_MAX_REGISTRY 256

typedef struct {
    int number; /* 1-based list order of checkbox tasks */
    char name[192];
    bool done; /* true iff [x] / [X] */
} cbm_game_board_task_t;

typedef struct {
    char owner[48];     /* "@" + slug */
    char task[256];
    char blocked_by[512];
} cbm_game_board_blocked_t;

typedef struct {
    char slug[96];
    int nnn;
    bool hide;
} cbm_game_board_registry_row_t;

typedef struct {
    char id[96];    /* full token debt:<tag> */
    char title[256];
} cbm_game_board_debt_t;

typedef struct {
    char kind[16];           /* "artifact" | "epic" */
    char id[256];            /* root-relative; dirs have no trailing slash */
    char title[256];         /* basename */
    char track[4];           /* "A"|"B"|"H"; empty → JSON null */
    char work_state[16];     /* pending|in_progress|done|blocked; empty → JSON null */
    char owner[48];          /* @role or "" */
    char continue_cmd[96];   /* JSON key "continue" on the card */
    char summary[512];       /* "" on artifacts */
    char plan_title[256];    /* "" on artifacts */
    char blurb[CBM_GAME_BOARD_BLURB_MAX];
    char inputs[CBM_GAME_BOARD_BLURB_MAX];
    char last_decision[CBM_GAME_BOARD_BLURB_MAX];
    char open[CBM_GAME_BOARD_BLURB_MAX];
    char recent[CBM_GAME_BOARD_BLURB_MAX];
    char blocked_by[CBM_GAME_BOARD_BLURB_MAX]; /* empty → JSON null */
    bool archived;           /* read leaves 0; HTTP merge later */
    cbm_game_board_task_t tasks[CBM_GAME_BOARD_MAX_TASKS];
    int task_count;
} cbm_game_board_card_t;

typedef struct {
    bool gamedev_skill_present;
    char phase[32];          /* empty → JSON null */
    char focus[512];         /* empty → JSON null */
    char continue_cmd[64];   /* JSON key "continue" (chrome) */
    cbm_game_board_blocked_t blocked[CBM_GAME_BOARD_MAX_BLOCKED];
    int blocked_count;
    cbm_game_board_card_t inbox[CBM_GAME_BOARD_MAX_CARDS];
    int inbox_count;
    cbm_game_board_card_t preproduction[CBM_GAME_BOARD_MAX_CARDS];
    int preproduction_count;
    cbm_game_board_card_t production[CBM_GAME_BOARD_MAX_CARDS];
    int production_count;
    cbm_game_board_card_t postproduction[CBM_GAME_BOARD_MAX_CARDS];
    int postproduction_count;
    cbm_game_board_debt_t debt[CBM_GAME_BOARD_MAX_DEBT];
    int debt_count;
} cbm_game_board_t;

/* Best-effort read of root/.gamedev. Always succeeds. Never writes. */
void cbm_game_board_read(const char *root_path, cbm_game_board_t *out);

/* Serialize *b to a JSON object. Caller must free(). NULL only on alloc fail. */
char *cbm_game_board_to_json(const cbm_game_board_t *b);

/* Track A blurb: first 1–2 sentences of ## What it does (spec-005 algorithm). */
void cbm_game_board_extract_what_it_does(const char *md, char *out, size_t outsz);

/* playtest-log.md: last ## Round  block through next ## Round  or EOF. */
void cbm_game_board_extract_last_round(const char *md, char *out, size_t outsz);

/* changelog.md: last 8 non-empty non-HTML-comment lines, joined by newline. */
void cbm_game_board_extract_changelog_tail(const char *md, char *out, size_t outsz);

/* Buffer parse of epics_registry.md. Does not fopen. NULL/empty md → *count 0. */
void cbm_game_board_parse_epics_registry(const char *md, cbm_game_board_registry_row_t *out,
                                         int *count);

/* Buffer parse of backlog.md debt:* entries. Does not fopen. NULL/empty md → *count 0. */
void cbm_game_board_parse_backlog_debt(const char *md, cbm_game_board_debt_t *out, int *count);

#endif /* CBM_UI_GAME_BOARD_H */
