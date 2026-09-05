/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-074 Graph ADR omit Game debt; Specs leftover stays spec-board
 * @sdd-why: Graph/ADR unmount Game debt region; Game tab paints it
 * @human-debug: If Graph shows debt:gate → GameBoardTab stayed mounted; if Game tab omits strip → debt not on mock/parse
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { messages } from "./lib/i18n";
import type { GameBoard, GameBoardCard, SpecBoardDebt, SpecBoardEpic } from "./lib/types";
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

const BEVY = {
  name: "bevy",
  root_path: "/tmp/bevy",
  indexed_at: "2026-08-29T10:00:00Z",
  canonical_root: "/tmp/bevy",
};

type GameBoardMode = "hang" | "500" | "true" | "false" | object;

function emptyGameBoard(present: boolean): GameBoard {
  return {
    gamedev_skill_present: present,
    phase: null,
    focus: null,
    continue: present ? "/gamedev-skill continue" : "",
    blocked: [],
    inbox: [],
    preproduction: [],
    production: [],
    postproduction: [],
  };
}

function gameArtifact(partial: Partial<GameBoardCard> & Pick<GameBoardCard, "id" | "title">): GameBoardCard {
  return {
    kind: "artifact",
    track: "B",
    work_state: "pending",
    owner: "",
    continue: "/gamedev-skill continue",
    summary: "",
    plan_title: "",
    blurb: "",
    tasks: [],
    inputs: "",
    last_decision: "",
    open: "",
    recent: "",
    blocked_by: null,
    archived: false,
    ...partial,
  };
}

function gameEpic(partial: Partial<GameBoardCard> & Pick<GameBoardCard, "id" | "title">): GameBoardCard {
  return {
    kind: "epic",
    track: null,
    work_state: null,
    owner: "",
    continue: "/gamedev-skill continue",
    summary: "",
    plan_title: "",
    blurb: "",
    tasks: [],
    inputs: "",
    last_decision: "",
    open: "",
    recent: "",
    blocked_by: null,
    archived: false,
    ...partial,
  };
}

function filledBevyBoard(): GameBoard {
  return {
    gamedev_skill_present: true,
    phase: "02-production",
    focus: "Ship feel",
    continue: "/gamedev-skill continue",
    blocked: [],
    inbox: [
      gameEpic({
        id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md",
        title: "inbox",
        summary: "Filter unread first.",
        plan_title: "Inbox Plan",
        continue: "/gamedev-skill continue",
      }),
    ],
    preproduction: [
      gameArtifact({
        id: ".gamedev/phases/01-preproduction/gdd.md",
        title: "gdd.md",
        track: "B",
        work_state: "pending",
        owner: "@game-designer",
        continue: "/gamedev-skill continue @game-designer",
      }),
    ],
    production: [
      gameArtifact({
        id: ".gamedev/phases/02-production/systems/SYS-001-movement",
        title: "SYS-001-movement",
        track: "A",
        work_state: "pending",
        owner: "@gameplay-engineer",
        continue: "/gamedev-skill continue @gameplay-engineer",
      }),
    ],
    postproduction: [],
  };
}

function postedTo(fetchMock: ReturnType<typeof vi.fn>, path: string): boolean {
  return fetchMock.mock.calls.some((args) => {
    const url = String(args[0]);
    const init = args[1] as RequestInit | undefined;
    const method = (init?.method ?? "GET").toUpperCase();
    return method === "POST" && url.includes(path);
  });
}

function assertGetOnlySkillBoards(fetchMock: ReturnType<typeof vi.fn>) {
  for (const [u, init] of fetchMock.mock.calls) {
    const url = String(u);
    expect(url).not.toContain("/api/skill-presence");
    const method = ((init as RequestInit | undefined)?.method ?? "GET").toUpperCase();
    if (url.includes("/api/game-board") || url.includes("/api/spec-board")) {
      expect(method).toBe("GET");
    }
  }
  expect(postedTo(fetchMock, "/api/game-board")).toBe(false);
  expect(postedTo(fetchMock, "/api/spec-board")).toBe(false);
}

