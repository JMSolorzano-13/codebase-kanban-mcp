/**
 * @sdd-task: Task #4 - Dashboard page: list + Control + create-index
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-002 - Stacked Dashboard: list then Control, one ScrollArea
 * @sdd-why: Task #4 Gherkin — identity rows, path-only index, Control polls, empty/error
 * @human-debug: If 202 calls onSelectProject → create wired navigate; if polls=1 → fake timers not advanced
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { act, cleanup, fireEvent, render, screen, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { messages } from "../lib/i18n";
import { Dashboard } from "./Dashboard";

function rpcToolName(init?: RequestInit): string | undefined {
  if (!init?.body) return undefined;
  const body = JSON.parse(String(init.body)) as { params?: { name?: string } };
  return body.params?.name;
}

function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function okRpc(payload: unknown): Response {
  return json({
    result: { content: [{ text: JSON.stringify(payload) }] },
  });
}

function twoProjects() {
  return {
    projects: [
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" },
      { name: "beta", root_path: "/tmp/beta", indexed_at: "2026-08-29T11:30:00Z" },
    ],
  };
}

function mockDashboardFetch(extra?: (url: string, init?: RequestInit) => Response | undefined) {
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    const overridden = extra?.(url, init);
    if (overridden) return overridden;
    if (url === "/rpc") {
      return okRpc({ projects: [] });
    }
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/browse")) {
      return json({
        path: "/home/dev",
        parent: "/home",
        dirs: ["alpha", "beta"],
        roots: ["/", "D:/"],
      });
    }
    if (url === "/api/index") {
      return json({ status: "indexing", slot: 0 }, 202);
    }
    if (url.startsWith("/api/index-status")) {
      return json([{ slot: 0, status: "indexing", path: "/home/dev" }]);
    }
    if (url.startsWith("/api/processes")) {
      return json({ processes: [], self_rss_mb: 0, self_user_cpu_s: 0, self_sys_cpu_s: 0 });
    }
    if (url.startsWith("/api/logs")) {
      return json({ lines: [] });
    }
    if (url.startsWith("/api/project-health")) {
      return json({ status: "healthy" });
    }
    return json({});
  });
  vi.stubGlobal("fetch", fetchMock);
  return fetchMock;
}

describe("Dashboard list + Control", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.useRealTimers();
    vi.restoreAllMocks();
  });

  it("renders two projects with identity and freshness only", async () => {
    const fetchMock = mockDashboardFetch((url, init) => {
      if (url === "/rpc") return okRpc(twoProjects());
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    expect(screen.getByText("/tmp/alpha")).toBeInTheDocument();
    expect(screen.getByText("beta")).toBeInTheDocument();
    expect(screen.getByText("/tmp/beta")).toBeInTheDocument();

    const alphaTime = document.querySelector('time[datetime="2026-08-29T10:00:00Z"]');
    const betaTime = document.querySelector('time[datetime="2026-08-29T11:30:00Z"]');
    expect(alphaTime).toBeTruthy();
    expect(betaTime).toBeTruthy();
    expect(alphaTime?.textContent).toContain("2026");
    expect(alphaTime?.textContent).toContain("29");
    expect(alphaTime?.textContent).not.toBe("2026-08-29T10:00:00Z");
    expect(betaTime?.textContent).toContain("2026");
    expect(betaTime?.textContent).toContain("29");
    expect(betaTime?.textContent).not.toBe("2026-08-29T11:30:00Z");

    expect(screen.getAllByRole("button", { name: messages.en.projects.enter })).toHaveLength(2);
    expect(screen.getByRole("button", { name: `+ ${messages.en.index.newIndex}` })).toBeInTheDocument();
    expect(screen.getAllByRole("button", { name: messages.en.common.refresh }).length).toBeGreaterThanOrEqual(1);

    expect(screen.queryByText(messages.en.projects.nodes, { exact: true })).not.toBeInTheDocument();
    expect(screen.queryByText(messages.en.projects.edges, { exact: true })).not.toBeInTheDocument();
    expect(screen.queryByText("Function")).not.toBeInTheDocument();
    expect(screen.queryByText("Class")).not.toBeInTheDocument();
    expect(screen.queryByRole("button", { name: "ADR" })).not.toBeInTheDocument();
    expect(screen.queryByRole("button", { name: "+ ADR" })).not.toBeInTheDocument();

    const schemaCalls = fetchMock.mock.calls.filter((call) => rpcToolName(call[1] as RequestInit | undefined) === "get_graph_schema");
    expect(schemaCalls).toHaveLength(0);
  });

  it("shows empty CTA and Control when there are zero projects", async () => {
    mockDashboardFetch();
    render(<Dashboard onSelectProject={() => {}} />);

    expect(await screen.findByText(messages.en.projects.noIndexedProjects)).toBeInTheDocument();
    expect(screen.getByRole("button", { name: messages.en.projects.indexFirstRepository })).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.panel)).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.totalCpu)).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.totalRam)).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.processes)).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.selfRam)).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.activeProcesses)).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.processLogs)).toBeInTheDocument();
  });

  it("shows a destructive list error and still shows Control", async () => {
    mockDashboardFetch((url) => {
      if (url === "/rpc") {
        return new Response("fail", { status: 500, statusText: "Internal Server Error" });
      }
      return undefined;
    });
    render(<Dashboard onSelectProject={() => {}} />);

    const alert = await screen.findByRole("alert");
    expect(alert.className).toMatch(/destructive/);
    expect(alert.textContent?.trim().length).toBeGreaterThan(0);
    expect(screen.getByText(messages.en.control.panel)).toBeInTheDocument();
  });

  it("does not DELETE when delete confirm is cancelled", async () => {
    const fetchMock = mockDashboardFetch((url) => {
      if (url === "/rpc") return okRpc({
        projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" }],
      });
      return undefined;
    });
    vi.stubGlobal("confirm", () => false);

    render(<Dashboard onSelectProject={() => {}} />);
    expect(await screen.findByText("alpha")).toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: messages.en.projects.deleteTitle }));

    expect(screen.getByText("alpha")).toBeInTheDocument();
    const deletes = fetchMock.mock.calls.filter((call) => {
      const url = String(call[0]);
      const init = call[1] as RequestInit | undefined;
      return url.includes("/api/project?name=alpha") && init?.method === "DELETE";
    });
    expect(deletes).toHaveLength(0);
  });

  it("keeps Control polls running on the same screen", async () => {
    vi.useFakeTimers();
    const fetchMock = mockDashboardFetch();
    render(<Dashboard onSelectProject={() => {}} />);

    await act(async () => {
      await vi.advanceTimersByTimeAsync(0);
    });
    expect(screen.getByText(messages.en.control.panel)).toBeInTheDocument();

    await act(async () => {
      await vi.advanceTimersByTimeAsync(3500);
    });

    const processCalls = fetchMock.mock.calls.filter((c) => String(c[0]).startsWith("/api/processes"));
    const logCalls = fetchMock.mock.calls.filter((c) => String(c[0]).startsWith("/api/logs"));
    expect(processCalls.length).toBeGreaterThanOrEqual(2);
    expect(logCalls.length).toBeGreaterThanOrEqual(2);
  });
});

describe("Dashboard create-index modal", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.useRealTimers();
  });

  it("posts path only and stays on Dashboard after 202", async () => {
    let submitted: unknown = null;
    mockDashboardFetch((url, init) => {
      if (url === "/api/index") {
        submitted = JSON.parse(String(init?.body));
        return json({ status: "indexing", slot: 0 }, 202);
      }
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));

    expect(await screen.findByDisplayValue("/home/dev")).toBeInTheDocument();
    expect(screen.queryByLabelText(messages.en.index.projectName)).not.toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/home/dev" });
    });
    expect(submitted).not.toHaveProperty("project_name");
    expect(await screen.findByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(onSelectProject).not.toHaveBeenCalled();
  });

  it("keeps the modal open when index POST fails", async () => {
    mockDashboardFetch((url) => {
      if (url === "/api/index") {
        return json({ error: "not a directory" }, 400);
      }
      return undefined;
    });
    render(<Dashboard onSelectProject={() => {}} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));
    expect(await screen.findByDisplayValue("/home/dev")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByText("not a directory")).toBeInTheDocument();
    expect(screen.getByText(messages.en.index.selectRepositoryFolder)).toBeInTheDocument();
  });

  it("filters picker rows and exposes quick row indexing", async () => {
    mockDashboardFetch();
    render(<Dashboard onSelectProject={() => {}} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));

    fireEvent.change(await screen.findByPlaceholderText("Filter folders"), {
      target: { value: "bet" },
    });

    expect(screen.queryByText("alpha")).not.toBeInTheDocument();
    expect(screen.getByText("beta")).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "Index beta" })).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "Browse D:/" })).toBeInTheDocument();
  });

  it("navigates Windows breadcrumb segments to real drive paths", async () => {
    const fetchMock = mockDashboardFetch((url) => {
      if (url.startsWith("/api/browse")) {
        return json({
          path: "C:/Users/rap",
          parent: "C:/Users",
          dirs: ["Documents", "Downloads"],
          roots: ["C:/", "D:/"],
        });
      }
      return undefined;
    });

    render(<Dashboard onSelectProject={() => {}} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));

    await screen.findByRole("button", { name: "C:" });
    expect(screen.queryByRole("button", { name: "/" })).not.toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: "C:" }));
    await waitFor(() => {
      expect(fetchMock).toHaveBeenCalledWith("/api/browse?path=C%3A%2F");
    });

    fireEvent.click(screen.getByRole("button", { name: "Users" }));
    await waitFor(() => {
      expect(fetchMock).toHaveBeenCalledWith("/api/browse?path=C%3A%2FUsers");
    });
  });

  it("refreshes the folder list when a drive is typed into the path field", async () => {
    mockDashboardFetch((url) => {
      if (url.startsWith("/api/browse")) {
        const m = /[?&]path=([^&]*)/.exec(url);
        const path = m ? decodeURIComponent(m[1]) : "C:/Users/rap";
        const onD = path.replace(/\\/g, "/").toUpperCase().startsWith("D:");
        return json({
          path,
          parent: "C:/",
          dirs: onD ? ["projects", "games"] : ["Documents", "Downloads"],
          roots: ["C:/", "D:/"],
        });
      }
      return undefined;
    });

    render(<Dashboard onSelectProject={() => {}} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));

    expect(await screen.findByText("Documents")).toBeInTheDocument();

    fireEvent.change(await screen.findByLabelText("Repository path"), {
      target: { value: "D:/" },
    });

    expect(await screen.findByText("projects")).toBeInTheDocument();
    expect(screen.queryByText("Documents")).not.toBeInTheDocument();
  });

  it("replaces the meaningless '/' root with the drive on Windows", async () => {
    const fetchMock = mockDashboardFetch((url) => {
      if (url.startsWith("/api/browse")) {
        const m = /[?&]path=([^&]*)/.exec(url);
        const path = m ? decodeURIComponent(m[1]) : "C:/Users/rap";
        return json({
          path,
          parent: "C:/",
          dirs: ["Documents"],
          roots: ["/"],
        });
      }
      return undefined;
    });

    render(<Dashboard onSelectProject={() => {}} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));

    expect(await screen.findByRole("button", { name: "Browse C:/" })).toBeInTheDocument();
    expect(screen.queryByRole("button", { name: "Browse /" })).not.toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: "Browse C:/" }));
    await waitFor(() => {
      expect(fetchMock).toHaveBeenCalledWith("/api/browse?path=C%3A%2F");
    });
  });

  it("does not auto-refresh on POSIX when a path is typed", async () => {
    const fetchMock = mockDashboardFetch();

    render(<Dashboard onSelectProject={() => {}} />);
    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));
    await screen.findByText("alpha");

    const browseCalls = () =>
      fetchMock.mock.calls.filter((c) => String(c[0]).startsWith("/api/browse")).length;
    const before = browseCalls();

    fireEvent.change(screen.getByLabelText("Repository path"), {
      target: { value: "/usr/local" },
    });

    await new Promise((r) => setTimeout(r, 400));
    expect(browseCalls()).toBe(before);
  });
});
