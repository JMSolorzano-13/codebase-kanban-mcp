/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-074 Game-only strip after BlockedStrip; reuse specBoard.openTechDebt
 * @sdd-why: strip paint/omit, placement vs Blocked, wrap, dead row, Show Dones lock
 * @human-debug: If strip missing with mocked debt → ?? [] or region not after Blocked; if Graph shows it → App leftover failed
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { useCallback, useState } from "react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { colorForLabel } from "../lib/colors";
import { messages } from "../lib/i18n";
import type { GameBoard, GameBoardCard } from "../lib/types";
import { GameBoardTab } from "./GameBoardTab";

function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function mockUiFetch() {
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    void init;
    if (String(input).startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    return json({});
  });
  vi.stubGlobal("fetch", fetchMock);
  return fetchMock;
}

function markArchived(cards: GameBoardCard[], cardId: string, archived: boolean): GameBoardCard[] {
  return cards.map((card) => (card.id === cardId ? { ...card, archived } : card));
}

function applyArchiveFlag(current: GameBoard, cardId: string, archived: boolean): GameBoard {
  return {
    ...current,
    inbox: markArchived(current.inbox, cardId, archived),
    preproduction: markArchived(current.preproduction, cardId, archived),
    production: markArchived(current.production, cardId, archived),
    postproduction: markArchived(current.postproduction, cardId, archived),
  };
}

function mockArchiveFetch(store: { board: GameBoard }) {
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    const method = ((init?.method ?? "GET") as string).toUpperCase();
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (method === "POST" && url.includes("/api/game-board")) {
      const body = JSON.parse(String(init?.body ?? "{}")) as {
        project?: string;
        card_id?: string;
        archived?: boolean;
      };
      if (typeof body.card_id === "string" && typeof body.archived === "boolean") {
        store.board = applyArchiveFlag(store.board, body.card_id, body.archived);
      }
      return json({
        project: body.project,
        card_id: body.card_id,
        archived: body.archived,
      });
    }
    return json({});
  });
  vi.stubGlobal("fetch", fetchMock);
  return fetchMock;
}

function LiveGameBoardTab({
  store,
  project = "bevy",
}: {
  store: { board: GameBoard };
  project?: string;
}) {
  const [board, setBoard] = useState(() => store.board);
  const refresh = useCallback(async () => {
    setBoard({
      ...store.board,
      blocked: [...store.board.blocked],
      inbox: [...store.board.inbox],
      preproduction: [...store.board.preproduction],
      production: [...store.board.production],
      postproduction: [...store.board.postproduction],
    });
  }, [store]);
  return <GameBoardTab board={board} project={project} refresh={refresh} />;
}

