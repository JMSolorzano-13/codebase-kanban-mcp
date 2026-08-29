/**
 * @sdd-task: Task #2 - Grayscale chrome tokens + palette lock
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-005 - Chrome grayscale; lock colorForLabel and EdgeLines hex
 * @sdd-why: Gherkin Graph deep link — colorForLabel("Function") hex must stay
 * @human-debug: If Function is not #06b6d4 → colors.ts LABEL_COLORS changed or CSS vars leaked in
 */
import { describe, expect, it } from "vitest";
import { colorForLabel } from "./colors";

const LABEL_COLORS: Record<string, string> = {
  Project: "#e11d48",
  Package: "#f97316",
  Module: "#f97316",
  Folder: "#22c55e",
  File: "#3b82f6",
  Class: "#a855f7",
  Interface: "#a855f7",
  Function: "#06b6d4",
  Method: "#06b6d4",
  Route: "#eab308",
  Variable: "#64748b",
};

describe("colorForLabel palette lock", () => {
  it("keeps Function at #06b6d4 (Gherkin graph deep link)", () => {
    expect(colorForLabel("Function")).toBe("#06b6d4");
  });

  it("locks the rest of LABEL_COLORS", () => {
    for (const [label, hex] of Object.entries(LABEL_COLORS)) {
      expect(colorForLabel(label)).toBe(hex);
    }
  });

  it("falls back to default for unknown labels", () => {
    expect(colorForLabel("__unknown__")).toBe("#94a3b8");
  });
});
