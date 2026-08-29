/**
 * @sdd-task: Task #2 - Grayscale chrome tokens + palette lock
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-005 - Chrome grayscale; lock colorForLabel and EdgeLines hex
 * @sdd-why: File-level proof chrome is grayscale while graph maps stay teal
 * @human-debug: If @theme has teal primary → globals.css rewrite missed; if CALLS drifted → EdgeLines map edited
 */
import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { describe, expect, it } from "vitest";
import { GRAPH_EDGE_PALETTE } from "../components/EdgeLines";
import { gaugeFillColor } from "../components/ControlTab";

const here = dirname(fileURLToPath(import.meta.url));
const src = join(here, "..");

function readSrc(rel: string): string {
  return readFileSync(join(src, rel), "utf8");
}

function themeHex(block: string, name: string): string {
  const match = block.match(new RegExp(`${name}:\\s*(#[0-9A-Fa-f]{3,8})`));
  if (!match) {
    throw new Error(`missing theme token ${name}`);
  }
  return match[1].toLowerCase();
}

const TEAL_CHROME = new Set(["#1da27e", "#1c8585"]);

describe("globals.css chrome tokens", () => {
  const css = readSrc("styles/globals.css");
  const theme = css.match(/@theme inline \{([\s\S]*?)\n\}/)?.[1] ?? "";

  it("does not set teal primary / accent / ring", () => {
    expect(theme).not.toMatch(/#1DA27E/i);
    expect(theme).not.toMatch(/#1C8585/i);
    expect(TEAL_CHROME.has(themeHex(theme, "--color-primary"))).toBe(false);
    expect(TEAL_CHROME.has(themeHex(theme, "--color-accent"))).toBe(false);
    expect(TEAL_CHROME.has(themeHex(theme, "--color-ring"))).toBe(false);
  });

  it("keeps background / card / hover / border as distinct gray levels", () => {
    const background = themeHex(theme, "--color-background");
    const card = themeHex(theme, "--color-card");
    const hover = themeHex(theme, "--color-hover");
    const border = themeHex(theme, "--color-border");
    const surfaces = [background, card, hover, border];
    expect(new Set(surfaces).size).toBe(4);
  });

  it("leaves graph-loader constellation #22d3ee", () => {
    expect(css).toContain("fill: #22d3ee");
    expect(css).toContain("stroke: #22d3ee");
  });
});

describe("chrome surfaces drop hardcoded teal-black", () => {
  const files = [
    "App.tsx",
    "components/Dashboard.tsx",
    "components/GraphTab.tsx",
    "components/NodeDetailPanel.tsx",
    "components/DisplaySettingsMenu.tsx",
  ];

  it.each(files)("%s uses bg-card and not hardcoded panel hex", (rel) => {
    const source = readSrc(rel);
    expect(source).toMatch(/bg-card/);
    expect(source).not.toMatch(/bg-\[#0b1920\]/);
    expect(source).not.toMatch(/bg-\[#0e2028\]/);
  });
});

describe("GRAPH_EDGE_PALETTE lock", () => {
  it("keeps CALLS and default hex (export only)", () => {
    expect(GRAPH_EDGE_PALETTE.CALLS).toBe("#1DA27E");
    expect(GRAPH_EDGE_PALETTE.DEFAULT_EDGE_COLOR).toBe("#1C8585");
  });
});

describe("gaugeFillColor", () => {
  it("uses gray for healthy fill, not teal", () => {
    expect(gaugeFillColor(10)).toBe("#a3a3a3");
    expect(gaugeFillColor(50)).toBe("#a3a3a3");
    expect(gaugeFillColor(10)).not.toBe("#1DA27E");
  });

  it("keeps >80 red and >50 amber", () => {
    expect(gaugeFillColor(51)).toBe("#eab308");
    expect(gaugeFillColor(80)).toBe("#eab308");
    expect(gaugeFillColor(81)).toBe("#e05252");
  });
});
