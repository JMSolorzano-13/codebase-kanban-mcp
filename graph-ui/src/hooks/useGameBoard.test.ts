/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 parseGameBoard keeps debt; missing → []
 * @sdd-why: live GET debt must survive the constructor; old mocks without debt stay []
 * @human-debug: If parse drops debt → parseDebtArray not wired; if missing debt throws → field not optional
 */
/* @vitest-environment jsdom */
import { renderHook, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { parseGameBoard, useGameBoard } from "./useGameBoard";

function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function emptyBoard(present: boolean): Record<string, unknown> {
  return {
    gamedev_skill_present: present,
    phase: null,
    focus: null,
    continue: present ? "/gamedev-skill continue" : "",
    inbox: [],
    preproduction: [],
    production: [],
    postproduction: [],
  };
}

describe("useGameBoard", () => {
  afterEach(() => {
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("one-shot GET /api/game-board sets present true on 200 when gamedev_skill_present === true", async () => {
    const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
      expect(String(input)).toBe("/api/game-board?project=bevy");
      expect(String(input)).not.toContain("/api/skill-presence");
      expect((init?.method ?? "GET").toUpperCase()).toBe("GET");
      return json(emptyBoard(true));
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useGameBoard("bevy"));
    expect(result.current.settled).toBe(false);
    expect(result.current.present).toBe(false);

    await waitFor(() => {
      expect(result.current.settled).toBe(true);
    });
    expect(result.current.present).toBe(true);
    expect(result.current.board?.gamedev_skill_present).toBe(true);
    expect(fetchMock).toHaveBeenCalledTimes(1);
  });

  it("sets present false on HTTP 200 when gamedev_skill_present === false", async () => {
    vi.stubGlobal("fetch", vi.fn(async () => json(emptyBoard(false))));

    const { result } = renderHook(() => useGameBoard("alpha"));
    await waitFor(() => {
      expect(result.current.settled).toBe(true);
    });
    expect(result.current.present).toBe(false);
  });

  it("keeps settled false while the game-board request hangs", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(() => new Promise<Response>(() => undefined)),
    );

    const { result } = renderHook(() => useGameBoard("bevy"));
    await Promise.resolve();
    expect(result.current.settled).toBe(false);
    expect(result.current.present).toBe(false);
  });

  it("settles true with present false on HTTP 500", async () => {
    vi.stubGlobal("fetch", vi.fn(async () => json({ error: "board failed" }, 500)));

    const { result } = renderHook(() => useGameBoard("alpha"));
    await waitFor(() => {
      expect(result.current.settled).toBe(true);
    });
    expect(result.current.present).toBe(false);
    expect(result.current.board).toBeNull();
  });

  it("settles true with present false on HTTP 4xx and throw", async () => {
    for (const status of [400, 404]) {
      vi.stubGlobal("fetch", vi.fn(async () => json({ error: "fail" }, status)));
      const { result, unmount } = renderHook(() => useGameBoard("alpha"));
      await waitFor(() => {
        expect(result.current.settled).toBe(true);
      });
      expect(result.current.present).toBe(false);
      unmount();
    }

    vi.stubGlobal(
      "fetch",
      vi.fn(async () => {
        throw new TypeError("Failed to fetch");
      }),
    );
    const { result } = renderHook(() => useGameBoard("alpha"));
    await waitFor(() => {
      expect(result.current.settled).toBe(true);
    });
    expect(result.current.present).toBe(false);
  });

  it("does not start a 4s poll and never calls /api/skill-presence", async () => {
    const interval = vi.spyOn(globalThis, "setInterval");
    const fetchMock = vi.fn(async (input: RequestInfo | URL) => {
      expect(String(input)).not.toContain("/api/skill-presence");
      return json(emptyBoard(true));
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useGameBoard("bevy"));
    await waitFor(() => {
      expect(result.current.present).toBe(true);
    });
    expect(interval.mock.calls.filter(([, ms]) => ms === 4000)).toHaveLength(0);
    expect(fetchMock).toHaveBeenCalledTimes(1);
  });

  it("parseGameBoard types cards and skips objects without kind artifact|epic", () => {
    const parsed = parseGameBoard({
      gamedev_skill_present: true,
      phase: "02-production",
      focus: "Ship feel",
      continue: "/gamedev-skill continue",
      inbox: [
        {
          kind: "epic",
          id: ".grill/plans/inbox-plan/epics/epic-001-inbox.md",
          title: "inbox",
          track: null,
          work_state: null,
          owner: "",
          continue: "/gamedev-skill continue",
          summary: "Filter unread first.",
          plan_title: "Inbox Plan",
        },
        { kind: "spec", id: "skip-spec", title: "nope" },
        { id: "no-kind", title: "nope" },
      ],
      preproduction: [
        { foo: 1 },
        {
          kind: "artifact",
          id: ".gamedev/phases/01-preproduction/gdd.md",
          title: "gdd.md",
          track: "B",
          work_state: "pending",
          owner: "@game-designer",
          continue: "/gamedev-skill continue @game-designer",
          summary: "",
          plan_title: "",
        },
      ],
      production: "not-an-array",
      postproduction: [{ kind: "artifact" }],
    });

    expect(parsed).not.toBeNull();
    expect(parsed?.inbox).toHaveLength(1);
    expect(parsed?.inbox[0]?.kind).toBe("epic");
    expect(parsed?.inbox[0]?.continue).toBe("/gamedev-skill continue");
    expect(parsed?.preproduction).toHaveLength(1);
    expect(parsed?.preproduction[0]?.kind).toBe("artifact");
    expect(parsed?.preproduction[0]?.id).toBe(".gamedev/phases/01-preproduction/gdd.md");
    expect(parsed?.production).toEqual([]);
    expect(parsed?.postproduction).toHaveLength(1);
    expect(parsed?.postproduction[0]?.kind).toBe("artifact");
    expect(parsed?.postproduction[0]?.id).toBe("");
    expect(parsed?.blocked).toEqual([]);
    expect(parsed?.debt).toEqual([]);
    expect(parsed?.preproduction[0]?.archived).toBe(false);
    expect(parsed?.preproduction[0]?.blocked_by).toBeNull();
    expect(parsed?.preproduction[0]?.tasks).toEqual([]);
    expect(parsed?.preproduction[0]?.blurb).toBe("");
    expect(parsed?.inbox[0]?.archived).toBe(false);
  });

  it("parseGameBoard fills additive fields and defaults missing archived/blocked", () => {
    const parsed = parseGameBoard({
      gamedev_skill_present: true,
      blocked: [
        {
          owner: "@gameplay-engineer",
          task: "Combat system v2",
          blocked_by: "Waiting on final boss design",
        },
        { skip: true },
      ],
      production: [
        {
          kind: "artifact",
          id: ".gamedev/phases/02-production/systems/SYS-001-movement",
          title: "SYS-001-movement",
          track: "A",
          work_state: "blocked",
          blurb: "Moves the tetromino left and right. DAS applies after the first tap.",
          inputs: "Grid occupancy from collision. Outputs a new piece position.",
          tasks: [
            { number: 1, name: "Parse input", done: true },
            { number: 2, name: "Apply DAS", done: false },
            { name: "no-number" },
          ],
          last_decision: "",
          open: "",
          recent: "",
          blocked_by: "Waiting on final boss design",
          archived: true,
        },
      ],
    });

    expect(parsed?.blocked).toEqual([
      {
        owner: "@gameplay-engineer",
        task: "Combat system v2",
        blocked_by: "Waiting on final boss design",
      },
    ]);
    expect(parsed?.production[0]?.blurb).toBe(
      "Moves the tetromino left and right. DAS applies after the first tap.",
    );
    expect(parsed?.production[0]?.inputs).toBe(
      "Grid occupancy from collision. Outputs a new piece position.",
    );
    expect(parsed?.production[0]?.tasks).toEqual([
      { number: 1, name: "Parse input", done: true },
      { number: 2, name: "Apply DAS", done: false },
    ]);
    expect(parsed?.production[0]?.blocked_by).toBe("Waiting on final boss design");
    expect(parsed?.production[0]?.archived).toBe(true);
    expect(parsed?.production[0]?.work_state).toBe("blocked");
    expect(parsed?.debt).toEqual([]);
  });

  it("parseGameBoard keeps open debt rows and defaults missing debt to []", () => {
    const parsed = parseGameBoard({
      gamedev_skill_present: true,
      debt: [
        { id: "debt:gate-preproduction", title: "missing GDD lock" },
        { skip: true },
        { id: "", title: "nope" },
      ],
    });
    expect(parsed?.debt).toEqual([{ id: "debt:gate-preproduction", title: "missing GDD lock" }]);
  });

  it("hook parse skips junk cards and stays one-shot", async () => {
    const interval = vi.spyOn(globalThis, "setInterval");
    const fetchMock = vi.fn(async () =>
      json({
        ...emptyBoard(true),
        preproduction: [
          { kind: "artifact", id: "keep", title: "gdd.md", track: "B", work_state: "pending", owner: "", continue: "", summary: "", plan_title: "" },
          { kind: "column", id: "drop" },
        ],
      }),
    );
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useGameBoard("bevy"));
    await waitFor(() => {
      expect(result.current.settled).toBe(true);
    });
    expect(result.current.board?.preproduction).toHaveLength(1);
    expect(result.current.board?.preproduction[0]?.kind).toBe("artifact");
    expect(interval.mock.calls.filter(([, ms]) => ms === 4000)).toHaveLength(0);
    expect(fetchMock).toHaveBeenCalledTimes(1);
  });

  it("refresh re-GETs without unsetting settled or present", async () => {
    let archived = false;
    const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
      expect(String(input)).toBe("/api/game-board?project=bevy");
      expect((init?.method ?? "GET").toUpperCase()).toBe("GET");
      return json({
        ...emptyBoard(true),
        preproduction: [
          {
            kind: "artifact",
            id: ".gamedev/phases/01-preproduction/audio-direction.md",
            title: "audio-direction.md",
            track: "B",
            work_state: "done",
            archived,
          },
        ],
      });
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useGameBoard("bevy"));
    await waitFor(() => {
      expect(result.current.settled).toBe(true);
    });
    expect(result.current.present).toBe(true);
    expect(result.current.board?.preproduction[0]?.archived).toBe(false);
    expect(fetchMock).toHaveBeenCalledTimes(1);

    archived = true;
    await result.current.refresh();
    await waitFor(() => {
      expect(result.current.board?.preproduction[0]?.archived).toBe(true);
    });
    expect(result.current.settled).toBe(true);
    expect(result.current.present).toBe(true);
    expect(fetchMock).toHaveBeenCalledTimes(2);
  });

  it("refresh 500 keeps settled and present true", async () => {
    let calls = 0;
    const fetchMock = vi.fn(async () => {
      calls += 1;
      if (calls === 1) return json(emptyBoard(true));
      return json({ error: "board failed" }, 500);
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useGameBoard("bevy"));
    await waitFor(() => {
      expect(result.current.present).toBe(true);
    });
    await result.current.refresh();
    await Promise.resolve();
    expect(result.current.settled).toBe(true);
    expect(result.current.present).toBe(true);
    expect(result.current.board?.gamedev_skill_present).toBe(true);
  });
});
