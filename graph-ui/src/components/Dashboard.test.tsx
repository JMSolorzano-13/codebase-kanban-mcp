/**
 * @sdd-task: Task #2 - Surface Gherkin: local text, raw ISO dateTime/title
 * @sdd-spec: specs/spec-007-n6p-last-indexed-local/spec.md
 * @sdd-decision: SDD-ADR-034 - indexed_at visible text uses runtime TZ; dateTime/title stay ISO
 * @sdd-why: US-002/005/006 — list + conflict <time> text equals helper; dateTime/title stay raw
 * @human-debug: If text !== formatIndexedAt → surface bypassed helper; if dateTime !== ISO → title/dateTime rewritten
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { act, cleanup, fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { formatIndexedAt } from "../lib/formatIndexedAt";
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
    const fetchMock = mockDashboardFetch((url) => {
      if (url === "/rpc") return okRpc(twoProjects());
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    expect(screen.getByText("/tmp/alpha")).toBeInTheDocument();
    expect(screen.getByText("beta")).toBeInTheDocument();
    expect(screen.getByText("/tmp/beta")).toBeInTheDocument();

    const alphaIso = "2026-08-29T10:00:00Z";
    const betaIso = "2026-08-29T11:30:00Z";
    const alphaTime = document.querySelector(`time[datetime="${alphaIso}"]`);
    const betaTime = document.querySelector(`time[datetime="${betaIso}"]`);
    expect(alphaTime).toBeTruthy();
    expect(betaTime).toBeTruthy();
    expect(alphaTime).toHaveAttribute("dateTime", alphaIso);
    expect(alphaTime).toHaveAttribute("title", alphaIso);
    expect(alphaTime?.textContent).toBe(formatIndexedAt(alphaIso, "en"));
    expect(betaTime).toHaveAttribute("dateTime", betaIso);
    expect(betaTime).toHaveAttribute("title", betaIso);
    expect(betaTime?.textContent).toBe(formatIndexedAt(betaIso, "en"));
    expect(document.body.textContent).toContain("Last indexed");

    expect(screen.getAllByRole("button", { name: messages.en.projects.enter })).toHaveLength(2);
    expect(screen.getAllByRole("button", { name: "Reindex" })).toHaveLength(2);
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

  it("shows invalid indexed_at as raw on the list time", async () => {
    mockDashboardFetch((url) => {
      if (url === "/rpc") {
        return okRpc({
          projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: "not-a-date" }],
        });
      }
      return undefined;
    });
    render(<Dashboard onSelectProject={() => {}} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    const time = document.querySelector('time[datetime="not-a-date"]');
    expect(time).toHaveAttribute("dateTime", "not-a-date");
    expect(time?.textContent).toBe("not-a-date");
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

function twoClones() {
  return {
    projects: [
      { name: "alpha-old", root_path: "/tmp/alpha", indexed_at: "2026-08-28T10:00:00Z", canonical_root: "/tmp/alpha" },
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z", canonical_root: "/tmp/alpha" },
    ],
  };
}

function threeClones() {
  return {
    projects: [
      { name: "a1", root_path: "/tmp/alpha", indexed_at: "2026-08-27T10:00:00Z", canonical_root: "/tmp/alpha" },
      { name: "a2", root_path: "/tmp/alpha", indexed_at: "2026-08-28T10:00:00Z", canonical_root: "/tmp/alpha" },
      { name: "a3", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z", canonical_root: "/tmp/alpha" },
    ],
  };
}

function deleteCalls(fetchMock: ReturnType<typeof vi.fn>, name: string) {
  return fetchMock.mock.calls.filter((call) => {
    const url = String(call[0]);
    const init = call[1] as RequestInit | undefined;
    return url === `/api/project?name=${encodeURIComponent(name)}` && init?.method === "DELETE";
  });
}

describe("Dashboard path conflict groups", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.useRealTimers();
    vi.restoreAllMocks();
  });

  it("shows a two-clone conflict and Enter opens the newest name", async () => {
    mockDashboardFetch((url) => {
      if (url === "/rpc") return okRpc(twoClones());
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);

    const region = await screen.findByRole("region", { name: messages.en.projects.conflict });
    expect(within(region).getByText("alpha-old")).toBeInTheDocument();
    expect(within(region).getByText("alpha")).toBeInTheDocument();
    expect(within(region).getAllByRole("button", { name: "Reindex" })).toHaveLength(2);
    expect(within(region).getAllByText("/tmp/alpha")).toHaveLength(1);

    const oldIso = "2026-08-28T10:00:00Z";
    const newIso = "2026-08-29T10:00:00Z";
    const oldTime = region.querySelector(`time[datetime="${oldIso}"]`);
    const newTime = region.querySelector(`time[datetime="${newIso}"]`);
    expect(oldTime).toHaveAttribute("dateTime", oldIso);
    expect(oldTime).toHaveAttribute("title", oldIso);
    expect(oldTime?.textContent).toBe(formatIndexedAt(oldIso, "en"));
    expect(newTime).toHaveAttribute("dateTime", newIso);
    expect(newTime).toHaveAttribute("title", newIso);
    expect(newTime?.textContent).toBe(formatIndexedAt(newIso, "en"));

    fireEvent.click(within(region).getByRole("button", { name: messages.en.projects.enter }));
    expect(onSelectProject).toHaveBeenCalledTimes(1);
    expect(onSelectProject).toHaveBeenCalledWith("alpha");
  });

  it("confirms delete older and removes only that name after refresh", async () => {
    let listed = twoClones().projects;
    const fetchMock = mockDashboardFetch((url, init) => {
      if (url === "/rpc") return okRpc({ projects: listed });
      if (url.startsWith("/api/project") && init?.method === "DELETE") {
        const q = new URL(url, "http://ui.local");
        listed = listed.filter((p) => p.name !== q.searchParams.get("name"));
        return json({});
      }
      return undefined;
    });
    const onSelectProject = vi.fn();
    vi.stubGlobal("confirm", () => true);

    render(<Dashboard onSelectProject={onSelectProject} />);
    const region = await screen.findByRole("region", { name: messages.en.projects.conflict });
    fireEvent.click(within(region).getByRole("button", { name: messages.en.projects.deleteNamed("alpha-old") }));

    await waitFor(() => {
      expect(deleteCalls(fetchMock, "alpha-old")).toHaveLength(1);
    });
    await waitFor(() => {
      expect(screen.queryByText("alpha-old")).not.toBeInTheDocument();
    });
    expect(screen.getByText("alpha")).toBeInTheDocument();
    expect(deleteCalls(fetchMock, "alpha")).toHaveLength(0);
    expect(onSelectProject).not.toHaveBeenCalled();
  });

  it("three clones: Enter a3 and Delete is per older name", async () => {
    mockDashboardFetch((url) => {
      if (url === "/rpc") return okRpc(threeClones());
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);

    const region = await screen.findByRole("region", { name: messages.en.projects.conflict });
    expect(within(region).getByText("a1")).toBeInTheDocument();
    expect(within(region).getByText("a2")).toBeInTheDocument();
    expect(within(region).getByText("a3")).toBeInTheDocument();
    expect(within(region).getByRole("button", { name: messages.en.projects.deleteNamed("a1") })).toBeInTheDocument();
    expect(within(region).getByRole("button", { name: messages.en.projects.deleteNamed("a2") })).toBeInTheDocument();
    expect(within(region).getAllByRole("button", { name: "Reindex" })).toHaveLength(3);
    expect(screen.queryByRole("button", { name: /delete (both|all|a1 and a2|a2 and a1)/i })).not.toBeInTheDocument();

    fireEvent.click(within(region).getByRole("button", { name: messages.en.projects.enter }));
    expect(onSelectProject).toHaveBeenCalledWith("a3");
  });

  it("cancelled delete older sends no DELETE and keeps the conflict", async () => {
    const fetchMock = mockDashboardFetch((url) => {
      if (url === "/rpc") return okRpc(twoClones());
      return undefined;
    });
    vi.stubGlobal("confirm", () => false);

    render(<Dashboard onSelectProject={() => {}} />);
    const region = await screen.findByRole("region", { name: messages.en.projects.conflict });
    fireEvent.click(within(region).getByRole("button", { name: messages.en.projects.deleteNamed("alpha-old") }));

    expect(deleteCalls(fetchMock, "alpha-old")).toHaveLength(0);
    expect(within(region).getByText("alpha-old")).toBeInTheDocument();
    expect(within(region).getByText("alpha")).toBeInTheDocument();
  });
});

function projectRow(name: string): HTMLElement {
  const label = screen.getByText(name);
  const row = label.closest("li") ?? label.closest(".rounded-xl");
  if (!(row instanceof HTMLElement)) throw new Error(`missing row ${name}`);
  return row;
}

function indexPosts(fetchMock: ReturnType<typeof vi.fn>) {
  return fetchMock.mock.calls.filter((call) => {
    const url = String(call[0]);
    const init = call[1] as RequestInit | undefined;
    return url === "/api/index" && init?.method === "POST";
  });
}

describe("Dashboard Reindex", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.useRealTimers();
    vi.restoreAllMocks();
  });

  it("Reindex POSTs root_path and project and shows IndexProgress", async () => {
    let submitted: unknown = null;
    const fetchMock = mockDashboardFetch((url, init) => {
      if (url === "/rpc") {
        return okRpc({
          projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" }],
        });
      }
      if (url === "/api/index") {
        submitted = JSON.parse(String(init?.body));
        return json({ status: "indexing", slot: 0 }, 202);
      }
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    fireEvent.click(within(projectRow("alpha")).getByRole("button", { name: "Reindex" }));

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/tmp/alpha", project: "alpha" });
    });
    expect(submitted).not.toHaveProperty("project_name");
    expect(indexPosts(fetchMock)).toHaveLength(1);
    expect(await screen.findByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(screen.getByText("alpha")).toBeInTheDocument();
    expect(onSelectProject).not.toHaveBeenCalled();
  });

  it("Reindex of a custom-named project keeps that name", async () => {
    let submitted: unknown = null;
    mockDashboardFetch((url, init) => {
      if (url === "/rpc") {
        return okRpc({
          projects: [{ name: "custom", root_path: "/tmp/newrepo", indexed_at: "2026-08-29T10:00:00Z" }],
        });
      }
      if (url === "/api/index") {
        submitted = JSON.parse(String(init?.body));
        return json({ status: "indexing", slot: 0 }, 202);
      }
      return undefined;
    });
    render(<Dashboard onSelectProject={() => {}} />);

    expect(await screen.findByText("custom")).toBeInTheDocument();
    fireEvent.click(within(projectRow("custom")).getByRole("button", { name: "Reindex" }));

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/tmp/newrepo", project: "custom" });
    });
    expect(submitted).not.toHaveProperty("project_name");
    expect(await screen.findByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(screen.getByText("custom")).toBeInTheDocument();
    expect(screen.queryByRole("heading", { name: "newrepo" })).not.toBeInTheDocument();
  });

  it("Reindex 500 shows a visible error and keeps the row", async () => {
    mockDashboardFetch((url) => {
      if (url === "/rpc") {
        return okRpc({
          projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" }],
        });
      }
      if (url === "/api/index") {
        return json({ error: "index exploded" }, 500);
      }
      return undefined;
    });
    const onSelectProject = vi.fn();
    render(<Dashboard onSelectProject={onSelectProject} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    fireEvent.click(within(projectRow("alpha")).getByRole("button", { name: "Reindex" }));

    const alert = await screen.findByRole("alert");
    expect(alert.textContent?.trim().length).toBeGreaterThan(0);
    expect(screen.getByText("alpha")).toBeInTheDocument();
    expect(onSelectProject).not.toHaveBeenCalled();
    expect(screen.queryByText(messages.en.projects.indexingInProgress)).not.toBeInTheDocument();
  });

  it("conflict member Reindex POSTs that row name", async () => {
    let submitted: unknown = null;
    mockDashboardFetch((url, init) => {
      if (url === "/rpc") return okRpc(twoClones());
      if (url === "/api/index") {
        submitted = JSON.parse(String(init?.body));
        return json({ status: "indexing", slot: 0 }, 202);
      }
      return undefined;
    });
    render(<Dashboard onSelectProject={() => {}} />);

    const region = await screen.findByRole("region", { name: messages.en.projects.conflict });
    expect(within(region).getAllByRole("button", { name: "Reindex" })).toHaveLength(2);
    fireEvent.click(within(projectRow("alpha-old")).getByRole("button", { name: "Reindex" }));

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/tmp/alpha", project: "alpha-old" });
    });
    expect(submitted).not.toHaveProperty("project_name");
  });
});
