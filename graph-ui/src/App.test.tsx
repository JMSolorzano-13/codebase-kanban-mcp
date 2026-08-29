/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-015 - 409 path_exists → Graph + notice; name_exists stays in modal
 * @sdd-why: US-001/006 Gherkin — redirect, status notice, no Project ID, no 202-then-redirect
 * @human-debug: If path_exists stays on Dashboard → onPathExists not wired; if name_exists opens Graph → code treated as path_exists
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { messages } from "./lib/i18n";
import { App } from "./App";

vi.mock("./components/GraphTab", () => ({
  GraphTab: function GraphTabMock({ project }: { project: string | null }) {
    return <div data-testid="graph-tab">{`GraphTab:${project ?? ""}`}</div>;
  },
}));

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

const ALPHA = {
  name: "alpha",
  root_path: "/tmp/alpha",
  indexed_at: "2026-08-29T10:00:00Z",
  canonical_root: "/tmp/alpha",
};

type SpecBoardMode = "false" | "true" | "hang" | "500";

function mockAppFetch(
  projects: { name: string; root_path: string; indexed_at: string; canonical_root?: string }[] = [],
  options: {
    specBoard?: SpecBoardMode;
    adrGet?: unknown;
    onAdrPost?: (body: unknown) => void;
    browse?: { path: string; parent: string; dirs: string[]; roots: string[] };
    onIndex?: (body: unknown) => Response;
  } = {},
) {
  const specBoard = options.specBoard ?? "false";
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    if (url === "/rpc") {
      return okRpc({ projects });
    }
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/spec-board")) {
      if (specBoard === "hang") return new Promise<Response>(() => undefined);
      if (specBoard === "500") return json({ error: "board failed" }, 500);
      return json({
        sdd_skill_present: specBoard === "true",
        specs: [],
      });
    }
    if (url.startsWith("/api/adr")) {
      if (init?.method === "POST") {
        options.onAdrPost?.(JSON.parse(String(init.body)));
        return json({ saved: true });
      }
      return json(options.adrGet ?? { has_adr: true, content: "# Existing ADR\n" });
    }
    if (url.startsWith("/api/browse")) {
      return json(options.browse ?? {
        path: "/home/dev",
        parent: "/home",
        dirs: ["alpha"],
        roots: ["/"],
      });
    }
    if (url === "/api/index") {
      const body = init?.body ? JSON.parse(String(init.body)) : {};
      if (options.onIndex) return options.onIndex(body);
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

function assertNoAccountHeaderTabs() {
  const header = document.querySelector("header");
  expect(header).toBeTruthy();
  const scoped = within(header as HTMLElement);
  for (const name of ["Specs", "Graph", "Projects", "Control"]) {
    expect(scoped.queryByRole("button", { name })).not.toBeInTheDocument();
    expect(scoped.queryByRole("tab", { name })).not.toBeInTheDocument();
  }
}

async function expectDashboard() {
  await waitFor(() => {
    const home =
      screen.queryByText(messages.en.projects.indexedProjects) ||
      screen.queryByText(messages.en.projects.indexFirstRepository);
    expect(home).toBeTruthy();
  });
  expect(screen.getByText(messages.en.control.panel)).toBeInTheDocument();
  expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  expect(screen.queryByText(messages.en.specBoard.selectProject)).not.toBeInTheDocument();
  expect(screen.queryByRole("tab", { name: messages.en.tabs.adr })).not.toBeInTheDocument();
}

async function enterAlpha() {
  expect(await screen.findByText("alpha")).toBeInTheDocument();
  fireEvent.click(screen.getByRole("button", { name: messages.en.projects.enter }));
  expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
}

describe("App routing + workspace", () => {
  afterEach(() => {
    cleanup();
    window.history.replaceState(null, "", "/");
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("opens Dashboard on the default URL and writes ?tab=dashboard without project", async () => {
    mockAppFetch();
    window.history.replaceState(null, "", "/");
    render(<App />);

    await expectDashboard();
    assertNoAccountHeaderTabs();
    await waitFor(() => {
      expect(window.location.search).toBe("?tab=dashboard");
    });
    expect(new URLSearchParams(window.location.search).has("project")).toBe(false);
  });

  it("Enter opens workspace Graph with last-indexed and Graph+ADR tabs", async () => {
    mockAppFetch([ALPHA], { specBoard: "false" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=alpha");
    expect(screen.queryByText(messages.en.control.panel)).not.toBeInTheDocument();

    expect(await screen.findByRole("tab", { name: messages.en.tabs.graph })).toBeInTheDocument();
    expect(screen.getByRole("tab", { name: messages.en.tabs.adr })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    assertNoAccountHeaderTabs();

    const header = document.querySelector("header");
    expect(header?.textContent).toContain("alpha");
    const time = header?.querySelector("time");
    expect(time).toHaveAttribute("dateTime", "2026-08-29T10:00:00Z");
    expect(time?.textContent).toBeTruthy();
    expect(time?.textContent).not.toBe("2026-08-29T10:00:00Z");
  });

  it("shows Specs when sdd-skill is present and opens Kanban without picker copy", async () => {
    mockAppFetch([ALPHA], { specBoard: "true" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    fireEvent.click(await screen.findByRole("tab", { name: messages.en.tabs.specs }));

    await waitFor(() => {
      expect(window.location.search).toContain("tab=specs");
      expect(window.location.search).toContain("project=alpha");
    });
    expect(screen.getByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(screen.queryByText(messages.en.specBoard.selectProject)).not.toBeInTheDocument();
  });

  it("loads and saves ADR from the workspace tab", async () => {
    let posted: unknown = null;
    mockAppFetch([ALPHA], {
      specBoard: "false",
      adrGet: { has_adr: true, content: "# Existing ADR\n" },
      onAdrPost: (body) => { posted = body; },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.adr }));

    await waitFor(() => {
      expect(window.location.search).toContain("tab=adr");
    });
    const textarea = await screen.findByDisplayValue(/# Existing ADR/);
    fireEvent.change(textarea, { target: { value: "# Edited ADR" } });
    fireEvent.click(screen.getByRole("button", { name: messages.en.common.save }));

    await waitFor(() => {
      expect(posted).toEqual({ project: "alpha", content: "# Edited ADR" });
    });
    expect(document.body.textContent).not.toContain("CBM-GENERATED");
  });

  it("leave project returns to Dashboard without project=", async () => {
    mockAppFetch([ALPHA]);
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    fireEvent.click(screen.getByRole("button", { name: messages.en.graph.backToDashboard }));

    await expectDashboard();
    expect(await screen.findByText("alpha")).toBeInTheDocument();
    expect(window.location.search).toBe("?tab=dashboard");
    expect(window.location.search).not.toContain("project=alpha");
  });

  it("omits Specs while spec-board is still loading", async () => {
    mockAppFetch([ALPHA], { specBoard: "hang" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.adr })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
  });

  it("rewrites specs deep link without sdd-skill to Graph", async () => {
    mockAppFetch([ALPHA], { specBoard: "false" });
    window.history.replaceState(null, "", "/?tab=specs&project=alpha");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    await waitFor(() => {
      expect(window.location.search).toContain("tab=graph");
      expect(window.location.search).toContain("project=alpha");
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
  });

  it("omits last-indexed time when the name is not in the list", async () => {
    mockAppFetch([]);
    window.history.replaceState(null, "", "/?tab=graph&project=ghost");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:ghost");
    const header = document.querySelector("header");
    expect(header?.textContent).toContain("ghost");
    expect(header?.querySelector("time")).toBeNull();
  });

  it("aliases ?tab=stats to Dashboard", async () => {
    mockAppFetch();
    window.history.replaceState(null, "", "/?tab=stats");
    render(<App />);

    await expectDashboard();
    await waitFor(() => {
      expect(window.location.search).toBe("?tab=dashboard");
    });
  });

  it("aliases ?tab=control to Dashboard", async () => {
    mockAppFetch();
    window.history.replaceState(null, "", "/?tab=control");
    render(<App />);

    await expectDashboard();
    await waitFor(() => {
      expect(window.location.search).toBe("?tab=dashboard");
    });
  });

  it("aliases specs, unknown, missing, and graph-without-project to Dashboard", async () => {
    mockAppFetch();

    for (const search of ["/?tab=specs", "/?tab=unknown", "/?tab=missing", "/?tab=graph"]) {
      cleanup();
      window.history.replaceState(null, "", search);
      render(<App />);
      await expectDashboard();
      await waitFor(() => {
        expect(window.location.search).toBe("?tab=dashboard");
      });
      expect(new URLSearchParams(window.location.search).has("project")).toBe(false);
    }
  });

  it("opens GraphTab for ?tab=graph&project=alpha", async () => {
    mockAppFetch();
    window.history.replaceState(null, "", "/?tab=graph&project=alpha");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    expect(screen.queryByText(messages.en.control.panel)).not.toBeInTheDocument();
    expect(screen.queryByText(messages.en.specBoard.selectProject)).not.toBeInTheDocument();
    expect(screen.getByRole("button", { name: messages.en.graph.backToDashboard })).toBeInTheDocument();
    await waitFor(() => {
      expect(window.location.search).toBe("?tab=graph&project=alpha");
    });
  });

  it("create-index 202 stays on Dashboard and does not become tab=graph", async () => {
    mockAppFetch();
    window.history.replaceState(null, "", "/");
    render(<App />);

    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));
    expect(await screen.findByDisplayValue("/home/dev")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(window.location.search).not.toContain("tab=graph");
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  });

  it("keeps a dirty ADR draft when leave confirm is dismissed", async () => {
    vi.spyOn(window, "confirm").mockReturnValue(false);
    mockAppFetch([ALPHA], {
      adrGet: { has_adr: true, content: "# Existing ADR\n" },
    });
    window.history.replaceState(null, "", "/?tab=adr&project=alpha");
    render(<App />);

    const textarea = await screen.findByDisplayValue(/# Existing ADR/);
    fireEvent.change(textarea, { target: { value: "# Draft" } });
    fireEvent.click(screen.getByRole("button", { name: messages.en.graph.backToDashboard }));

    expect(window.location.search).toContain("tab=adr");
    expect(window.location.search).toContain("project=alpha");
    expect(screen.getByRole("textbox")).toHaveValue("# Draft");
    expect(screen.queryByText(messages.en.control.panel)).not.toBeInTheDocument();
  });

  it("omits Specs when spec-board returns 500", async () => {
    mockAppFetch([ALPHA], { specBoard: "500" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.adr })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
  });

  it("opens Dashboard for ?tab=adr without project", async () => {
    mockAppFetch();
    window.history.replaceState(null, "", "/?tab=adr");
    render(<App />);

    await expectDashboard();
    await waitFor(() => {
      expect(new URLSearchParams(window.location.search).has("project")).toBe(false);
    });
  });

  it("redirects create 409 path_exists to Graph with a status notice", async () => {
    mockAppFetch([], {
      browse: { path: "/tmp/alpha", parent: "/tmp", dirs: [], roots: ["/"] },
      onIndex: () => json({
        error: "path already indexed",
        code: "path_exists",
        existing_project: "alpha",
        indexed_at: "2026-08-29T10:00:00Z",
      }, 409),
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    fireEvent.click(await screen.findByRole("button", { name: messages.en.projects.indexFirstRepository }));
    expect(await screen.findByDisplayValue("/tmp/alpha")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=alpha");
    expect(screen.getByRole("status")).toHaveTextContent("alpha");
    expect(screen.queryByText(messages.en.projects.indexingInProgress)).not.toBeInTheDocument();
    expect(screen.queryByText(messages.en.index.selectRepositoryFolder)).not.toBeInTheDocument();
  });

  it("keeps the create modal open on 409 name_exists and does not open Graph", async () => {
    mockAppFetch([
      { name: "foo", root_path: "/tmp/a/foo", indexed_at: "2026-08-29T10:00:00Z", canonical_root: "/tmp/a/foo" },
    ], {
      browse: { path: "/tmp/b/foo", parent: "/tmp/b", dirs: [], roots: ["/"] },
      onIndex: () => json({
        error: "name already used by foo",
        code: "name_exists",
        existing_project: "foo",
      }, 409),
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    fireEvent.click(await screen.findByRole("button", { name: `+ ${messages.en.index.newIndex}` }));
    expect(await screen.findByDisplayValue("/tmp/b/foo")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByText("name already used by foo")).toBeInTheDocument();
    expect(screen.getByText(messages.en.index.selectRepositoryFolder)).toBeInTheDocument();
    expect(window.location.search).not.toContain("project=foo");
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
    expect(screen.queryByRole("status")).not.toBeInTheDocument();
  });

  it("skips POST when the list already owns the Path and still redirects", async () => {
    const fetchMock = mockAppFetch([ALPHA], {
      browse: { path: "/tmp/alpha", parent: "/tmp", dirs: [], roots: ["/"] },
      onIndex: () => json({ status: "indexing", slot: 0 }, 202),
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    fireEvent.click(await screen.findByRole("button", { name: `+ ${messages.en.index.newIndex}` }));
    expect(await screen.findByDisplayValue("/tmp/alpha")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=alpha");
    expect(screen.getByRole("status")).toHaveTextContent("alpha");
    expect(screen.queryByText(messages.en.projects.indexingInProgress)).not.toBeInTheDocument();
    const indexPosts = fetchMock.mock.calls.filter(([u]) => String(u) === "/api/index");
    expect(indexPosts).toHaveLength(0);
  });

  it("clears the path_exists notice on the next navigation", async () => {
    mockAppFetch([ALPHA], {
      browse: { path: "/tmp/alpha", parent: "/tmp", dirs: [], roots: ["/"] },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    fireEvent.click(await screen.findByRole("button", { name: `+ ${messages.en.index.newIndex}` }));
    expect(await screen.findByDisplayValue("/tmp/alpha")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByRole("status")).toHaveTextContent("alpha");
    fireEvent.click(screen.getByRole("button", { name: messages.en.graph.backToDashboard }));

    await expectDashboard();
    expect(screen.queryByRole("status")).not.toBeInTheDocument();
    expect(window.location.search).toBe("?tab=dashboard");
  });
});
