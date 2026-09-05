/**
 * @sdd-task: Task #1 - Presence predicate sdd OR grill
 * @sdd-spec: specs/spec-009-t4x-specs-tab-grill-presence/spec.md
 * @sdd-decision: SDD-ADR-039 - Keep useSddSkillPresent; present is 200 and sdd OR grill
 * @sdd-why: Invert sdd-only Then; grill-only / sdd-only / both / neither / missing grill
 * @human-debug: If grill-only stays false → bodyHasSkill still sdd-only; if missing grill is true → missing key treated as truthy
 */
/* @vitest-environment jsdom */
import { renderHook, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { useSddSkillPresent } from "./useSddSkillPresent";

function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function specBoardBody(flags: {
  sdd_skill_present?: unknown;
  grill_skill_present?: unknown;
  omitGrill?: boolean;
}): Record<string, unknown> {
  const body: Record<string, unknown> = {
    sdd_skill_present: flags.sdd_skill_present,
    specs: [] as const,
  };
  if (!flags.omitGrill) {
    body.grill_skill_present = flags.grill_skill_present;
  }
  return body;
}

describe("useSddSkillPresent", () => {
  afterEach(() => {
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("sets present true on HTTP 200 when grill_skill_present === true and sdd is false", async () => {
    const fetchMock = vi.fn(async (input: RequestInfo | URL) => {
      expect(String(input)).toBe("/api/spec-board?project=alpha");
      expect(String(input)).not.toContain("skill-presence");
      return json(specBoardBody({ sdd_skill_present: false, grill_skill_present: true }));
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    expect(result.current.present).toBe(false);

    await waitFor(() => {
      expect(result.current.present).toBe(true);
    });
    expect(fetchMock).toHaveBeenCalledTimes(1);
  });

  it("sets present true on HTTP 200 when sdd_skill_present === true and grill is false", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(async () =>
        json(specBoardBody({ sdd_skill_present: true, grill_skill_present: false })),
      ),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(result.current.present).toBe(true);
    });
  });

  it("sets present true on HTTP 200 when both sdd and grill are true", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(async () =>
        json(specBoardBody({ sdd_skill_present: true, grill_skill_present: true })),
      ),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(result.current.present).toBe(true);
    });
  });

  it("keeps present false on HTTP 200 when both sdd and grill are false", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(async () =>
        json(specBoardBody({ sdd_skill_present: false, grill_skill_present: false })),
      ),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(fetch).toHaveBeenCalled();
    });
    expect(result.current.present).toBe(false);
  });

  it("keeps present false on HTTP 200 when sdd is false and grill_skill_present is missing", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(async () => json(specBoardBody({ sdd_skill_present: false, omitGrill: true }))),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(fetch).toHaveBeenCalled();
    });
    expect(result.current.present).toBe(false);
  });

  it("keeps present false when skill flags are truthy but not boolean true", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(async () =>
        json(specBoardBody({ sdd_skill_present: 1, grill_skill_present: "true" })),
      ),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(fetch).toHaveBeenCalled();
    });
    expect(result.current.present).toBe(false);
  });

  it("omits Specs while the spec-board request is still in flight", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(() => new Promise<Response>(() => undefined)),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await Promise.resolve();
    expect(result.current.present).toBe(false);
  });

  it("keeps present false on HTTP 4xx and 5xx", async () => {
    for (const status of [400, 404, 500]) {
      vi.stubGlobal("fetch", vi.fn(async () => json({ error: "fail" }, status)));
      const { result, unmount } = renderHook(() => useSddSkillPresent("alpha"));
      await waitFor(() => {
        expect(fetch).toHaveBeenCalled();
      });
      expect(result.current.present).toBe(false);
      unmount();
    }
  });

  it("keeps present false on network failure", async () => {
    vi.stubGlobal(
      "fetch",
      vi.fn(async () => {
        throw new TypeError("Failed to fetch");
      }),
    );

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(fetch).toHaveBeenCalled();
    });
    expect(result.current.present).toBe(false);
  });

  it("does not start a 4s poll and never calls /api/skill-presence", async () => {
    const interval = vi.spyOn(globalThis, "setInterval");
    const fetchMock = vi.fn(async (input: RequestInfo | URL) => {
      expect(String(input)).not.toContain("/api/skill-presence");
      return json(specBoardBody({ sdd_skill_present: true, grill_skill_present: false }));
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useSddSkillPresent("alpha"));
    await waitFor(() => {
      expect(result.current.present).toBe(true);
    });
    /* waitFor itself uses a 50ms interval; the strip must not use useSpecBoard's 4000ms poll. */
    expect(interval.mock.calls.filter(([, ms]) => ms === 4000)).toHaveLength(0);
    expect(fetchMock).toHaveBeenCalledTimes(1);
  });
});
