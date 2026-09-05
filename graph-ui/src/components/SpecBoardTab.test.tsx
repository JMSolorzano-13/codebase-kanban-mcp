/**
 * @sdd-task: Task #4 - leftover Vitest locks
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-067 - Specs-only debt strip; SDD-ADR-068 - EpicCard id wraps
 * @sdd-why: leftover UI Thens — dead row, Has more omit, grill-only, Companion-to
 * @human-debug: If debt click POSTs → row became a control; if Has more appears → overflow chrome leaked; if converted epic paints → UI re-matched Companion-to
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor, within } from "@testing-library/react";
import { useCallback, useState } from "react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { colorForLabel } from "../lib/colors";
import { messages } from "../lib/i18n";
import type { SpecBoard, SpecBoardDebt, SpecBoardEntry, SpecBoardEpic } from "../lib/types";
import { SpecBoardTab } from "./SpecBoardTab";

vi.mock("../hooks/useSpecBoard", () => ({
  useSpecBoard: vi.fn(),
}));

import { useSpecBoard } from "../hooks/useSpecBoard";

function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function mockUiFetch(live?: { specs: SpecBoardEntry[] }) {
  vi.stubGlobal(
    "fetch",
    vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
      if (String(input).startsWith("/api/ui-config")) {
        return json({ lang: "en" });
      }
      if (String(input) === "/api/spec-board" && String(init?.method ?? "GET").toUpperCase() === "POST") {
        const body = JSON.parse(String(init?.body ?? "{}")) as {
          project?: string;
          spec_id?: string;
          archived?: boolean;
        };
        if (live && typeof body.spec_id === "string" && typeof body.archived === "boolean") {
          live.specs = live.specs.map((spec) =>
            spec.id === body.spec_id ? { ...spec, archived: body.archived } : spec,
          );
        }
        return json({
          project: body.project,
          spec_id: body.spec_id,
          archived: body.archived,
        });
      }
      return json({});
    }),
  );
}

function board(present: boolean, specs: SpecBoardEntry[] = []): SpecBoard {
  return { sdd_skill_present: present, specs };
}

function entry(partial: Partial<SpecBoardEntry> & Pick<SpecBoardEntry, "id" | "column">): SpecBoardEntry {
  return {
    title: "",
    blurb: "",
    active: false,
    current_agent: "",
    blocked_note: "",
    task_count: 0,
    tasks_done: 0,
    checklist_percent: -1,
    tasks: [],
    archived: false,
    ...partial,
  };
}

const planned = entry({
  id: "spec-010-aaa-planned",
  title: "Spec-010-aaa: Planned Work",
  blurb: "Planned work ships the inbox filter. Operators see unread first.",
  column: "todo",
  task_count: 2,
  tasks_done: 1,
  tasks: [
    { number: 1, name: "Write parser", done: true, current: false },
    { number: 2, name: "Write tests", done: false, current: false },
  ],
});

const active = entry({
  id: "spec-011-bbb-active",
  title: "Spec-011-bbb: Active Work",
  blurb: "Active work fills the card.",
  column: "in_progress",
  active: true,
  current_agent: "implementer",
  task_count: 2,
  tasks_done: 1,
  tasks: [
    { number: 1, name: "Write parser", done: true, current: false },
    { number: 2, name: "Write tests", done: false, current: true },
  ],
});

const closed = entry({
  id: "spec-012-ccc-closed",
  title: "Spec-012-ccc: Closed Work",
  blurb: "Closed work is still readable.",
  column: "done",
  task_count: 2,
  tasks_done: 2,
  tasks: [
    { number: 1, name: "Write parser", done: true, current: false },
    { number: 2, name: "Write tests", done: true, current: false },
  ],
});

const emptyBlurb = entry({
  id: "spec-013-ddd-empty",
  title: "Spec-013-ddd: Empty ES",
  blurb: "",
  column: "todo",
  task_count: 1,
  tasks: [{ number: 1, name: "Write parser", done: false, current: false }],
});

const bare = entry({
  id: "spec-014-eee-bare",
  title: "Spec-014-eee: Bare",
  blurb: "Bare spec still opens.",
  column: "todo",
  task_count: 0,
  tasks: [],
});

const visibleDone = entry({
  id: "spec-017-hhh-visible",
  title: "Spec-017-hhh: Visible",
  blurb: "Still in Done.",
  column: "done",
});

const archivedClosed = { ...closed, archived: true };

function makeEpic(partial: Partial<SpecBoardEpic> & Pick<SpecBoardEpic, "id" | "title">): SpecBoardEpic {
  return {
    kind: "epic",
    summary: "",
    plan_title: "",
    column: "todo",
    ...partial,
  };
}

function makeDebt(partial: Partial<SpecBoardDebt> & Pick<SpecBoardDebt, "id" | "title">): SpecBoardDebt {
  return { ...partial };
}

const inboxEpic = makeEpic({
  id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md",
  title: "inbox",
  summary: "Filter unread first.",
  plan_title: "Inbox Plan",
});

const planAFirst = makeEpic({
  id: ".grill/plans/plan-a/epics/epic-001-first.md",
  title: "first",
  summary: "First in plan A.",
  plan_title: "Plan A",
});

const planASecond = makeEpic({
  id: ".grill/plans/plan-a/epics/epic-002-second.md",
  title: "second",
  summary: "Second in plan A.",
  plan_title: "Plan A",
});

const planBOther = makeEpic({
  id: ".grill/plans/plan-b/epics/epic-001-other.md",
  title: "other",
  summary: "First in plan B.",
  plan_title: "Plan B",
});

const similarlyNamed = entry({
  id: "spec-010-aaa-inbox",
  title: "Spec-010-aaa: Inbox",
  blurb: "Named like the epic, no Companion-to.",
  column: "todo",
});

function sixtyFourEpics(): SpecBoardEpic[] {
  return Array.from({ length: 64 }, (_, i) => {
    const n = String(i + 1).padStart(3, "0");
    return makeEpic({
      id: `.grill/plans/inbox-plan/epics/epic-${n}-item.md`,
      title: `item-${n}`,
      summary: `Summary ${n}.`,
      plan_title: "Inbox Plan",
    });
  });
}

function sixteenOpenDebt(): SpecBoardDebt[] {
  return Array.from({ length: 16 }, (_, i) => {
    const n = String(i + 1).padStart(3, "0");
    return makeDebt({ id: `TD-${n}`, title: `open ${n}` });
  });
}

function mockBoard(
  specs: SpecBoardEntry[],
  refresh: () => Promise<void> = async () => undefined,
  extra: Pick<SpecBoard, "grill_skill_present" | "epics" | "debt"> = {},
) {
  vi.mocked(useSpecBoard).mockReturnValue({
    board: { ...board(true, specs), ...extra },
    loading: false,
    error: null,
    refresh,
  });
}

function installLiveBoard(initial: SpecBoardEntry[]) {
  const store: { specs: SpecBoardEntry[] } = { specs: initial.map((spec) => ({ ...spec })) };
  const refreshInner = vi.fn(async () => undefined);

  vi.mocked(useSpecBoard).mockImplementation(() => {
    const [specs, setSpecs] = useState<SpecBoardEntry[]>(() => store.specs);
    const refresh = useCallback(async () => {
      setSpecs(store.specs.map((spec) => ({ ...spec, tasks: spec.tasks.map((task) => ({ ...task })) })));
      await refreshInner();
    }, []);
    return {
      board: board(true, specs),
      loading: false,
      error: null,
      refresh,
    };
  });

  return { store, refresh: refreshInner };
}

function titleButton(id: string) {
  return screen.getByRole("button", { name: new RegExp(id) });
}

function cardById(id: string): HTMLElement {
  const card = titleButton(id).closest("div");
  if (!card) throw new Error(`no card for ${id}`);
  return card;
}

function doneHeader(): HTMLElement {
  const heading = screen.getByRole("heading", { name: /^Done$/i });
  const header = heading.parentElement;
  if (!header) throw new Error("no Done header");
  return header;
}

function columnRoot(title: string): HTMLElement {
  const heading = screen.getByRole("heading", { name: new RegExp(`^${title}$`, "i") });
  const root = heading.closest(".flex-1");
  if (!root) throw new Error(`no column ${title}`);
  return root as HTMLElement;
}

function expectNoArchiveOrUnarchive(scope?: HTMLElement) {
  const root = scope ?? document.body;
  expect(within(root).queryByRole("button", { name: /^Archive$/i })).toBeNull();
  expect(within(root).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
  expect(within(root).queryByRole("link", { name: /^Archive$/i })).toBeNull();
  expect(within(root).queryByRole("link", { name: /^Unarchive$/i })).toBeNull();
}

function specBoardPostCalls(): unknown[] {
  return vi.mocked(fetch).mock.calls.filter(([url, init]) => {
    return (
      String(url) === "/api/spec-board" &&
      String((init as RequestInit | undefined)?.method ?? "GET").toUpperCase() === "POST"
    );
  });
}

function columnCardIds(title: string): string[] {
  return Array.from(columnRoot(title).querySelectorAll(".font-mono")).map(
    (el) => (el.textContent ?? "").trim(),
  );
}

function todoCardIds(): string[] {
  return columnCardIds("Todo");
}

function epicCardById(id: string): HTMLElement {
  const idNode = within(columnRoot("Todo")).getByText(id);
  const card = idNode.closest(".rounded-lg");
  if (!card) throw new Error(`no epic card for ${id}`);
  return card as HTMLElement;
}

async function lastSpecBoardPost(): Promise<{
  status: number;
  body: { project?: string; spec_id?: string; archived?: boolean };
}> {
  const mocked = vi.mocked(fetch);
  for (let i = mocked.mock.calls.length - 1; i >= 0; i--) {
    const [url, init] = mocked.mock.calls[i] as [RequestInfo, RequestInit | undefined];
    if (String(url) !== "/api/spec-board") continue;
    if (String(init?.method ?? "GET").toUpperCase() !== "POST") continue;
    const res = (await mocked.mock.results[i]?.value) as Response;
    return {
      status: res.status,
      body: (await res.clone().json()) as { project?: string; spec_id?: string; archived?: boolean },
    };
  }
  throw new Error("no POST /api/spec-board");
}

describe("SpecBoardTab workspace host", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("never renders selectProject when project is non-null", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: { ...board(true), grill_skill_present: true, epics: [inboxEpic] },
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.queryByText(messages.en.specBoard.selectProject)).toBeNull();
    expect(screen.getByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(useSpecBoard).toHaveBeenCalledWith("alpha");
  });

  it("does not show the picker while spec-board is loading for a project", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: null,
      loading: true,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.queryByText(messages.en.specBoard.selectProject)).toBeNull();
    expect(screen.getByText(messages.en.common.loading)).toBeInTheDocument();
    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
    expect(screen.queryByText(inboxEpic.id)).toBeNull();
  });

  it("paints specs when epics is missing from a sdd-true board without grill_skill_present", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: board(true, [planned]),
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
    expect(screen.getByText(planned.id)).toBeInTheDocument();
    expect(screen.getByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
  });

  // Gherkin: Grill-only Kanban paints an epic in Todo and empty spec columns
  it("paints grill-only Kanban with an epic in Todo and noSpecs in spec columns", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: {
        sdd_skill_present: false,
        grill_skill_present: true,
        specs: [],
        epics: [inboxEpic],
      },
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
    expect(screen.queryByText(messages.en.specBoard.selectProject)).toBeNull();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();

    const todo = columnRoot("Todo");
    const card = epicCardById(inboxEpic.id);
    expect(within(card).getByText("E")).toBeInTheDocument();
    expect(within(card).getByText("inbox")).toBeInTheDocument();
    expect(within(todo).getByText(inboxEpic.id)).toBeInTheDocument();

    expect(within(columnRoot("In Progress")).getByText(messages.en.specBoard.noSpecs)).toBeInTheDocument();
    expect(within(columnRoot("Done")).getByText(messages.en.specBoard.noSpecs)).toBeInTheDocument();
    expect(columnCardIds("In Progress")).not.toContain(inboxEpic.id);
    expect(columnCardIds("Done")).not.toContain(inboxEpic.id);
  });

  // Gherkin: Limit — grill directory with no epics still shows Specs (pane Thens)
  it("paints three noSpecs columns when grill-only has empty epics and specs", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: {
        sdd_skill_present: false,
        grill_skill_present: true,
        specs: [],
        epics: [],
      },
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
    expect(screen.getByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(within(columnRoot("Todo")).getByText(messages.en.specBoard.noSpecs)).toBeInTheDocument();
    expect(within(columnRoot("In Progress")).getByText(messages.en.specBoard.noSpecs)).toBeInTheDocument();
    expect(within(columnRoot("Done")).getByText(messages.en.specBoard.noSpecs)).toBeInTheDocument();
  });

  it("shows not-sdd-skill copy when board is null after load", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: null,
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.getByText(messages.en.specBoard.notSddSkill)).toBeInTheDocument();
    expect(screen.queryByText(messages.en.common.loading)).toBeNull();
    expect(screen.queryByText(messages.en.specBoard.columnTodo)).toBeNull();
  });

  it("shows not-sdd-skill copy when both skill flags are false on a stale mount", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: {
        sdd_skill_present: false,
        grill_skill_present: false,
        specs: [planned],
        epics: [inboxEpic],
      },
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.getByText(messages.en.specBoard.notSddSkill)).toBeInTheDocument();
    expect(screen.queryByText(messages.en.specBoard.columnTodo)).toBeNull();
    expect(screen.queryByText(inboxEpic.id)).toBeNull();
    expect(screen.queryByText(planned.id)).toBeNull();
  });
});

describe("SpecBoardTab expand Set + TaskList", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("expands a Todo card with blurb and pending tasks only", () => {
    mockUiFetch();
    mockBoard([planned, active]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(planned.id));

    const card = cardById(planned.id);
    expect(within(card).getByText(planned.blurb)).toBeInTheDocument();
    expect(within(card).getByText("#2 Write tests")).toBeInTheDocument();
    expect(within(card).queryByText("#1 Write parser")).toBeNull();
  });

  it("shows In Progress expanded on first paint with blurb, all tasks, and chrome", () => {
    mockUiFetch();
    mockBoard([planned, active]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const card = cardById(active.id);
    expect(titleButton(active.id)).toHaveAttribute("aria-expanded", "true");
    expect(within(card).getByText(active.blurb)).toBeInTheDocument();
    expect(within(card).getByText("#1 Write parser")).toBeInTheDocument();
    expect(within(card).getByText("#2 Write tests")).toBeInTheDocument();
    expect(within(card).getByText("implementer")).toBeInTheDocument();
    expect(within(card).getByText(messages.en.specBoard.tasksDone(1, 2))).toBeInTheDocument();
    expect(within(card).getByText("1/2 tasks")).toBeInTheDocument();
    expectNoArchiveOrUnarchive();
  });

  it("expands a Done card with the full task list and Archive", () => {
    mockUiFetch();
    mockBoard([closed]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(screen.queryByRole("button", { name: /^Archive$/i })).toBeNull();
    fireEvent.click(titleButton(closed.id));

    const card = cardById(closed.id);
    expect(within(card).getByText(closed.blurb)).toBeInTheDocument();
    expect(within(card).getByText("#1 Write parser")).toBeInTheDocument();
    expect(within(card).getByText("#2 Write tests")).toBeInTheDocument();
    expect(within(card).getByRole("button", { name: "Archive" })).toBeInTheDocument();
    expect(within(card).queryByRole("button", { name: "Unarchive" })).toBeNull();
  });

  it("keeps two cards expanded at once", () => {
    mockUiFetch();
    mockBoard([planned, active, closed]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(planned.id));
    expect(within(cardById(planned.id)).getByText(planned.blurb)).toBeInTheDocument();
    expect(titleButton(closed.id)).toHaveAttribute("aria-expanded", "false");

    fireEvent.click(titleButton(closed.id));

    expect(within(cardById(planned.id)).getByText(planned.blurb)).toBeInTheDocument();
    expect(within(cardById(closed.id)).getByText(closed.blurb)).toBeInTheDocument();
    expect(titleButton(planned.id)).toHaveAttribute("aria-expanded", "true");
    expect(titleButton(closed.id)).toHaveAttribute("aria-expanded", "true");
  });

  it("omits the blurb region when blurb is empty and still expands", () => {
    mockUiFetch();
    mockBoard([emptyBlurb]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(emptyBlurb.id));

    const card = cardById(emptyBlurb.id);
    expect(within(card).getByText("#1 Write parser")).toBeInTheDocument();
    expect(within(card).queryByText("KPI")).toBeNull();
    expect(card.querySelector("[data-region='blurb']")).toBeNull();
  });

  it("treats a missing blurb as empty", () => {
    mockUiFetch();
    const { blurb: _omit, ...rest } = emptyBlurb;
    void _omit;
    mockBoard([rest as SpecBoardEntry]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(emptyBlurb.id));

    expect(cardById(emptyBlurb.id).querySelector("[data-region='blurb']")).toBeNull();
    expect(screen.getByText("#1 Write parser")).toBeInTheDocument();
  });

  it("expands a zero-task Todo with no-tasks copy", () => {
    mockUiFetch();
    mockBoard([bare]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(bare.id));

    const card = cardById(bare.id);
    expect(within(card).getByText(bare.blurb)).toBeInTheDocument();
    expect(within(card).getByText(messages.en.specBoard.noTasksYet)).toBeInTheDocument();
  });

  it("keeps an opened Todo expanded when a new board object arrives", () => {
    mockUiFetch();
    mockBoard([planned, active]);

    const { rerender } = render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(planned.id));
    expect(screen.getByText(planned.blurb)).toBeInTheDocument();

    mockBoard([{ ...planned }, { ...active }]);
    rerender(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.getByText(planned.blurb)).toBeInTheDocument();
    expect(within(cardById(planned.id)).getByText("#2 Write tests")).toBeInTheDocument();
  });

  it("clears expanded ids and reseeds active on project change", () => {
    mockUiFetch();
    mockBoard([planned, active]);

    const { rerender } = render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(planned.id));
    expect(screen.getByText(planned.blurb)).toBeInTheDocument();

    const betaActive = entry({
      id: "spec-099-zzz-beta",
      title: "Beta active",
      blurb: "Beta card starts open.",
      column: "in_progress",
      active: true,
      current_agent: "review",
      task_count: 1,
      tasks_done: 0,
      tasks: [{ number: 1, name: "Beta task", done: false, current: true }],
    });
    mockBoard([betaActive]);
    rerender(<SpecBoardTab project="beta" onSelectProject={() => undefined} />);

    expect(screen.queryByText(planned.blurb)).toBeNull();
    expect(screen.getByText("Beta card starts open.")).toBeInTheDocument();
    expect(titleButton(betaActive.id)).toHaveAttribute("aria-expanded", "true");
  });

  it("keeps active chrome visible while the card is collapsed", () => {
    mockUiFetch();
    mockBoard([active]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(active.id));

    const card = cardById(active.id);
    expect(titleButton(active.id)).toHaveAttribute("aria-expanded", "false");
    expect(within(card).queryByText(active.blurb)).toBeNull();
    expect(within(card).getByText("implementer")).toBeInTheDocument();
    expect(within(card).getByText(messages.en.specBoard.tasksDone(1, 2))).toBeInTheDocument();
  });
});

describe("SpecBoardTab archive filter + toggle", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  // Gherkin: Fresh visit starts with archived hidden
  it("hides an archived Done card on first paint and keeps Show archived unpressed", () => {
    mockUiFetch();
    mockBoard([archivedClosed]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const toggle = screen.getByRole("button", { name: "Show archived" });
    expect(toggle).toHaveAttribute("aria-pressed", "false");
    expect(screen.queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();
    expect(within(columnRoot("Todo")).queryByRole("button", { name: "Show archived" })).toBeNull();
    expect(within(columnRoot("In Progress")).queryByRole("button", { name: "Show archived" })).toBeNull();
  });

  it("resets Show archived on remount", () => {
    mockUiFetch();
    mockBoard([archivedClosed]);

    const { unmount } = render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");
    expect(titleButton(closed.id)).toBeInTheDocument();

    unmount();
    mockBoard([archivedClosed]);
    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(screen.queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();
  });

  it("resets Show archived on project change with expandedIds", () => {
    mockUiFetch();
    mockBoard([archivedClosed]);

    const { rerender } = render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "true");

    mockBoard([visibleDone]);
    rerender(<SpecBoardTab project="beta" onSelectProject={() => undefined} />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
  });

  // Gherkin: Limit — Done count ignores hidden archived
  it("counts only shown Done cards", () => {
    mockUiFetch();
    mockBoard([archivedClosed, visibleDone]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(within(doneHeader()).getByText("1")).toBeInTheDocument();
    expect(titleButton(visibleDone.id)).toBeInTheDocument();
    expect(screen.queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();
  });

  // Gherkin: Limit — all Done archived shows empty copy and keeps the toggle
  it("shows empty Done copy and keeps the toggle when every Done spec is archived", () => {
    mockUiFetch();
    mockBoard([archivedClosed]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(within(doneHeader()).getByText("0")).toBeInTheDocument();
    expect(within(columnRoot("Done")).getByText(messages.en.specBoard.noSpecs)).toBeInTheDocument();
    expect(within(doneHeader()).getByRole("button", { name: "Show archived" })).toBeInTheDocument();
  });

  // Gherkin: Limit — leftover flag on a Todo spec does not hide it
  it("does not hide a leftover archived Todo and shows neither Archive nor Unarchive", () => {
    mockUiFetch();
    mockBoard([{ ...planned, archived: true }]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(titleButton(planned.id)).toBeInTheDocument();
    fireEvent.click(titleButton(planned.id));
    expectNoArchiveOrUnarchive(cardById(planned.id));
  });

  it("treats missing archived as false", () => {
    mockUiFetch();
    const { archived: _omit, ...rest } = closed;
    void _omit;
    mockBoard([rest as SpecBoardEntry]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(titleButton(closed.id)).toBeInTheDocument();
    fireEvent.click(titleButton(closed.id));
    expect(within(cardById(closed.id)).getByRole("button", { name: "Archive" })).toBeInTheDocument();
  });

  // Gherkin: Archive hides a Done card without a confirm dialog
  it("archives a Done card via POST without a confirm and hides it after refresh", async () => {
    const live = installLiveBoard([closed]);
    mockUiFetch(live.store);
    const confirm = vi.spyOn(window, "confirm");

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(closed.id));
    fireEvent.click(screen.getByRole("button", { name: "Archive" }));

    await waitFor(() => {
      expect(within(columnRoot("Done")).queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();
    });
    expect(confirm).not.toHaveBeenCalled();
    expect(screen.queryByRole("dialog")).toBeNull();
    expect(screen.queryByRole("alertdialog")).toBeNull();
    expect(live.refresh).toHaveBeenCalled();
    expect(fetch).toHaveBeenCalledWith(
      "/api/spec-board",
      expect.objectContaining({
        method: "POST",
        body: JSON.stringify({ project: "alpha", spec_id: closed.id, archived: true }),
      }),
    );
    const post = await lastSpecBoardPost();
    expect(post.status).toBe(200);
    expect(post.body.spec_id).toBe(closed.id);
    expect(post.body.archived).toBe(true);
  });

  // Gherkin: Show archived reveals the card and Unarchive restores it
  it("reveals an archived card then Unarchive keeps it visible while hide is on", async () => {
    const live = installLiveBoard([archivedClosed]);
    mockUiFetch(live.store);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    const toggle = screen.getByRole("button", { name: "Show archived" });
    expect(toggle).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Done")).queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();

    fireEvent.click(toggle);
    expect(toggle).toHaveAttribute("aria-pressed", "true");
    expect(titleButton(closed.id)).toBeInTheDocument();

    fireEvent.click(titleButton(closed.id));
    expect(within(cardById(closed.id)).getByRole("button", { name: "Unarchive" })).toBeInTheDocument();
    expect(within(cardById(closed.id)).queryByRole("button", { name: "Archive" })).toBeNull();
    fireEvent.click(screen.getByRole("button", { name: "Unarchive" }));

    await waitFor(() => {
      expect(live.refresh).toHaveBeenCalled();
    });
    expect(fetch).toHaveBeenCalledWith(
      "/api/spec-board",
      expect.objectContaining({
        method: "POST",
        body: JSON.stringify({ project: "alpha", spec_id: closed.id, archived: false }),
      }),
    );
    const post = await lastSpecBoardPost();
    expect(post.status).toBe(200);
    expect(post.body.spec_id).toBe(closed.id);
    expect(post.body.archived).toBe(false);
    expect(within(columnRoot("Done")).getByRole("button", { name: new RegExp(closed.id) })).toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Done")).getByRole("button", { name: new RegExp(closed.id) })).toBeInTheDocument();
  });

  // Gherkin: Limit — poll after archive does not resurrect the card
  it("does not resurrect a hidden archived card when a later GET poll arrives", async () => {
    mockUiFetch();
    mockBoard([closed]);

    const { rerender } = render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(closed.id));
    fireEvent.click(screen.getByRole("button", { name: "Archive" }));

    await waitFor(() => {
      expect(fetch).toHaveBeenCalledWith(
        "/api/spec-board",
        expect.objectContaining({
          method: "POST",
          body: JSON.stringify({ project: "alpha", spec_id: closed.id, archived: true }),
        }),
      );
    });

    mockBoard([{ ...closed, archived: true }]);
    rerender(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Done")).queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();

    mockBoard([{ ...closed, archived: true }]);
    rerender(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expect(screen.getByRole("button", { name: "Show archived" })).toHaveAttribute("aria-pressed", "false");
    expect(within(columnRoot("Done")).queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();
    expect(screen.queryByRole("button", { name: new RegExp(closed.id) })).toBeNull();
  });

  // spec-005 "document has no Archive/Unarchive" replaced: names only on Done expand
  it("shows Archive and Unarchive names only on expanded Done cards", () => {
    mockUiFetch();
    mockBoard([planned, active, closed]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    expectNoArchiveOrUnarchive();

    fireEvent.click(titleButton(planned.id));
    expectNoArchiveOrUnarchive(cardById(planned.id));

    fireEvent.click(titleButton(closed.id));
    expect(within(cardById(closed.id)).getByRole("button", { name: "Archive" })).toBeInTheDocument();
    expect(within(cardById(closed.id)).queryByRole("button", { name: "Unarchive" })).toBeNull();

    cleanup();
    mockBoard([archivedClosed]);
    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(screen.getByRole("button", { name: "Show archived" }));
    fireEvent.click(titleButton(closed.id));
    expect(within(cardById(closed.id)).getByRole("button", { name: "Unarchive" })).toBeInTheDocument();
    expect(within(cardById(closed.id)).queryByRole("button", { name: "Archive" })).toBeNull();
  });
});

describe("SpecBoardTab EpicCard + Todo epics-then-specs", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  // Gherkin: Mixed Todo paints an unconverted epic then a planned spec
  it("paints Mixed Todo as epic then spec with E, title, summary, and plan", () => {
    mockUiFetch();
    mockBoard([planned, active, closed], async () => undefined, {
      grill_skill_present: true,
      epics: [inboxEpic],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const todo = columnRoot("Todo");
    const card = epicCardById(inboxEpic.id);
    const mark = within(card).getByText("E");
    expect(mark).toBeInTheDocument();
    expect(mark.className).toContain("text-[var(--color-epic-mark)]");
    expect(mark.className).not.toMatch(/rounded-full|badge|pill/i);
    expect(within(card).queryByText(/^Epic$/)).toBeNull();
    expect(within(card).getByText("inbox")).toBeInTheDocument();
    expect(within(card).getByText("Filter unread first.")).toBeInTheDocument();
    expect(within(card).getByText("Inbox Plan")).toBeInTheDocument();
    expect(within(todo).getByText(planned.id)).toBeInTheDocument();
    expect(todoCardIds()).toEqual([inboxEpic.id, planned.id]);
    expect(within(todo).getByText("2 pending")).toBeInTheDocument();

    expect(within(columnRoot("In Progress")).queryByText(inboxEpic.id)).toBeNull();
    expect(within(columnRoot("Done")).queryByText(inboxEpic.id)).toBeNull();
    expect(screen.queryAllByText("E")).toHaveLength(1);
  });

  // Gherkin: Limit — epic card does not expand or archive
  it("does not expand or archive when the operator activates an epic card", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [inboxEpic],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    const card = epicCardById(inboxEpic.id);
    fireEvent.click(card);
    fireEvent.click(within(card).getByText("E"));
    fireEvent.click(within(card).getByText("inbox"));
    fireEvent.click(within(card).getByText(inboxEpic.id));

    expectNoArchiveOrUnarchive(card);
    expect(within(card).queryByRole("button", { name: /^Archive$/i })).toBeNull();
    expect(within(card).queryByRole("button", { name: /^Unarchive$/i })).toBeNull();
    expect(within(card).queryByText("No tasks planned yet")).toBeNull();
    expect(within(card).queryByText(messages.en.specBoard.noTasksYet)).toBeNull();
    expect(within(card).queryByRole("button")).toBeNull();
    expect(specBoardPostCalls()).toHaveLength(0);
  });

  it("keeps spec expand and Archive when epics are present", () => {
    mockUiFetch();
    mockBoard([planned, closed], async () => undefined, {
      grill_skill_present: true,
      epics: [inboxEpic],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);
    fireEvent.click(titleButton(planned.id));
    expect(within(cardById(planned.id)).getByText(planned.blurb)).toBeInTheDocument();
    expectNoArchiveOrUnarchive(cardById(planned.id));

    fireEvent.click(titleButton(closed.id));
    expect(within(cardById(closed.id)).getByRole("button", { name: "Archive" })).toBeInTheDocument();
    expect(specBoardPostCalls()).toHaveLength(0);
  });

  // Gherkin: Companion-to omit — UI paints already-filtered epics (no re-match)
  it("does not show an omitted epic id in Todo when the mock already dropped it", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(within(columnRoot("Todo")).queryByText(inboxEpic.id)).toBeNull();
    expect(within(columnRoot("Todo")).getByText(planned.id)).toBeInTheDocument();
    expect(todoCardIds()).toEqual([planned.id]);
  });

  // Gherkin: Done spec claiming an epic still omits it from Todo
  it("shows the Done spec id and never paints the claimed epic id in Done or In Progress", () => {
    mockUiFetch();
    mockBoard([planned, active, closed], async () => undefined, {
      grill_skill_present: true,
      epics: [],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(within(columnRoot("Done")).getByText(closed.id)).toBeInTheDocument();
    expect(within(columnRoot("Done")).queryByText(inboxEpic.id)).toBeNull();
    expect(within(columnRoot("In Progress")).queryByText(inboxEpic.id)).toBeNull();
    expect(columnCardIds("Done")).not.toContain(inboxEpic.id);
    expect(columnCardIds("In Progress")).not.toContain(inboxEpic.id);
    expect(todoCardIds()).toEqual([planned.id]);
  });

  // Gherkin: Limit — two plans follow index.md then epic-NNN then specs
  it("paints Todo card ids in index.md then epic-NNN then spec order", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [planAFirst, planASecond, planBOther],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(todoCardIds()).toEqual([
      ".grill/plans/plan-a/epics/epic-001-first.md",
      ".grill/plans/plan-a/epics/epic-002-second.md",
      ".grill/plans/plan-b/epics/epic-001-other.md",
      "spec-010-aaa-planned",
    ]);
    for (const id of [planAFirst.id, planASecond.id, planBOther.id]) {
      expect(columnCardIds("In Progress")).not.toContain(id);
      expect(columnCardIds("Done")).not.toContain(id);
    }
  });

  // Gherkin: Limit — missing Companion-to keeps the epic beside a similarly named spec
  it("shows both the unconverted epic and a similarly named Todo spec", () => {
    mockUiFetch();
    mockBoard([similarlyNamed], async () => undefined, {
      grill_skill_present: true,
      epics: [inboxEpic],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(within(columnRoot("Todo")).getByText(inboxEpic.id)).toBeInTheDocument();
    expect(within(columnRoot("Todo")).getByText(similarlyNamed.id)).toBeInTheDocument();
    expect(todoCardIds()).toEqual([inboxEpic.id, similarlyNamed.id]);
  });

  // Gherkin: Limit — 65th omitted in mock of 64; no Has more; Function hex locked
  it("paints a 64-epic mock without a Has more control and keeps Function hex", () => {
    mockUiFetch();
    const epics = sixtyFourEpics();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(todoCardIds()).toEqual([...epics.map((epic) => epic.id), planned.id]);
    expect(todoCardIds()).toHaveLength(65);
    expect(screen.queryByRole("button", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByRole("link", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByText("Has more")).toBeNull();
    expect(colorForLabel("Function")).toBe("#06b6d4");
  });

  it("does not copy Show Dones, Track A, or All from Game chrome", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.queryByRole("button", { name: "Show Dones" })).toBeNull();
    expect(screen.queryByRole("button", { name: "Track A" })).toBeNull();
    expect(screen.queryByRole("button", { name: "All" })).toBeNull();
    expect(screen.queryByRole("link", { name: "Show Dones" })).toBeNull();
    expect(screen.queryByRole("link", { name: "Track A" })).toBeNull();
    expect(screen.queryByRole("link", { name: "All" })).toBeNull();
  });
});

describe("SpecBoardTab debt strip + EpicCard wrap", () => {
  const originalClipboard = navigator.clipboard;

  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
    Object.defineProperty(navigator, "clipboard", {
      configurable: true,
      value: originalClipboard,
    });
  });

  // Gherkin: Open heading item appears on GET and in the Specs strip (UI Thens)
  it("paints Open tech debt above the three columns with TD-005 leftover cache", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [inboxEpic],
      debt: [makeDebt({ id: "TD-005", title: "leftover cache" })],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(region).toHaveTextContent("TD-005");
    expect(region).toHaveTextContent("leftover cache");
    expect(within(region).queryByRole("button")).toBeNull();
    expect(within(region).queryByRole("link")).toBeNull();

    const todo = screen.getByRole("heading", { name: /^Todo$/i });
    expect(region.compareDocumentPosition(todo) & Node.DOCUMENT_POSITION_FOLLOWING).toBeGreaterThan(0);
    expect(columnRoot("Todo").contains(region)).toBe(false);
    expect(columnRoot("In Progress").contains(region)).toBe(false);
    expect(columnRoot("Done").contains(region)).toBe(false);
    expect(screen.getByRole("heading", { name: /^Todo$/i })).toBeInTheDocument();
    expect(epicCardById(inboxEpic.id)).toBeInTheDocument();
  });

  // Gherkin: All-resolved TECH_DEBT.md omits the strip (UI)
  it("omits Open tech debt when debt is an empty array", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, { debt: [] });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.getByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
  });

  // Gherkin: Limit — missing TECH_DEBT.md omits the strip (UI: missing key)
  it("omits Open tech debt and does not throw when debt is missing", () => {
    mockUiFetch();
    mockBoard([planned]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.getByText(planned.id)).toBeInTheDocument();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
  });

  // Gherkin: Todo epic id wraps the full path under the short name
  it("wraps the epic id line and keeps title truncate", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [inboxEpic],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const card = epicCardById(inboxEpic.id);
    expect(within(card).getByText(inboxEpic.id)).toBeInTheDocument();
    expect(within(card).getByText("inbox")).toBeInTheDocument();
    const idLine = within(card).getByText(inboxEpic.id);
    expect(idLine.className).not.toMatch(/\btruncate\b/);
    expect(idLine.className).toMatch(/whitespace-normal/);
    expect(idLine.className).toMatch(/break-all/);
    expect(getComputedStyle(idLine).textOverflow).not.toBe("ellipsis");
    const titleLine = within(card).getByText("inbox");
    expect(titleLine.className).toMatch(/\btruncate\b/);
  });

  // Gherkin: Limit — spec card id line still truncates
  it("keeps truncate on the spec card id line", () => {
    mockUiFetch();
    mockBoard([planned]);

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const specId = within(cardById(planned.id)).getByText(planned.id);
    expect(specId.className).toMatch(/\btruncate\b/);
  });

  // Gherkin: Limit — long debt title wraps in the strip
  it("wraps a long debt title in the Open tech debt region", () => {
    mockUiFetch();
    const longTitle = "a very long leftover cache title that exceeds one chrome line";
    mockBoard([planned], async () => undefined, {
      debt: [makeDebt({ id: "TD-005", title: longTitle })],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(region).toHaveTextContent(longTitle);
    const titleLine = within(region).getByText(new RegExp(longTitle));
    expect(titleLine.tagName).toBe("P");
    expect(titleLine.className).toMatch(/whitespace-normal/);
    expect(titleLine.className).toMatch(/break-words/);
    expect(titleLine.className).not.toMatch(/\btruncate\b/);
    expect(getComputedStyle(titleLine).textOverflow).not.toBe("ellipsis");
  });

  // Gherkin: Limit — grill-only without TECH_DEBT.md still shows Specs
  it("shows the Specs pane on grill-only with no debt and omits Open tech debt and notSddSkill", () => {
    mockUiFetch();
    vi.mocked(useSpecBoard).mockReturnValue({
      board: {
        sdd_skill_present: false,
        grill_skill_present: true,
        specs: [],
        epics: [inboxEpic],
        debt: [],
      },
      loading: false,
      error: null,
      refresh: async () => undefined,
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(screen.getByText(messages.en.specBoard.columnTodo)).toBeInTheDocument();
    expect(screen.getByText(inboxEpic.id)).toBeInTheDocument();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
  });

  // Gherkin: Companion-to still omits the converted epic (Todo) — UI mocks already-filtered epics
  it("does not show the converted epic id in Todo when the mock already omitted it", () => {
    mockUiFetch();
    mockBoard([planned], async () => undefined, {
      grill_skill_present: true,
      epics: [],
      debt: [makeDebt({ id: "TD-005", title: "leftover cache" })],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    expect(within(columnRoot("Todo")).queryByText(inboxEpic.id)).toBeNull();
    expect(todoCardIds()).toEqual([planned.id]);
    expect(screen.getByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeInTheDocument();
  });

  // Gherkin: Limit — 17th omitted: no Has more control
  it("paints 16 open debt rows without a Has more control and keeps Function hex", () => {
    mockUiFetch();
    const debt = sixteenOpenDebt();
    mockBoard([planned], async () => undefined, { debt });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    expect(region).toHaveTextContent("TD-001");
    expect(region).toHaveTextContent("TD-016");
    expect(region).not.toHaveTextContent("TD-017");
    expect(screen.queryByRole("button", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByRole("link", { name: /^Has more$/i })).toBeNull();
    expect(screen.queryByText("Has more")).toBeNull();
    expect(colorForLabel("Function")).toBe("#06b6d4");
  });

  // Gherkin: Limit — activating a debt row does nothing
  it("does not POST, copy, or expand when the operator activates a debt row", () => {
    mockUiFetch();
    const writeText = vi.fn(async () => undefined);
    Object.defineProperty(navigator, "clipboard", {
      configurable: true,
      value: { writeText },
    });
    mockBoard([planned], async () => undefined, {
      debt: [makeDebt({ id: "TD-005", title: "leftover cache" })],
    });

    render(<SpecBoardTab project="alpha" onSelectProject={() => undefined} />);

    const region = screen.getByRole("region", { name: messages.en.specBoard.openTechDebt });
    const row = within(region).getByText(/TD-005/);
    expect(row.tagName).toBe("P");
    fireEvent.click(row);
    fireEvent.click(region);

    expect(specBoardPostCalls()).toHaveLength(0);
    expect(writeText).not.toHaveBeenCalled();
    expect(titleButton(planned.id)).toHaveAttribute("aria-expanded", "false");
    expect(document.querySelector("[data-region='blurb']")).toBeNull();
    expect(within(region).queryByRole("button")).toBeNull();
    expect(within(region).queryByRole("link")).toBeNull();
  });
});
