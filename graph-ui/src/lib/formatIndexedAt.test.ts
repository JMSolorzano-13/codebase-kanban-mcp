/**
 * @sdd-task: Task #1 - Drop UTC pin in formatIndexedAt + helper oracles
 * @sdd-spec: specs/spec-007-n6p-last-indexed-local/spec.md
 * @sdd-decision: SDD-ADR-034 - indexed_at visible text uses runtime TZ; dateTime/title stay ISO
 * @sdd-why: Gherkin helper oracles — localFmt vs utcFmt; no hardcoded wall clock; no TZ=
 * @human-debug: If en !== localFmt → helper still pins timeZone; if host-UTC Then fails → skipped the shared-string assert
 */
import { describe, expect, it } from "vitest";
import { formatIndexedAt } from "./formatIndexedAt";

const ISO = "2026-08-29T10:00:00Z";
const INSTANT = new Date(ISO);
const PARTS: Intl.DateTimeFormatOptions = {
  year: "numeric",
  month: "numeric",
  day: "numeric",
  hour: "numeric",
  minute: "numeric",
  timeZoneName: "short",
};

describe("formatIndexedAt", () => {
  it("matches runtime-local Intl and not the UTC pin when those differ", () => {
    const localFmt = new Intl.DateTimeFormat("en-US", PARTS).format(INSTANT);
    const utcFmt = new Intl.DateTimeFormat("en-US", {
      ...PARTS,
      timeZone: "UTC",
    }).format(INSTANT);
    const formatted = formatIndexedAt(ISO, "en");
    expect(formatted).toBe(localFmt);
    expect(formatted).not.toBe(ISO);
    if (localFmt !== utcFmt) {
      expect(formatted).not.toBe(utcFmt);
    }
  });

  it("formats zh locale as the same instant in zh-CN local", () => {
    const zhLocal = new Intl.DateTimeFormat("zh-CN", PARTS).format(INSTANT);
    const formatted = formatIndexedAt(ISO, "zh");
    expect(formatted).toBe(zhLocal);
    expect(formatted).not.toBe(ISO);
  });

  it("returns the raw string when input is not a valid datetime", () => {
    expect(formatIndexedAt("not-a-date", "en")).toBe("not-a-date");
    expect(formatIndexedAt("", "zh")).toBe("");
  });

  it("equals the shared Intl string when runtime timezone is UTC", () => {
    const localFmt = new Intl.DateTimeFormat("en-US", PARTS).format(INSTANT);
    const utcFmt = new Intl.DateTimeFormat("en-US", {
      ...PARTS,
      timeZone: "UTC",
    }).format(INSTANT);
    const formatted = formatIndexedAt(ISO, "en");
    expect(formatted).toBe(localFmt);
    if (localFmt === utcFmt) {
      expect(formatted).toBe(utcFmt);
    }
  });
});