function lastGameBoardPost(fetchMock: ReturnType<typeof vi.fn>): {
  project: string;
  card_id: string;
  archived: boolean;
} {
  const call = [...fetchMock.mock.calls].reverse().find((args) => {
    const url = String(args[0]);
    const init = args[1] as RequestInit | undefined;
    return (init?.method ?? "GET").toUpperCase() === "POST" && url.includes("/api/game-board");
  });
  if (!call) throw new Error("no POST /api/game-board");
  return JSON.parse(String((call[1] as RequestInit).body)) as {
    project: string;
    card_id: string;
    archived: boolean;
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

function gameBoardRequests(fetchMock: ReturnType<typeof vi.fn>): { method: string; url: string }[] {
  return fetchMock.mock.calls
    .filter((args) => String(args[0]).includes("/api/game-board"))
    .map((args) => {
      const url = String(args[0]);
      const init = args[1] as RequestInit | undefined;
      return {
        method: ((init?.method ?? "GET") as string).toUpperCase(),
        url,
      };
    });
}

function gameBoardUrlHasQuery(fetchMock: ReturnType<typeof vi.fn>, name: string): boolean {
  return gameBoardRequests(fetchMock).some((req) => {
    const qIndex = req.url.indexOf("?");
    if (qIndex < 0) return false;
    return new URLSearchParams(req.url.slice(qIndex + 1)).has(name);
  });
}

function localStorageHasFilterKey(): boolean {
  const storage = window.localStorage;
  if (!storage) return false;
  const keys: string[] = [];
  for (let i = 0; i < storage.length; i++) {
    const key = storage.key(i);
    if (key !== null) keys.push(key);
  }
  return keys.some((key) => key.includes("showDones") || key.includes("gameTrack"));
}

function installMemoryLocalStorage(): void {
  const store = new Map<string, string>();
  const storage: Storage = {
    get length() {
      return store.size;
    },
    clear() {
      store.clear();
    },
    getItem(key: string) {
      return store.get(key) ?? null;
    },
    key(index: number) {
      return [...store.keys()][index] ?? null;
    },
    removeItem(key: string) {
      store.delete(key);
    },
    setItem(key: string, value: string) {
      store.set(key, value);
    },
  };
  Object.defineProperty(window, "localStorage", {
    configurable: true,
    value: storage,
  });
}

function stubWriteText(impl: () => Promise<void>) {
  const writeText = vi.fn(impl);
  Object.defineProperty(navigator, "clipboard", {
    configurable: true,
    value: { writeText },
  });
  return writeText;
}

function assertNoToast(root: HTMLElement) {
  expect(screen.queryByRole("status")).toBeNull();
  expect(screen.queryByRole("alert")).toBeNull();
  expect(root).not.toHaveTextContent("Copied");
  expect(root).not.toHaveTextContent("toast");
}

function artifact(partial: Partial<GameBoardCard> & Pick<GameBoardCard, "id" | "title">): GameBoardCard {
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

function epic(partial: Partial<GameBoardCard> & Pick<GameBoardCard, "id" | "title">): GameBoardCard {
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

function board(partial: Partial<GameBoard>): GameBoard {
  return {
    gamedev_skill_present: true,
    phase: null,
    focus: null,
    continue: "/gamedev-skill continue",
    blocked: [],
    inbox: [],
    preproduction: [],
    production: [],
    postproduction: [],
    ...partial,
  };
}

function pane(): HTMLElement {
  return screen.getByRole("region", { name: messages.en.tabs.game });
}

function columnRoot(title: string): HTMLElement {
  const heading = screen.getByRole("heading", { name: new RegExp(`^${title}$`) });
  const root = heading.parentElement;
  if (!root) throw new Error(`no column ${title}`);
  return root;
}

function cardsIn(title: string): HTMLElement[] {
  return within(columnRoot(title)).queryAllByRole("article");
}

function cardByText(column: string, text: string): HTMLElement {
  const card = cardsIn(column).find((el) => el.textContent?.includes(text));
  if (!card) throw new Error(`no card with ${text} in ${column}`);
  return card;
}

function titleControl(card: HTMLElement): HTMLElement {
  const btn = within(card)
    .getAllByRole("button")
    .find((el) => el.hasAttribute("aria-expanded"));
  if (!btn) throw new Error("no title control");
  return btn;
}

const originalClipboard = navigator.clipboard;
const originalLocalStorage = Object.getOwnPropertyDescriptor(window, "localStorage");

describe("GameBoardTab", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
    Object.defineProperty(navigator, "clipboard", {
      configurable: true,
      value: originalClipboard,
    });
    if (originalLocalStorage) {
      Object.defineProperty(window, "localStorage", originalLocalStorage);
    }
  });

  it("shows Production chrome, four columns, and continue without a launcher", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          focus: "Triaging playtest round 3 feedback on movement feel",
          continue: "/gamedev-skill continue",
        })}
      />,
    );

    const root = pane();
    expect(root).toHaveTextContent(messages.en.gameBoard.phaseProduction);
    expect(root).toHaveTextContent("Triaging playtest round 3 feedback on movement feel");
    expect(root).toHaveTextContent("/gamedev-skill continue");
    expect(root).toHaveTextContent(messages.en.gameBoard.columnInbox);
    expect(root).toHaveTextContent(messages.en.gameBoard.phasePreproduction);
    expect(root).toHaveTextContent(messages.en.gameBoard.phasePostproduction);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Show archived" })).toBeNull();
    expect(screen.queryByRole("button", { name: /^\/gamedev-skill continue$/ })).toBeNull();
    expect(root).not.toHaveTextContent("Todo");
    expect(root).not.toHaveTextContent("In Progress");
    expect(root).not.toHaveTextContent("gate-review");
  });

  it("shows state.md missing chrome and four headers when phase and focus are null", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          gamedev_skill_present: true,
          phase: null,
          focus: null,
          continue: "/gamedev-skill continue",
        })}
      />,
    );

    const root = pane();
    expect(root).toHaveTextContent(messages.en.gameBoard.stateMdMissing);
    expect(root).toHaveTextContent("/gamedev-skill continue");
    expect(root).toHaveTextContent(messages.en.gameBoard.columnInbox);
    expect(root).toHaveTextContent(messages.en.gameBoard.phasePreproduction);
    expect(root).toHaveTextContent(messages.en.gameBoard.phaseProduction);
    expect(root).toHaveTextContent(messages.en.gameBoard.phasePostproduction);
    expect(root).not.toHaveTextContent("Todo");
    expect(root).not.toHaveTextContent("In Progress");
    expect(root).not.toHaveTextContent("gate-review");
    expect(root).not.toHaveTextContent("launcher");
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.queryByRole("button", { name: /gamedev-skill/i })).not.toBeInTheDocument();
  });

  it("paints column arrays when GET keys carry cards", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          focus: "Ship feel",
          inbox: [epic({ id: "in-1", title: "Hidden inbox card" })],
          production: [artifact({ id: "pr-1", title: "Hidden production card", track: "A" })],
        })}
      />,
    );

    const root = pane();
    expect(root).toHaveTextContent("Hidden inbox card");
    expect(root).toHaveTextContent("Hidden production card");
    expect(root).toHaveTextContent("Inbox");
    expect(root).not.toHaveTextContent("Todo");
    expect(root).not.toHaveTextContent("In Progress");
    expect(root).not.toHaveTextContent("gate-review");
    expect(root).not.toHaveTextContent("launcher");
    expect(root).toHaveTextContent(messages.en.gameBoard.phaseProduction);
    expect(root).toHaveTextContent("Ship feel");
    expect(screen.queryByRole("button", { name: /launcher/i })).not.toBeInTheDocument();
  });

  it("shows four column headers and current-phase highlight", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          focus: "Ship feel",
          continue: "/gamedev-skill continue",
        })}
      />,
    );

    const headings = screen.getAllByRole("heading", { level: 3 }).map((el) => el.textContent);
    expect(headings).toEqual([
      messages.en.gameBoard.columnInbox,
      messages.en.gameBoard.phasePreproduction,
      messages.en.gameBoard.phaseProduction,
      messages.en.gameBoard.phasePostproduction,
    ]);
    expect(columnRoot("Production")).toHaveAttribute("aria-current", "true");
    expect(columnRoot("Inbox")).not.toHaveAttribute("aria-current", "true");
    expect(columnRoot("Pre-production")).not.toHaveAttribute("aria-current", "true");
    expect(columnRoot("Post-production & Launch")).not.toHaveAttribute("aria-current", "true");
    expect(pane()).toHaveTextContent(messages.en.gameBoard.phaseProduction);
    expect(pane()).toHaveTextContent("/gamedev-skill continue");
  });

  it("paints an existing gdd.md Pre-production artifact card", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              owner: "@game-designer",
              continue: "/gamedev-skill continue @game-designer",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    expect(card).toHaveTextContent("gdd.md");
    expect(card).toHaveTextContent("B");
    expect(card).toHaveTextContent("Pending");
    expect(card).toHaveTextContent("@game-designer");
    expect(card).toHaveTextContent("/gamedev-skill continue @game-designer");
    expect(within(card).queryByText("E", { exact: true })).toBeNull();
  });

  it("paints a SYS directory without spec.md as a Production card", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "pending",
              owner: "@gameplay-engineer",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Production", "SYS-001-movement");
    expect(card).toHaveTextContent("A");
    expect(card).toHaveTextContent("Pending");
  });

  it("keeps two in_progress cards in Production", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "in_progress",
              owner: "@gameplay-engineer",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-002-score",
              title: "SYS-002-score",
              track: "A",
              work_state: "in_progress",
              owner: "@gameplay-engineer",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
          ],
        })}
      />,
    );

    expect(cardByText("Production", "SYS-001-movement")).toHaveTextContent("In progress");
    expect(cardByText("Production", "SYS-002-score")).toHaveTextContent("In progress");
  });

  it("paints an unconverted grill epic in Inbox", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md",
              title: "inbox",
              summary: "Filter unread first.",
              plan_title: "Inbox Plan",
              continue: "/gamedev-skill continue",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Inbox", ".grill/plans/inbox-plan/epics/epic-001-inbox.md");
    expect(within(card).getByText("E", { exact: true })).toBeInTheDocument();
    expect(card).toHaveTextContent("inbox");
    expect(card).toHaveTextContent("Filter unread first.");
    expect(card).toHaveTextContent("Inbox Plan");
    expect(card).toHaveTextContent("/gamedev-skill continue");
    expect(card).not.toHaveTextContent("@director");
    expect(card).not.toHaveTextContent("Pending");
    expect(within(card).queryByText("A", { exact: true })).toBeNull();
  });

  it("Limit — empty column has header and no placeholder", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          postproduction: [],
        })}
      />,
    );

    const col = columnRoot("Post-production & Launch");
    expect(col).toBeInTheDocument();
    expect(cardsIn("Post-production & Launch")).toHaveLength(0);
    expect(col).not.toHaveTextContent("no artifacts in this phase");
    expect(col.className).not.toMatch(/opacity|hidden|invisible/);
  });

  it("Limit — phase null highlights no phase column", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          gamedev_skill_present: true,
          phase: null,
          focus: null,
          continue: "/gamedev-skill continue",
        })}
      />,
    );

    expect(columnRoot("Inbox")).toBeInTheDocument();
    expect(columnRoot("Pre-production")).toBeInTheDocument();
    expect(columnRoot("Production")).toBeInTheDocument();
    expect(columnRoot("Post-production & Launch")).toBeInTheDocument();
    expect(pane().querySelector('[aria-current="true"]')).toBeNull();
    expect(pane()).toHaveTextContent(messages.en.gameBoard.stateMdMissing);
  });

  it("Limit — 65th production: no control named Has more", () => {
    mockUiFetch();
    const production = Array.from({ length: 64 }, (_, i) =>
      artifact({
        id: `.gamedev/phases/02-production/systems/SYS-${String(i + 1).padStart(3, "0")}-item`,
        title: `SYS-${String(i + 1).padStart(3, "0")}-item`,
        track: "A",
      }),
    );
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production,
        })}
      />,
    );

    expect(cardsIn("Production")).toHaveLength(64);
    expect(screen.queryByRole("button", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByRole("link", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByText("Has more")).toBeNull();
  });

  it("Limit — status ready maps to In progress on the card", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/qa/playtest-log.md",
              title: "playtest-log.md",
              track: "H",
              work_state: "in_progress",
              owner: "@qa-lead",
              continue: "/gamedev-skill continue @qa-lead",
            }),
          ],
        })}
      />,
    );

    expect(cardByText("Production", ".gamedev/phases/02-production/qa/playtest-log.md")).toHaveTextContent(
      "In progress",
    );
  });

  it("Limit — no grill directory: Inbox shows 0 cards", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [],
        })}
      />,
    );

    expect(columnRoot("Inbox")).toBeInTheDocument();
    expect(cardsIn("Inbox")).toHaveLength(0);
  });

  it("click artifact copies continue at-role and does not toast or POST", async () => {
    const fetchMock = mockUiFetch();
    const writeText = stubWriteText(async () => undefined);
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              owner: "@game-designer",
              continue: "/gamedev-skill continue @game-designer",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    const control = within(card).getByRole("button", { name: "/gamedev-skill continue @game-designer" });
    fireEvent.click(control);
    await waitFor(() => {
      expect(writeText).toHaveBeenCalledWith("/gamedev-skill continue @game-designer");
    });
    assertNoToast(pane());
    expect(postedTo(fetchMock, "/api/game-board")).toBe(false);
    expect(screen.queryByRole("button", { name: /^\/gamedev-skill continue$/ })).toBeNull();
  });

  it("click inbox copies continue without at-role and does not toast", async () => {
    mockUiFetch();
    const writeText = stubWriteText(async () => undefined);
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md",
              title: "inbox",
              summary: "Filter unread first.",
              plan_title: "Inbox Plan",
              continue: "/gamedev-skill continue",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Inbox", ".grill/plans/inbox-plan/epics/epic-001-inbox.md");
    const control = within(card).getByRole("button", { name: "/gamedev-skill continue" });
    fireEvent.click(control);
    await waitFor(() => {
      expect(writeText).toHaveBeenCalledWith("/gamedev-skill continue");
    });
    assertNoToast(pane());
    expect(control).not.toHaveTextContent("@");
  });

  it("clipboard denied or missing selects continue text and shows no toast", async () => {
    mockUiFetch();
    const writeText = stubWriteText(async () => {
      throw new DOMException("denied", "NotAllowedError");
    });
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              owner: "@game-designer",
              continue: "/gamedev-skill continue @game-designer",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    const control = within(card).getByRole("button", { name: "/gamedev-skill continue @game-designer" });
    fireEvent.click(control);
    await waitFor(() => {
      expect(writeText).toHaveBeenCalled();
    });
    expect(window.getSelection()?.toString()).toBe("/gamedev-skill continue @game-designer");
    assertNoToast(pane());

    cleanup();
    Object.defineProperty(navigator, "clipboard", {
      configurable: true,
      value: undefined,
    });
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              owner: "@game-designer",
              continue: "/gamedev-skill continue @game-designer",
            }),
          ],
        })}
      />,
    );
    const again = within(cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md")).getByRole(
      "button",
      { name: "/gamedev-skill continue @game-designer" },
    );
    fireEvent.click(again);
    await waitFor(() => {
      expect(window.getSelection()?.toString()).toBe("/gamedev-skill continue @game-designer");
    });
    assertNoToast(pane());
  });

  it("Error — card body click does not expand or archive", () => {
    const fetchMock = mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              owner: "@game-designer",
              continue: "/gamedev-skill continue @game-designer",
              last_decision: "Lock the loop as rotating tetrominoes.",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    fireEvent.click(card);
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "false");
    expect(card).not.toHaveTextContent("Lock the loop as rotating tetrominoes.");
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(card).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
    expect(card).not.toHaveTextContent("No tasks planned yet");
    expect(postedTo(fetchMock, "/api/game-board")).toBe(false);
    expect(postedTo(fetchMock, "/api/spec-board")).toBe(false);
  });

  it("Error — cards are not draggable and drop does not move them", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "pending",
              owner: "@gameplay-engineer",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Production", "SYS-001-movement");
    expect(card).toHaveAttribute("draggable", "false");
    const preCount = cardsIn("Pre-production").length;
    fireEvent.dragStart(card);
    fireEvent.dragOver(columnRoot("Pre-production"));
    fireEvent.drop(columnRoot("Pre-production"));
    expect(cardsIn("Pre-production")).toHaveLength(preCount);
    expect(cardByText("Production", "SYS-001-movement")).toBeInTheDocument();
  });

  it("maps 01-preproduction and 03-postproduction to aria-current", () => {
    mockUiFetch();
    render(<GameBoardTab board={board({ phase: "01-preproduction" })} />);
    expect(columnRoot("Pre-production")).toHaveAttribute("aria-current", "true");
    expect(columnRoot("Inbox")).not.toHaveAttribute("aria-current", "true");
    expect(columnRoot("Production")).not.toHaveAttribute("aria-current", "true");
    expect(columnRoot("Post-production & Launch")).not.toHaveAttribute("aria-current", "true");

    cleanup();
    mockUiFetch();
    render(<GameBoardTab board={board({ phase: "03-postproduction" })} />);
    expect(columnRoot("Post-production & Launch")).toHaveAttribute("aria-current", "true");
    expect(columnRoot("Inbox")).not.toHaveAttribute("aria-current", "true");
    expect(columnRoot("Pre-production")).not.toHaveAttribute("aria-current", "true");
    expect(columnRoot("Production")).not.toHaveAttribute("aria-current", "true");
  });

  it("paints Done and Blocked work-state labels", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-003-done",
              title: "SYS-003-done",
              track: "A",
              work_state: "done",
            }),
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-004-blocked",
              title: "SYS-004-blocked",
              track: "A",
              work_state: "blocked",
            }),
          ],
        })}
      />,
    );

    expect(cardByText("Production", "SYS-004-blocked")).toHaveTextContent("Blocked");
    expect(within(columnRoot("Production")).queryByText("SYS-003-done")).toBeNull();
    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    expect(cardByText("Production", "SYS-003-done")).toHaveTextContent("Done");
  });

  it("does not dim or hide non-current columns", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [],
          preproduction: [],
          postproduction: [],
        })}
      />,
    );

    for (const name of ["Inbox", "Pre-production", "Post-production & Launch"]) {
      const col = columnRoot(name);
      expect(col).toBeVisible();
      expect(col.className).not.toMatch(/opacity|hidden|invisible/);
    }
    expect(columnRoot("Production")).toHaveAttribute("aria-current", "true");
  });

  it("Limit — missing conversion link sits beside a SYS card", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md",
              title: "inbox",
              summary: "Filter unread first.",
              plan_title: "Inbox Plan",
              continue: "/gamedev-skill continue",
            }),
          ],
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-inbox",
              title: "SYS-001-inbox",
              track: "A",
              work_state: "pending",
              owner: "@gameplay-engineer",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
          ],
        })}
      />,
    );

    expect(cardByText("Inbox", ".grill/plans/inbox-plan/epics/epic-001-inbox.md")).toBeInTheDocument();
    expect(cardByText("Production", "SYS-001-inbox")).toBeInTheDocument();
  });

  it("keeps chrome continue as text when cards have continue buttons", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          continue: "/gamedev-skill continue",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
          ],
        })}
      />,
    );

    const chrome = [...pane().querySelectorAll("p")].find((el) => el.textContent === "/gamedev-skill continue");
    expect(chrome).toBeTruthy();
    expect(chrome?.tagName).toBe("P");
    expect(screen.getByRole("button", { name: "/gamedev-skill continue @gameplay-engineer" })).toBeInTheDocument();
    expect(screen.queryByRole("button", { name: /^\/gamedev-skill continue$/ })).toBeNull();
  });

  it("Track A SYS card expands with blurb, tasks, and Inputs", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "pending",
              owner: "@gameplay-engineer",
              continue: "/gamedev-skill continue @gameplay-engineer",
              blurb: "Moves the tetromino left and right. DAS applies after the first tap.",
              inputs: "Grid occupancy from collision. Outputs a new piece position.",
              tasks: [
                { number: 1, name: "Parse input", done: true },
                { number: 2, name: "Apply DAS", done: false },
              ],
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement");
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "false");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(card).toHaveTextContent("Moves the tetromino left and right. DAS applies after the first tap.");
    expect(card).toHaveTextContent("#1 Parse input");
    expect(card).toHaveTextContent("#2 Apply DAS");
    expect(card).toHaveTextContent("Inputs");
    expect(card).toHaveTextContent("Grid occupancy from collision. Outputs a new piece position.");
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(card).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
    expect(screen.queryByRole("dialog")).toBeNull();
    expect(screen.queryByRole("alertdialog")).toBeNull();
  });

  it("Track B gdd expand shows header only", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              last_decision: "Lock the loop as rotating tetrominoes.",
              open: "Need player fantasy one-liner.",
              blurb: "",
              tasks: [],
              inputs: "",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(card).toHaveTextContent("Lock the loop as rotating tetrominoes.");
    expect(card).toHaveTextContent("Need player fantasy one-liner.");
    expect(card).not.toHaveTextContent("No tasks planned yet");
    expect(card).not.toHaveTextContent("Inputs");
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
  });

  it("blocked strip and overlay paint on that owner's cards", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          blocked: [
            {
              owner: "@gameplay-engineer",
              task: "Combat system v2",
              blocked_by: "Waiting on final boss design",
            },
          ],
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              owner: "@game-designer",
              blocked_by: null,
            }),
          ],
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "blocked",
              owner: "@gameplay-engineer",
              blocked_by: "Waiting on final boss design",
            }),
          ],
        })}
      />,
    );

    const strip = screen.getByRole("region", { name: messages.en.gameBoard.blockedStrip });
    expect(strip).toHaveTextContent("@gameplay-engineer");
    expect(strip).toHaveTextContent("Waiting on final boss design");
    expect(within(strip).queryByRole("button")).toBeNull();
    expect(within(strip).queryByRole("link")).toBeNull();
    fireEvent.click(strip);
    expect(strip).toBeInTheDocument();

    const sys = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement");
    expect(sys).toHaveTextContent("Blocked");
    expect(sys).toHaveTextContent("Waiting on final boss design");
    expect(titleControl(sys)).toHaveAttribute("aria-expanded", "false");
    const gdd = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    expect(gdd).not.toHaveTextContent("Waiting on final boss design");
  });

  it("Limit — Inbox expand has no Archive and no No tasks planned yet", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: ".grill/plans/bevy-funnel/epics/epic-003-juice.md",
              title: "juice",
              summary: "Juice pass",
              plan_title: "Bevy funnel",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Inbox", ".grill/plans/bevy-funnel/epics/epic-003-juice.md");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(card).toHaveTextContent("Juice pass");
    expect(card).toHaveTextContent("Bevy funnel");
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(card).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
    expect(card).not.toHaveTextContent("No tasks planned yet");
  });

  it("Limit — empty Track A tasks still expands", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-002-score",
              title: "SYS-002-score",
              track: "A",
              blurb: "Adds a score counter.",
              tasks: [],
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-002-score");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(card).toHaveTextContent("Adds a score counter.");
    expect(card).toHaveTextContent("No tasks planned yet");
  });

  it("Limit — missing Inputs heading omits the region", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              blurb: "Moves the tetromino.",
              inputs: "",
              tasks: [{ number: 1, name: "Parse input", done: true }],
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(card).not.toHaveTextContent("Inputs");
  });

  it("Limit — two cards stay expanded and continue does not toggle", async () => {
    mockUiFetch();
    const writeText = stubWriteText(async () => undefined);
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              owner: "@game-designer",
              continue: "/gamedev-skill continue @game-designer",
              last_decision: "Lock the loop as rotating tetrominoes.",
            }),
          ],
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              blurb: "Moves the tetromino left and right. DAS applies after the first tap.",
              continue: "/gamedev-skill continue @gameplay-engineer",
            }),
          ],
        })}
      />,
    );

    const gdd = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    const sys = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement");
    fireEvent.click(titleControl(gdd));
    fireEvent.click(titleControl(sys));
    expect(titleControl(gdd)).toHaveAttribute("aria-expanded", "true");
    expect(titleControl(sys)).toHaveAttribute("aria-expanded", "true");

    const cont = within(gdd).getByRole("button", { name: "/gamedev-skill continue @game-designer" });
    fireEvent.click(cont);
    await waitFor(() => {
      expect(writeText).toHaveBeenCalledWith("/gamedev-skill continue @game-designer");
    });
    expect(titleControl(gdd)).toHaveAttribute("aria-expanded", "true");
  });

  it("Limit — empty blocked omits the strip", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          blocked: [],
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
            }),
          ],
        })}
      />,
    );

    expect(screen.queryByRole("region", { name: messages.en.gameBoard.blockedStrip })).toBeNull();
    expect(pane()).toHaveAttribute("aria-label", messages.en.tabs.game);
  });

  it("chrome continue p is not an expand control; remount and project change collapse", () => {
    mockUiFetch();
    const first = board({
      phase: "02-production",
      continue: "/gamedev-skill continue",
      production: [
        artifact({
          id: ".gamedev/phases/02-production/systems/SYS-001-movement",
          title: "SYS-001-movement",
          track: "A",
          blurb: "Moves the tetromino.",
        }),
      ],
    });
    const { rerender, unmount } = render(<GameBoardTab board={first} project="bevy" />);
    const card = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");

    const chrome = [...pane().querySelectorAll("p")].find((el) => el.textContent === "/gamedev-skill continue");
    expect(chrome?.tagName).toBe("P");
    fireEvent.click(chrome as HTMLElement);
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");

    rerender(<GameBoardTab board={first} project="alpha" />);
    const afterProject = cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement");
    expect(titleControl(afterProject)).toHaveAttribute("aria-expanded", "false");

    fireEvent.click(titleControl(afterProject));
    rerender(
      <GameBoardTab
        board={{
          ...first,
          focus: "refetch",
        }}
        project="alpha"
      />,
    );
    expect(
      titleControl(cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement")),
    ).toHaveAttribute("aria-expanded", "true");

    unmount();
    mockUiFetch();
    render(<GameBoardTab board={first} project="alpha" />);
    expect(
      titleControl(cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement")),
    ).toHaveAttribute("aria-expanded", "false");
  });

  it("LVL expand shows recent changelog when JSON recent is set", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/levels/LVL-001-well",
              title: "LVL-001-well",
              track: "H",
              last_decision: "Keep the well tight.",
              open: "Tune spawn.",
              recent: "line-ten",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Production", ".gamedev/phases/02-production/levels/LVL-001-well");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(card).toHaveTextContent("Keep the well tight.");
    expect(card).toHaveTextContent("Tune spawn.");
    expect(card).toHaveTextContent("line-ten");
    expect(card).not.toHaveTextContent("No tasks planned yet");
  });

  it("Archive hides a done artifact without a confirm dialog", async () => {
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    const store = {
      board: board({
        phase: "01-preproduction",
        preproduction: [
          artifact({
            id: audioId,
            title: "audio-direction.md",
            track: "B",
            work_state: "done",
            archived: false,
          }),
        ],
      }),
    };
    const fetchMock = mockArchiveFetch(store);
    const confirm = vi.spyOn(window, "confirm");

    render(<LiveGameBoardTab store={store} project="bevy" />);
    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    const card = cardByText("Pre-production", audioId);
    fireEvent.click(titleControl(card));
    fireEvent.click(within(card).getByRole("button", { name: "Archive" }));

    await waitFor(() => {
      expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
    });
    expect(cardsIn("Pre-production")).toHaveLength(0);
    expect(confirm).not.toHaveBeenCalled();
    expect(screen.queryByRole("dialog")).toBeNull();
    expect(screen.queryByRole("alertdialog")).toBeNull();
    expect(lastGameBoardPost(fetchMock)).toEqual({
      project: "bevy",
      card_id: audioId,
      archived: true,
    });
    expect(postedTo(fetchMock, "/api/spec-board")).toBe(false);
  });

  it("Limit — Show archived is session-only and remount starts hidden", () => {
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    const archivedBoard = board({
      phase: "01-preproduction",
      preproduction: [
        artifact({
          id: audioId,
          title: "audio-direction.md",
          track: "B",
          work_state: "done",
          archived: true,
        }),
      ],
    });
    mockUiFetch();
    const { unmount, rerender } = render(
      <GameBoardTab board={archivedBoard} project="bevy" />,
    );
    const toggle = screen.getByRole("button", { name: "Show archived" });
    expect(toggle).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Show archived" })).toBeNull();

    fireEvent.click(toggle);
    expect(toggle).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Pre-production", audioId)).toBeInTheDocument();

    rerender(<GameBoardTab board={archivedBoard} project="bevy" />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");

    unmount();
    mockUiFetch();
    render(<GameBoardTab board={archivedBoard} project="bevy" />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
  });

  it("Limit — Unarchive restores the card while the toggle is off", async () => {
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    const store = {
      board: board({
        phase: "01-preproduction",
        preproduction: [
          artifact({
            id: audioId,
            title: "audio-direction.md",
            track: "B",
            work_state: "done",
            archived: true,
          }),
        ],
      }),
    };
    const fetchMock = mockArchiveFetch(store);
    const confirm = vi.spyOn(window, "confirm");

    render(<LiveGameBoardTab store={store} project="bevy" />);
    const toggle = screen.getByRole("button", { name: "Show archived" });
    expect(toggle).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(toggle);
    expect(toggle).toHaveAttribute("aria-pressed", "true");
    const card = cardByText("Pre-production", audioId);
    fireEvent.click(titleControl(card));
    expect(within(card).getByRole("button", { name: "Unarchive" })).toBeInTheDocument();
    expect(within(card).queryByRole("button", { name: "Archive" })).toBeNull();
    fireEvent.click(within(card).getByRole("button", { name: "Unarchive" }));

    await waitFor(() => {
      expect(lastGameBoardPost(fetchMock).archived).toBe(false);
    });
    expect(lastGameBoardPost(fetchMock)).toEqual({
      project: "bevy",
      card_id: audioId,
      archived: false,
    });
    expect(confirm).not.toHaveBeenCalled();
    expect(screen.queryByRole("dialog")).toBeNull();

    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Pre-production", audioId)).toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
  });

  it("Limit — leftover archived on pending does not hide", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              archived: true,
              last_decision: "Lock the loop as rotating tetrominoes.",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    const card = cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md");
    fireEvent.click(titleControl(card));
    expect(titleControl(card)).toHaveAttribute("aria-expanded", "true");
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(card).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
  });

  it("Limit — all artifacts in a column archived leaves header only", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/audio-direction.md",
              title: "audio-direction.md",
              track: "B",
              work_state: "done",
              archived: true,
            }),
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "done",
              archived: true,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    expect(screen.getByRole("heading", { name: "Pre-production" })).toBeInTheDocument();
    expect(cardsIn("Pre-production")).toHaveLength(0);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(pane()).not.toHaveTextContent("No specs yet");
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Show archived" })).toBeNull();
  });

  it("Inbox archived epic stays visible and never shows Archive", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: ".grill/plans/bevy-funnel/epics/epic-003-juice.md",
              title: "juice",
              summary: "Juice pass",
              plan_title: "Bevy funnel",
              archived: true,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    const card = cardByText("Inbox", ".grill/plans/bevy-funnel/epics/epic-003-juice.md");
    fireEvent.click(titleControl(card));
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(card).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
  });

  it("expanded in_progress and blocked artifacts have no Archive", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-005-wip",
              title: "SYS-005-wip",
              track: "A",
              work_state: "in_progress",
            }),
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-004-blocked",
              title: "SYS-004-blocked",
              track: "A",
              work_state: "blocked",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    const wip = cardByText("Production", "SYS-005-wip");
    fireEvent.click(titleControl(wip));
    expect(within(wip).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    const blocked = cardByText("Production", "SYS-004-blocked");
    fireEvent.click(titleControl(blocked));
    expect(within(blocked).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(blocked).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
  });

  it("project change resets Show archived; GET refetch does not", () => {
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    const archivedBoard = board({
      phase: "01-preproduction",
      preproduction: [
        artifact({
          id: audioId,
          title: "audio-direction.md",
          track: "B",
          work_state: "done",
          archived: true,
        }),
      ],
    });
    mockUiFetch();
    const { rerender } = render(<GameBoardTab board={archivedBoard} project="bevy" />);
    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    fireEvent.click(screen.getByRole("button", { name: "Track B" }));
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track B" })).toHaveAttribute("aria-pressed", "true");

    rerender(
      <GameBoardTab
        board={{ ...archivedBoard, focus: "refetch" }}
        project="bevy"
      />,
    );
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track B" })).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Pre-production", audioId)).toBeInTheDocument();

    rerender(<GameBoardTab board={archivedBoard} project="alpha" />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track B" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
  });

  it("First paint hides done artifacts and selects All", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "done",
              archived: false,
            }),
            artifact({
              id: ".gamedev/phases/01-preproduction/narrative-bible.md",
              title: "narrative-bible.md",
              track: "B",
              work_state: "pending",
              archived: false,
            }),
          ],
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "pending",
              archived: false,
            }),
            artifact({
              id: ".gamedev/phases/02-production/levels/LVL-001-well",
              title: "LVL-001-well",
              track: "H",
              work_state: "in_progress",
              archived: false,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track A" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "Track B" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(".gamedev/phases/01-preproduction/gdd.md")).toBeNull();
    expect(cardByText("Pre-production", ".gamedev/phases/01-preproduction/narrative-bible.md")).toBeInTheDocument();
    expect(cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement")).toBeInTheDocument();
    expect(cardByText("Production", ".gamedev/phases/02-production/levels/LVL-001-well")).toBeInTheDocument();
    const pipes = [...pane().querySelectorAll("span[aria-hidden='true']")].filter((el) => el.textContent === "|");
    expect(pipes).toHaveLength(2);
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Show Dones" })).toBeNull();
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Track A" })).toBeNull();
  });

  it("Show Dones reveals an unarchived done card", () => {
    mockUiFetch();
    const gddId = ".gamedev/phases/01-preproduction/gdd.md";
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: gddId,
              title: "gdd.md",
              track: "B",
              work_state: "done",
              archived: false,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    const toggle = screen.getByRole("button", { name: "Show Dones" });
    expect(toggle).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(gddId)).toBeNull();
    fireEvent.click(toggle);
    expect(toggle).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Pre-production", gddId)).toBeInTheDocument();
    fireEvent.click(toggle);
    expect(toggle).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(gddId)).toBeNull();
  });

  it("Limit — leftover archived pending stays visible with Show Dones off", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "pending",
              archived: true,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(cardByText("Pre-production", ".gamedev/phases/01-preproduction/gdd.md")).toBeInTheDocument();
  });

  it("Limit — remount resets all three controls", () => {
    mockUiFetch();
    const gddId = ".gamedev/phases/01-preproduction/gdd.md";
    const fixture = board({
      phase: "01-preproduction",
      preproduction: [
        artifact({
          id: gddId,
          title: "gdd.md",
          track: "B",
          work_state: "done",
          archived: false,
        }),
      ],
    });
    const { unmount } = render(<GameBoardTab board={fixture} project="bevy" />);
    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Track A" }));
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track A" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "false");
    unmount();
    mockUiFetch();
    render(<GameBoardTab board={fixture} project="bevy" />);
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track A" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(gddId)).toBeNull();
  });

  it("Error — Show Dones alone does not reveal an archived done card", () => {
    mockUiFetch();
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: audioId,
              title: "audio-direction.md",
              track: "B",
              work_state: "done",
              archived: true,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
  });

  it("Track A hides B and H", () => {
    const fetchMock = mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "pending",
            }),
            artifact({
              id: ".gamedev/phases/02-production/art/piece-set",
              title: "piece-set",
              track: "B",
              work_state: "pending",
            }),
            artifact({
              id: ".gamedev/phases/02-production/levels/LVL-001-well",
              title: "LVL-001-well",
              track: "H",
              work_state: "pending",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    const before = fetchMock.mock.calls.length;
    fireEvent.click(screen.getByRole("button", { name: "Track A" }));
    expect(screen.getByRole("button", { name: "Track A" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "false");
    expect(cardByText("Production", ".gamedev/phases/02-production/systems/SYS-001-movement")).toBeInTheDocument();
    expect(within(columnRoot("Production")).queryByText(".gamedev/phases/02-production/art/piece-set")).toBeNull();
    expect(within(columnRoot("Production")).queryByText(".gamedev/phases/02-production/levels/LVL-001-well")).toBeNull();
    expect(fetchMock.mock.calls.length).toBe(before);
    expect(gameBoardRequests(fetchMock)).toHaveLength(0);
  });

  it("Both toggles on reveal an archived done card", () => {
    mockUiFetch();
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: audioId,
              title: "audio-direction.md",
              track: "B",
              work_state: "done",
              archived: true,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Pre-production", audioId)).toBeInTheDocument();
  });

  it("Limit Case — Inbox stays visible under every filter", () => {
    mockUiFetch();
    const inboxId = ".grill/plans/bevy-funnel/epics/epic-003-juice.md";
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: inboxId,
              title: "juice",
              summary: "Juice pass",
              plan_title: "Bevy funnel",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Track A" }));
    expect(cardByText("Inbox", inboxId)).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: "Track B" }));
    expect(cardByText("Inbox", inboxId)).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(cardByText("Inbox", inboxId)).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(cardByText("Inbox", inboxId)).toBeInTheDocument();
  });

  it("Limit Case — Track B then All restores H", () => {
    mockUiFetch();
    const wellId = ".gamedev/phases/02-production/levels/LVL-001-well";
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          production: [
            artifact({
              id: wellId,
              title: "LVL-001-well",
              track: "H",
              work_state: "pending",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Track B" }));
    expect(within(columnRoot("Production")).queryByText(wellId)).toBeNull();
    fireEvent.click(screen.getByRole("button", { name: "All" }));
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Production", wellId)).toBeInTheDocument();
  });

  it("Limit Case — project change resets filters; GET refetch does not", () => {
    mockUiFetch();
    const gddId = ".gamedev/phases/01-preproduction/gdd.md";
    const fixture = board({
      phase: "01-preproduction",
      preproduction: [
        artifact({
          id: gddId,
          title: "gdd.md",
          track: "B",
          work_state: "done",
          archived: false,
        }),
      ],
    });
    const { rerender } = render(<GameBoardTab board={fixture} project="bevy" />);
    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Track B" }));

    rerender(<GameBoardTab board={{ ...fixture, focus: "refetch" }} project="bevy" />);
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Track B" })).toHaveAttribute("aria-pressed", "true");
    expect(cardByText("Pre-production", gddId)).toBeInTheDocument();

    rerender(<GameBoardTab board={fixture} project="other-game" />);
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.getByRole("button", { name: "All" })).toHaveAttribute("aria-pressed", "true");
    expect(within(columnRoot("Pre-production")).queryByText(gddId)).toBeNull();
  });

  it("Limit Case — all phase cards hidden leaves header only", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "done",
              archived: false,
            }),
            artifact({
              id: ".gamedev/phases/01-preproduction/audio-direction.md",
              title: "audio-direction.md",
              track: "B",
              work_state: "done",
              archived: false,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    expect(screen.getByRole("heading", { name: "Pre-production" })).toBeInTheDocument();
    expect(cardsIn("Pre-production")).toHaveLength(0);
    expect(screen.getByRole("button", { name: "Show Dones" })).toBeInTheDocument();
    expect(pane()).not.toHaveTextContent("No specs yet");
  });

  it("Limit Case — chrome has separators and Inbox does not copy the controls", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: ".grill/plans/bevy-funnel/epics/epic-003-juice.md",
              title: "juice",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    expect(pane()).toHaveTextContent("|");
    expect(screen.getByRole("button", { name: "Show archived" })).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "Show Dones" })).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "Track A" })).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "Track B" })).toBeInTheDocument();
    expect(screen.getByRole("button", { name: "All" })).toBeInTheDocument();
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Show Dones" })).toBeNull();
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "Track A" })).toBeNull();
    expect(within(columnRoot("Inbox")).queryByRole("button", { name: "All" })).toBeNull();
  });

  it("Error — Show archived alone does not reveal an archived done card", () => {
    mockUiFetch();
    const audioId = ".gamedev/phases/01-preproduction/audio-direction.md";
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: audioId,
              title: "audio-direction.md",
              track: "B",
              work_state: "done",
              archived: true,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Pre-production")).queryByText(audioId)).toBeNull();
  });

  it("Error — Track A plus Show Dones still hides a Track B done card", () => {
    mockUiFetch();
    const gddId = ".gamedev/phases/01-preproduction/gdd.md";
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: gddId,
              title: "gdd.md",
              track: "B",
              work_state: "done",
              archived: false,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Track A" }));
    expect(within(columnRoot("Pre-production")).queryByText(gddId)).toBeNull();
  });

  it("Error — filters do not add query params or write skill trees", () => {
    installMemoryLocalStorage();
    const fetchMock = mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              track: "B",
              work_state: "done",
              archived: false,
            }),
          ],
        })}
        project="bevy"
      />,
    );

    fireEvent.click(screen.getByRole("button", { name: "Show Dones" }));
    fireEvent.click(screen.getByRole("button", { name: "Track B" }));
    expect(gameBoardRequests(fetchMock)).toHaveLength(0);
    expect(gameBoardUrlHasQuery(fetchMock, "track")).toBe(false);
    expect(gameBoardUrlHasQuery(fetchMock, "show_dones")).toBe(false);
    expect(gameBoardUrlHasQuery(fetchMock, "show_archived")).toBe(false);
    expect(localStorageHasFilterKey()).toBe(false);
  });

  // Gherkin: Inbox id wraps the full path under the short name
  it("wraps the inbox id line and keeps title truncate", () => {
    mockUiFetch();
    const inboxId = ".grill/plans/inbox-plan/epics/epic-001-inbox.md";
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: inboxId,
              title: "inbox",
              summary: "Filter unread first.",
              plan_title: "Inbox Plan",
            }),
          ],
        })}
      />,
    );

    const card = cardByText("Inbox", inboxId);
    expect(within(card).getByText(inboxId)).toBeInTheDocument();
    expect(within(card).getByText("inbox")).toBeInTheDocument();
    expect(within(card).getByText("E", { exact: true })).toBeInTheDocument();
    const idLine = within(card).getByText(inboxId);
    expect(idLine.className).not.toMatch(/\btruncate\b/);
    expect(idLine.className).toMatch(/whitespace-normal/);
    expect(idLine.className).toMatch(/break-all/);
    expect(getComputedStyle(idLine).textOverflow).not.toBe("ellipsis");
    const titleLine = within(card).getByText("inbox");
    expect(titleLine.className).toMatch(/\btruncate\b/);
  });

  // Gherkin: Limit — empty Inbox after hide keeps header only
  it("Limit — empty Inbox after hide keeps header only", () => {
    mockUiFetch();
    const omittedId = ".grill/plans/inbox-plan/epics/epic-001-inbox.md";
    render(
      <GameBoardTab
        board={board({
          gamedev_skill_present: true,
          phase: "02-production",
          inbox: [],
        })}
      />,
    );

    const col = columnRoot("Inbox");
    expect(col).toBeInTheDocument();
    expect(cardsIn("Inbox")).toHaveLength(0);
    expect(within(col).queryByText(omittedId)).toBeNull();
    expect(col).not.toHaveTextContent("all tracked");
    expect(col).not.toHaveTextContent("no artifacts in this phase");
  });

  // Gherkin: Limit — artifact id line still truncates
  it("keeps truncate on the artifact card id line", () => {
    mockUiFetch();
    const gddId = ".gamedev/phases/01-preproduction/gdd.md";
    render(
      <GameBoardTab
        board={board({
          phase: "01-preproduction",
          preproduction: [
            artifact({
              id: gddId,
              title: "gdd.md",
              track: "B",
              work_state: "pending",
            }),
          ],
        })}
      />,
    );

    const artifactId = within(cardByText("Pre-production", gddId)).getByText(gddId);
    expect(artifactId.className).toMatch(/\btruncate\b/);
  });

  // Gherkin: Limit — Show Dones off still shows a visible Inbox card
  it("Limit — Show Dones off still shows a visible Inbox card", () => {
    mockUiFetch();
    const inboxId = ".grill/plans/inbox-plan/epics/epic-001-inbox.md";
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          inbox: [
            epic({
              id: inboxId,
              title: "inbox",
            }),
          ],
        })}
        project="bevy"
      />,
    );

    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(cardByText("Inbox", inboxId)).toBeInTheDocument();
  });

  it("paints Open tech debt after Blocked and before the four columns", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          blocked: [{ owner: "@producer", task: "lock", blocked_by: "gdd" }],
          inbox: [epic({ id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md", title: "inbox" })],
          debt: [{ id: "debt:gate-preproduction", title: "missing GDD lock" }],
        })}
      />,
    );

    const debtRegion = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(debtRegion).toHaveTextContent("debt:gate-preproduction");
    expect(debtRegion).toHaveTextContent("missing GDD lock");
    expect(debtRegion).not.toHaveTextContent("director");
    expect(debtRegion).not.toHaveTextContent("M1");
    expect(cardByText("Inbox", "inbox")).toBeInTheDocument();
    const blocked = screen.getByRole("region", { name: messages.en.gameBoard.blockedStrip });
    const cols = pane().querySelector(".grid.grid-cols-4");
    expect(cols).not.toBeNull();
    expect(blocked.compareDocumentPosition(debtRegion) & Node.DOCUMENT_POSITION_FOLLOWING).not.toBe(0);
    expect(debtRegion.compareDocumentPosition(cols as Node) & Node.DOCUMENT_POSITION_FOLLOWING).not.toBe(0);
  });

  it("omits Open tech debt when debt is an empty array and still shows Game", () => {
    mockUiFetch();
    render(<GameBoardTab board={board({ debt: [] })} />);
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(pane()).toBeInTheDocument();
  });

  it("wraps a long Game debt title in the Open tech debt region", () => {
    mockUiFetch();
    const longTitle = "a very long missing GDD lock title that exceeds one chrome line";
    render(
      <GameBoardTab
        board={board({
          debt: [{ id: "debt:gate-preproduction", title: longTitle }],
        })}
      />,
    );
    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(region).toHaveTextContent(longTitle);
    const titleLine = within(region).getByText(new RegExp(longTitle));
    expect(titleLine.tagName).toBe("P");
    expect(titleLine.className).toMatch(/whitespace-normal/);
    expect(titleLine.className).toMatch(/break-words/);
    expect(titleLine.className).not.toMatch(/\btruncate\b/);
    expect(getComputedStyle(titleLine).textOverflow).not.toBe("ellipsis");
  });

  it("does not POST, copy, or expand when the operator activates a Game debt row", () => {
    const fetchMock = mockArchiveFetch({
      board: board({
        preproduction: [
          artifact({
            id: ".gamedev/phases/01-preproduction/gdd.md",
            title: "gdd.md",
            work_state: "pending",
          }),
        ],
        debt: [{ id: "debt:gate-preproduction", title: "missing GDD lock" }],
      }),
    });
    const writeText = vi.fn(async () => undefined);
    Object.defineProperty(navigator, "clipboard", {
      configurable: true,
      value: { writeText },
    });
    render(
      <GameBoardTab
        board={board({
          preproduction: [
            artifact({
              id: ".gamedev/phases/01-preproduction/gdd.md",
              title: "gdd.md",
              work_state: "pending",
            }),
          ],
          debt: [{ id: "debt:gate-preproduction", title: "missing GDD lock" }],
        })}
        project="bevy"
      />,
    );
    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    const row = within(region).getByText(/debt:gate-preproduction/);
    expect(row.tagName).toBe("P");
    fireEvent.click(row);
    fireEvent.click(region);
    const posts = fetchMock.mock.calls.filter((args) => {
      const url = String(args[0]);
      const init = args[1] as RequestInit | undefined;
      return (init?.method ?? "GET").toUpperCase() === "POST" && url.includes("/api/game-board");
    });
    expect(posts).toHaveLength(0);
    expect(writeText).not.toHaveBeenCalled();
    expect(titleControl(cardByText("Pre-production", "gdd.md"))).toHaveAttribute("aria-expanded", "false");
    expect(within(region).queryByRole("button")).toBeNull();
    expect(within(region).queryByRole("link")).toBeNull();
  });

  it("keeps the Game debt row when Show Dones is off", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          production: [
            artifact({
              id: ".gamedev/phases/02-production/systems/SYS-001-movement",
              title: "SYS-001-movement",
              track: "A",
              work_state: "done",
            }),
          ],
          debt: [{ id: "debt:gate-preproduction", title: "missing GDD lock" }],
        })}
      />,
    );
    expect(screen.getByRole("button", { name: "Show Dones" })).toHaveAttribute("aria-pressed", "false");
    expect(cardsIn("Production")).toHaveLength(0);
    expect(screen.getByRole("region", { name: messages.en.specBoard.openTechDebt })).toHaveTextContent(
      "debt:gate-preproduction",
    );
  });

  it("paints 16 Game debt rows without a Has more control", () => {
    mockUiFetch();
    const debt = Array.from({ length: 16 }, (_, i) => ({
      id: `debt:d${String(i + 1).padStart(2, "0")}`,
      title: `item ${i + 1}`,
    }));
    render(<GameBoardTab board={board({ debt })} />);
    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(region).toHaveTextContent("debt:d01");
    expect(region).toHaveTextContent("debt:d16");
    expect(region).not.toHaveTextContent("debt:d17");
    expect(screen.queryByRole("button", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByRole("link", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByText("Has more")).toBeNull();
  });

  // Gherkin: Limit — Graph ADR and Game do not show the strip (Game half)
  it("does not show an Open tech debt region on mount", () => {
    mockUiFetch();
    render(
      <GameBoardTab
        board={board({
          phase: "02-production",
          focus: "Ship feel",
          continue: "/gamedev-skill continue",
        })}
      />,
    );

    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(screen.queryByRole("region", { name: "Open tech debt" })).toBeNull();
    expect(screen.queryByText("TD-005")).toBeNull();
    expect(pane()).toHaveAttribute("aria-label", messages.en.tabs.game);
  });

  it("locks colorForLabel Function hex", () => {
    expect(colorForLabel("Function")).toBe("#06b6d4");
  });
});