function gameBoardPayload(mode: GameBoardMode): GameBoard {
  if (mode === "true") return emptyGameBoard(true);
  if (mode === "false" || mode === "hang" || mode === "500") return emptyGameBoard(false);
  return { ...emptyGameBoard(false), ...(mode as Partial<GameBoard>) };
}

type AppFetchMock = ReturnType<typeof vi.fn> & {
  resolveGameBoard: (body?: unknown, status?: number) => void;
};

let lastAppFetch: AppFetchMock | undefined;

type SpecBoardMode = "false" | "true" | "hang" | "500";

interface SpecBoardMockFlags {
  sdd?: boolean;
  grill?: boolean;
  epics?: SpecBoardEpic[];
  debt?: SpecBoardDebt[];
}

function isSpecBoardFlags(value: SpecBoardMode | SpecBoardMockFlags): value is SpecBoardMockFlags {
  return typeof value === "object" && value !== null;
}

function specBoardPayload(flags: SpecBoardMockFlags): {
  sdd_skill_present: boolean;
  grill_skill_present: boolean;
  specs: [];
  epics?: SpecBoardEpic[];
  debt?: SpecBoardDebt[];
} {
  const body: {
    sdd_skill_present: boolean;
    grill_skill_present: boolean;
    specs: [];
    epics?: SpecBoardEpic[];
    debt?: SpecBoardDebt[];
  } = {
    sdd_skill_present: flags.sdd === true,
    grill_skill_present: flags.grill === true,
    specs: [],
  };
  if (flags.epics !== undefined) body.epics = flags.epics;
  if (flags.debt !== undefined) body.debt = flags.debt;
  return body;
}

