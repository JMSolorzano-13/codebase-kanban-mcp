/**
 * @sdd-task: Task #5 - App routing + TabBar delete
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-008 - TabId dashboard|graph; stats/control/specs alias home
 * @sdd-why: US-001 Gherkin — default URL, aliases, Enter/back, graph deep link
 * @human-debug: If GraphTab boots Three → vi.mock missing; if Specs shows → App still routes SpecBoardTab
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

function mockAppFetch(projects: { name: string; root_path: string; indexed_at: string }[] = []) {
  const fetchMock = vi.fn(async (input: RequestInfo | URL) => {
    const url = String(input);
    if (url === "/rpc") {
      return okRpc({ projects });
    }
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/browse")) {
      return json({
        path: "/home/dev",
        parent: "/home",
        dirs: ["alpha"],
        roots: ["/"],
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
}

describe("App routing + header IA", () => {
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

  it("Enter opens existing Graph and back returns to Dashboard", async () => {
    mockAppFetch([
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" },
    ]);
    window.history.replaceState(null, "", "/");
    render(<App />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.projects.enter }));

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=alpha");
    expect(screen.queryByText(messages.en.control.panel)).not.toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: messages.en.graph.backToDashboard }));

    await expectDashboard();
    expect(await screen.findByText("alpha")).toBeInTheDocument();
    expect(window.location.search).toBe("?tab=dashboard");
    expect(window.location.search).not.toContain("project=alpha");
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
});
