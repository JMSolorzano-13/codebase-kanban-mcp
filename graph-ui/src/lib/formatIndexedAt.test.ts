/**
 * @sdd-task: Task #3 - formatIndexedAt
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-003 - indexed_at shown as UTC locale via time[dateTime]
 * @sdd-why: Gherkin Two projects — datetime derived from indexed_at (helper)
 * @human-debug: If en output equals the ISO → formatter skipped or parse treated valid ISO as invalid
 */
import { describe, expect, it } from "vitest";
import { formatIndexedAt } from "./formatIndexedAt";

const ISO = "2026-08-29T10:00:00Z";

describe("formatIndexedAt", () => {
  it("derives an en locale string from ISO (year and day visible, not raw ISO)", () => {
    const formatted = formatIndexedAt(ISO, "en");
    expect(formatted.length).toBeGreaterThan(0);
    expect(formatted).toContain("2026");
    expect(formatted).toContain("29");
    expect(formatted).not.toBe(ISO);
  });

  it("derives a zh locale string from the same UTC instant", () => {
    const formatted = formatIndexedAt(ISO, "zh");
    expect(formatted).toContain("2026");
    expect(formatted).toContain("29");
    expect(formatted).not.toBe(ISO);
  });

  it("returns the raw string when input is not a valid datetime", () => {
    expect(formatIndexedAt("not-a-date", "en")).toBe("not-a-date");
    expect(formatIndexedAt("", "zh")).toBe("");
  });
});