function mockAppFetch(
  projects: { name: string; root_path: string; indexed_at: string; canonical_root?: string }[] = [],
  options: {
    specBoard?: SpecBoardMode | SpecBoardMockFlags;
    gameBoard?: GameBoardMode;
    adrGet?: unknown;
    onAdrPost?: (body: unknown) => void;
    browse?: { path: string; parent: string; dirs: string[]; roots: string[] };
    onIndex?: (body: unknown) => Response;
  } = {},
): AppFetchMock {
  const specBoard = options.specBoard ?? "false";
  const gameBoard = options.gameBoard ?? "false";
  let resolveHang: ((res: Response) => void) | undefined;
  const hangPromise = new Promise<Response>((resolve) => {
    resolveHang = resolve;
  });
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    if (url === "/rpc") {
      return okRpc({ projects });
    }
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/game-board")) {
      if (gameBoard === "hang") return hangPromise;
      if (gameBoard === "500") return json({ error: "board failed" }, 500);
      return json(gameBoardPayload(gameBoard));
    }
    if (url.startsWith("/api/spec-board")) {
      if (specBoard === "hang") return new Promise<Response>(() => undefined);
      if (specBoard === "500") return json({ error: "board failed" }, 500);
      const flags = isSpecBoardFlags(specBoard)
        ? specBoard
        : { sdd: specBoard === "true", grill: false };
      return json(specBoardPayload(flags));
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
  const typed = fetchMock as AppFetchMock;
  typed.resolveGameBoard = (body?: unknown, status = 200) => {
    resolveHang?.(json(body ?? emptyGameBoard(true), status));
  };
  lastAppFetch = typed;
  vi.stubGlobal("fetch", typed);
  return typed;
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

function workspaceTabNames(): string[] {
  return screen.getAllByRole("tab").map((el) => el.textContent ?? "");
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

async function enterBevy() {
  expect(await screen.findByText("bevy")).toBeInTheDocument();
  fireEvent.click(screen.getByRole("button", { name: messages.en.projects.enter }));
  expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:bevy");
}

function assertNoSkillPresence(fetchMock: ReturnType<typeof vi.fn>) {
  for (const [u] of fetchMock.mock.calls) {
    expect(String(u)).not.toContain("/api/skill-presence");
  }
}

function assertLastAppFetchNoSkillPresence() {
  if (!lastAppFetch) return;
  assertNoSkillPresence(lastAppFetch);
}

describe("App routing + workspace", () => {
  afterEach(() => {
    assertLastAppFetchNoSkillPresence();
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
    /* neither-skill: default specBoard "false" is sdd false + grill false */
    mockAppFetch([ALPHA], { specBoard: "false" });
    window.history.replaceState(null, "", "/?tab=specs&project=alpha");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    await waitFor(() => {
      expect(window.location.search).toContain("tab=graph");
      expect(window.location.search).toContain("project=alpha");
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    expect(workspaceTabNames()).toEqual([messages.en.tabs.graph, messages.en.tabs.adr]);
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
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
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

  it("Dashboard Reindex starts a job and stays on Dashboard", async () => {
    let submitted: unknown = null;
    mockAppFetch([ALPHA], {
      onIndex: (body) => {
        submitted = body;
        return json({ status: "indexing", slot: 0 }, 202);
      },
    });
    window.history.replaceState(null, "", "/?tab=dashboard");
    render(<App />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: "Reindex" }));

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/tmp/alpha", project: "alpha" });
    });
    expect(submitted).not.toHaveProperty("project_name");
    expect(await screen.findByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(window.location.search).not.toContain("tab=graph");
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
    expect(screen.getByText("alpha")).toBeInTheDocument();
  });

  it("Reindex of a custom-named project keeps that name", async () => {
    let submitted: unknown = null;
    mockAppFetch([
      { name: "custom", root_path: "/tmp/newrepo", indexed_at: "2026-08-29T10:00:00Z", canonical_root: "/tmp/newrepo" },
    ], {
      onIndex: (body) => {
        submitted = body;
        return json({ status: "indexing", slot: 0 }, 202);
      },
    });
    window.history.replaceState(null, "", "/?tab=dashboard");
    render(<App />);

    expect(await screen.findByText("custom")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: "Reindex" }));

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/tmp/newrepo", project: "custom" });
    });
    expect(submitted).not.toHaveProperty("project_name");
    expect(await screen.findByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(window.location.search).not.toContain("tab=graph");
    expect(screen.getByText("custom")).toBeInTheDocument();
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  });

  it("Dashboard Reindex 500 stays on Dashboard and keeps the row", async () => {
    mockAppFetch([ALPHA], {
      onIndex: () => json({ error: "index exploded" }, 500),
    });
    window.history.replaceState(null, "", "/?tab=dashboard");
    render(<App />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: "Reindex" }));

    const alert = await screen.findByRole("alert");
    expect(alert.textContent?.trim().length).toBeGreaterThan(0);
    expect(window.location.search).not.toContain("tab=graph");
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
    expect(screen.getByText("alpha")).toBeInTheDocument();
    expect(screen.getByText(messages.en.control.panel)).toBeInTheDocument();
  });

  it("workspace header has no Reindex control", async () => {
    mockAppFetch([ALPHA]);
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    const header = document.querySelector("header");
    expect(header).toBeTruthy();
    expect(within(header as HTMLElement).queryByRole("button", { name: "Reindex" })).not.toBeInTheDocument();
    expect(screen.queryByRole("button", { name: "Reindex" })).not.toBeInTheDocument();
  });
});

describe("App strip grill presence (spec-009)", () => {
  afterEach(() => {
    assertLastAppFetchNoSkillPresence();
    cleanup();
    window.history.replaceState(null, "", "/");
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("shows Specs on grill-only with accessible name and Graph then Specs then ADR", async () => {
    mockAppFetch([ALPHA], {
      specBoard: {
        sdd: false,
        grill: true,
        epics: [],
      },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    const specs = await screen.findByRole("tab", { name: "Specs" });
    expect(specs).toHaveAccessibleName("Specs");
    expect(workspaceTabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.specs,
      messages.en.tabs.adr,
    ]);
  });

  it("keeps tab=specs deep-link on grill-only and shows SpecBoardTab", async () => {
    mockAppFetch([ALPHA], { specBoard: { sdd: false, grill: true } });
    window.history.replaceState(null, "", "/?tab=specs&project=alpha");
    render(<App />);

    expect(await screen.findByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    await waitFor(() => {
      expect(window.location.search).toContain("tab=specs");
      expect(window.location.search).toContain("project=alpha");
      expect(window.location.search).not.toContain("tab=graph");
    });
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  });

  it("shows Specs when sdd and grill are both present", async () => {
    mockAppFetch([ALPHA], { specBoard: { sdd: true, grill: true } });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
  });

  it("omits Specs when neither skill and orders Graph then ADR", async () => {
    mockAppFetch([ALPHA], { specBoard: { sdd: false, grill: false } });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    expect(workspaceTabNames()).toEqual([messages.en.tabs.graph, messages.en.tabs.adr]);
  });

  it("omits Specs for gamedev-only when both flags are false and gamedev_skill_present is absent", async () => {
    const body = specBoardPayload({ sdd: false, grill: false });
    expect(body.sdd_skill_present).toBe(false);
    expect(body.grill_skill_present).toBe(false);
    expect(body).not.toHaveProperty("gamedev_skill_present");
    mockAppFetch([ALPHA], { specBoard: { sdd: false, grill: false } });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
  });

  it("Enter still opens Graph on grill-only and keeps Specs in the strip", async () => {
    mockAppFetch([ALPHA], { specBoard: { sdd: false, grill: true } });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=alpha");
    expect(await screen.findByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
  });
});

describe("App silent-win Game tab (spec-010 Task #4)", () => {
  afterEach(() => {
    assertLastAppFetchNoSkillPresence();
    cleanup();
    window.history.replaceState(null, "", "/");
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("gamedev-only project shows the Game tab", async () => {
    const fetchMock = mockAppFetch([BEVY], { gameBoard: "true" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    const game = await screen.findByRole("tab", { name: "Game" });
    expect(game).toHaveAccessibleName("Game");
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    expect(workspaceTabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.game,
      messages.en.tabs.adr,
    ]);
    assertNoSkillPresence(fetchMock);
  });

  it("silent win hides Specs when sdd and grill also exist", async () => {
    mockAppFetch([BEVY], {
      gameBoard: "true",
      specBoard: { sdd: true, grill: true },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.game })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    expect(document.body.textContent).not.toContain(messages.en.projects.conflict);
    expect(screen.queryByRole("status", { name: messages.en.projects.conflict })).not.toBeInTheDocument();
    expect(screen.queryByText(messages.en.projects.conflict)).not.toBeInTheDocument();
  });

  it("Deep-link tab=game stays when gamedev is present", async () => {
    mockAppFetch([BEVY], { gameBoard: "true" });
    window.history.replaceState(null, "", "/?tab=game&project=bevy");
    render(<App />);

    expect(await screen.findByRole("region", { name: messages.en.tabs.game })).toBeInTheDocument();
    await waitFor(() => {
      expect(window.location.search).toContain("tab=game");
      expect(window.location.search).toContain("project=bevy");
      expect(window.location.search).not.toContain("tab=graph");
    });
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  });

  it("Deep-link tab=specs on a gamedev path becomes tab=game", async () => {
    mockAppFetch([BEVY], {
      gameBoard: "true",
      specBoard: { sdd: true, grill: true },
    });
    window.history.replaceState(null, "", "/?tab=specs&project=bevy");
    render(<App />);

    expect(await screen.findByRole("region", { name: messages.en.tabs.game })).toBeInTheDocument();
    await waitFor(() => {
      expect(window.location.search).toContain("tab=game");
      expect(window.location.search).toContain("project=bevy");
      expect(window.location.search).not.toContain("tab=specs");
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
  });

  it("grill-only without gamedev still shows Specs and not Game", async () => {
    mockAppFetch([ALPHA], {
      gameBoard: "false",
      specBoard: { sdd: false, grill: true },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
    expect(workspaceTabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.specs,
      messages.en.tabs.adr,
    ]);
  });

  it("Limit — GET 200 with present false omits Game", async () => {
    mockAppFetch([ALPHA], { gameBoard: "false" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    await waitFor(() => {
      expect(screen.getByRole("tab", { name: messages.en.tabs.adr })).toBeInTheDocument();
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
  });

  it("Limit — Enter still opens Graph on a gamedev project", async () => {
    mockAppFetch([BEVY], { gameBoard: "true" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=bevy");
    expect(await screen.findByRole("tab", { name: messages.en.tabs.game })).toBeInTheDocument();
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:bevy");
  });

  it("Limit — sdd-only without gamedev still shows Specs", async () => {
    mockAppFetch([ALPHA], { gameBoard: "false", specBoard: "true" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
  });

  it("Limit — inbound tab=game restores after omit-until-true", async () => {
    const fetchMock = mockAppFetch([BEVY], { gameBoard: "hang" });
    window.history.replaceState(null, "", "/?tab=game&project=bevy");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:bevy");
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
    await waitFor(() => {
      expect(window.location.search).toContain("tab=graph");
    });

    fetchMock.resolveGameBoard(emptyGameBoard(true));

    expect(await screen.findByRole("region", { name: messages.en.tabs.game })).toBeInTheDocument();
    await waitFor(() => {
      expect(window.location.search).toContain("tab=game");
      expect(window.location.search).toContain("project=bevy");
    });
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  });

  it("Error — game-board still in flight omits Game and Specs", async () => {
    const fetchMock = mockAppFetch([BEVY], {
      gameBoard: "hang",
      specBoard: { sdd: true, grill: true },
    });
    window.history.replaceState(null, "", "/?tab=graph&project=bevy");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:bevy");
    await waitFor(() => {
      expect(fetchMock.mock.calls.some(([u]) => String(u).includes("/api/spec-board"))).toBe(true);
      expect(fetchMock.mock.calls.some(([u]) => String(u).includes("/api/game-board"))).toBe(true);
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    assertNoSkillPresence(fetchMock);
  });

  it("Error — game-board 500 omits Game and does not hide Specs", async () => {
    mockAppFetch([ALPHA], { gameBoard: "500", specBoard: "true" });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
  });

  it("Error — tab=game without gamedev falls back to Graph", async () => {
    mockAppFetch([ALPHA], { gameBoard: "false" });
    window.history.replaceState(null, "", "/?tab=game&project=alpha");
    render(<App />);

    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    await waitFor(() => {
      expect(window.location.search).toContain("tab=graph");
      expect(window.location.search).toContain("project=alpha");
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
  });

  it("Limit — empty gamedev directory still shows Game tab and missing-state chrome", async () => {
    const fetchMock = mockAppFetch([BEVY], { gameBoard: "true" });
    expect(gameBoardPayload("true")).toEqual(emptyGameBoard(true));
    expect(emptyGameBoard(true).phase).toBeNull();
    expect(emptyGameBoard(true).focus).toBeNull();
    expect(emptyGameBoard(true).continue).toBe("/gamedev-skill continue");
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    const game = await screen.findByRole("tab", { name: "Game" });
    expect(game).toHaveAccessibleName("Game");
    fireEvent.click(game);

    const pane = await screen.findByRole("region", { name: messages.en.tabs.game });
    expect(pane).toHaveTextContent(messages.en.gameBoard.stateMdMissing);
    expect(pane).toHaveTextContent("/gamedev-skill continue");
    expect(pane).toHaveTextContent(messages.en.gameBoard.columnInbox);
    expect(pane).toHaveTextContent(messages.en.gameBoard.phasePreproduction);
    expect(pane).toHaveTextContent(messages.en.gameBoard.phaseProduction);
    expect(pane).toHaveTextContent(messages.en.gameBoard.phasePostproduction);
    expect(screen.queryByRole("button", { name: /gamedev-skill/i })).not.toBeInTheDocument();
    expect(workspaceTabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.game,
      messages.en.tabs.adr,
    ]);
    assertNoSkillPresence(fetchMock);
  });

  it("Error — graph-ui issues no request whose path contains /api/skill-presence", async () => {
    const silentWin = mockAppFetch([BEVY], {
      gameBoard: "true",
      specBoard: { sdd: true, grill: true },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    expect(await screen.findByRole("tab", { name: "Game" })).toBeInTheDocument();
    await waitFor(() => {
      expect(silentWin.mock.calls.some(([u]) => String(u).includes("/api/game-board"))).toBe(true);
    });
    assertNoSkillPresence(silentWin);

    cleanup();
    vi.unstubAllGlobals();

    const grillOnly = mockAppFetch([ALPHA], {
      gameBoard: "false",
      specBoard: { sdd: false, grill: true },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(await screen.findByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).not.toBeInTheDocument();
    await waitFor(() => {
      expect(grillOnly.mock.calls.some(([u]) => String(u).includes("/api/spec-board"))).toBe(true);
    });
    assertNoSkillPresence(grillOnly);
  });
});

describe("App Game phase board leftover Gherkin (spec-011 Task #5)", () => {
  afterEach(() => {
    assertLastAppFetchNoSkillPresence();
    cleanup();
    window.history.replaceState(null, "", "/");
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("activates Game and paints GET /api/game-board cards", async () => {
    const fetchMock = mockAppFetch([BEVY], { gameBoard: filledBevyBoard() });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    fireEvent.click(await screen.findByRole("tab", { name: messages.en.tabs.game }));

    const pane = await screen.findByRole("region", { name: messages.en.tabs.game });
    expect(pane).toHaveTextContent("Inbox");
    expect(pane).toHaveTextContent("Pre-production");
    expect(pane).toHaveTextContent("Production");
    expect(pane).toHaveTextContent("Post-production & Launch");
    expect(pane).toHaveTextContent(".gamedev/phases/01-preproduction/gdd.md");
    expect(pane).toHaveTextContent("gdd.md");
    expect(pane).toHaveTextContent("SYS-001-movement");
    expect(pane).toHaveTextContent(".grill/plans/inbox-plan/epics/epic-001-inbox.md");
    expect(pane).toHaveTextContent("Filter unread first.");
    expect(pane).toHaveTextContent("Inbox Plan");
    expect(within(pane).getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(window.location.search).toContain("tab=game");
    expect(window.location.search).toContain("project=bevy");
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    assertGetOnlySkillBoards(fetchMock);
  });

  it("Inbox cards come from game-board inbox, not spec-board epics", async () => {
    const specOnlyEpic: SpecBoardEpic = {
      kind: "epic",
      id: ".grill/plans/other/epics/epic-099-spec.md",
      title: "should-not-appear-on-game",
      summary: "Spec-board leftover.",
      plan_title: "Other Plan",
      column: "todo",
    };
    const fetchMock = mockAppFetch([BEVY], {
      gameBoard: filledBevyBoard(),
      specBoard: { sdd: true, grill: true, epics: [specOnlyEpic] },
    });
    window.history.replaceState(null, "", "/?tab=game&project=bevy");
    render(<App />);

    const pane = await screen.findByRole("region", { name: messages.en.tabs.game });
    expect(pane).toHaveTextContent(".grill/plans/inbox-plan/epics/epic-001-inbox.md");
    expect(pane).toHaveTextContent("inbox");
    expect(pane).not.toHaveTextContent("should-not-appear-on-game");
    expect(pane).not.toHaveTextContent("epic-099-spec");
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    await waitFor(() => {
      expect(fetchMock.mock.calls.some(([u]) => String(u).includes("/api/game-board"))).toBe(true);
    });
    expect(fetchMock.mock.calls.some(([u]) => String(u).includes("/api/inbox"))).toBe(false);
    expect(fetchMock.mock.calls.filter(([u]) => String(u).includes("/api/game-board"))).toHaveLength(1);
    assertGetOnlySkillBoards(fetchMock);
  });

  it("Error — GET does not write skill trees (graph-ui no skill-presence half)", async () => {
    const fetchMock = mockAppFetch([BEVY], {
      gameBoard: filledBevyBoard(),
      specBoard: { sdd: true, grill: true },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    fireEvent.click(await screen.findByRole("tab", { name: "Game" }));
    expect(await screen.findByRole("region", { name: messages.en.tabs.game })).toBeInTheDocument();
    await waitFor(() => {
      expect(fetchMock.mock.calls.some(([u]) => String(u).includes("/api/game-board"))).toBe(true);
    });
    for (const [u] of fetchMock.mock.calls) {
      expect(String(u)).not.toContain("/api/skill-presence");
    }
    assertGetOnlySkillBoards(fetchMock);
    expect(fetchMock.mock.calls.some(([u]) => String(u).includes("/api/game-board"))).toBe(true);
  });

  it("Limit — Enter still opens Graph on a filled gamedev board", async () => {
    const fetchMock = mockAppFetch([BEVY], { gameBoard: filledBevyBoard() });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    expect(window.location.search).toContain("tab=graph");
    expect(window.location.search).toContain("project=bevy");
    expect(await screen.findByRole("tab", { name: messages.en.tabs.game })).toBeInTheDocument();
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:bevy");
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
    assertNoSkillPresence(fetchMock);
  });

  it("Deep-link leftover tab=specs on a filled gamedev board becomes tab=game", async () => {
    mockAppFetch([BEVY], {
      gameBoard: filledBevyBoard(),
      specBoard: { sdd: true, grill: true },
    });
    window.history.replaceState(null, "", "/?tab=specs&project=bevy");
    render(<App />);

    const pane = await screen.findByRole("region", { name: messages.en.tabs.game });
    expect(pane).toHaveTextContent("SYS-001-movement");
    await waitFor(() => {
      expect(window.location.search).toContain("tab=game");
      expect(window.location.search).toContain("project=bevy");
      expect(window.location.search).not.toContain("tab=specs");
    });
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).not.toBeInTheDocument();
  });
});

describe("App spec-015 leftover debt strip locks", () => {
  afterEach(() => {
    assertLastAppFetchNoSkillPresence();
    cleanup();
    window.history.replaceState(null, "", "/");
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  // Gherkin: Limit — Graph ADR and Game do not show the strip (Graph/ADR half)
  it("unmounts Open tech debt when Graph or ADR is activated", async () => {
    mockAppFetch([ALPHA], {
      specBoard: {
        sdd: true,
        debt: [{ id: "TD-005", title: "leftover cache" }],
      },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();

    fireEvent.click(await screen.findByRole("tab", { name: messages.en.tabs.specs }));
    expect(await screen.findByRole("region", { name: messages.en.specBoard.openTechDebt })).toHaveTextContent(
      "TD-005",
    );
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();

    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.graph }));
    expect(await screen.findByTestId("graph-tab")).toHaveTextContent("GraphTab:alpha");
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();

    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.adr }));
    await waitFor(() => {
      expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
    });
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
  });

  // Gherkin: Limit — grill-only without TECH_DEBT.md still shows Specs
  it("shows Specs on grill-only with empty debt and omits Open tech debt and notSddSkill", async () => {
    mockAppFetch([ALPHA], {
      specBoard: { sdd: false, grill: true, debt: [] },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterAlpha();
    fireEvent.click(await screen.findByRole("tab", { name: messages.en.tabs.specs }));

    expect(await screen.findByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(screen.getByRole("tab", { name: messages.en.tabs.specs })).toBeInTheDocument();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
    expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
  });
});

describe("App spec-017 Game debt leftover locks", () => {
  afterEach(() => {
    assertLastAppFetchNoSkillPresence();
    cleanup();
    window.history.replaceState(null, "", "/");
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("omits Open tech debt on Graph and ADR when Game debt is present", async () => {
    mockAppFetch([BEVY], {
      gameBoard: {
        ...emptyGameBoard(true),
        debt: [{ id: "debt:gate-preproduction", title: "missing GDD lock" }],
      },
    });
    window.history.replaceState(null, "", "/");
    render(<App />);

    await enterBevy();
    expect(screen.getByTestId("graph-tab")).toHaveTextContent("GraphTab:bevy");
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(screen.queryByText("debt:gate-preproduction")).toBeNull();

    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.adr }));
    await waitFor(() => {
      expect(screen.queryByTestId("graph-tab")).not.toBeInTheDocument();
    });
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(screen.queryByText("debt:gate-preproduction")).toBeNull();

    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.game }));
    const region = await screen.findByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(region).toHaveTextContent("debt:gate-preproduction");
    expect(region).toHaveTextContent("missing GDD lock");
  });
});
