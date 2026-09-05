/**
 * @sdd-task: Task #2 - TabId, route kernels, strip, i18n
 * @sdd-spec: specs/spec-010-c4h-game-tab-silent-win/spec.md
 * @sdd-decision: SDD-ADR-043
 * @sdd-why: closed set +game; dual fallback; strip showGame
 * @human-debug: If tab=game ignored → WORKSPACE_TABS / isWorkspaceTab
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { messages } from "../lib/i18n";
import { WorkspaceTabStrip } from "./WorkspaceTabStrip";

function mockUiFetch() {
  vi.stubGlobal(
    "fetch",
    vi.fn(async (input: RequestInfo | URL) => {
      if (String(input).startsWith("/api/ui-config")) {
        return new Response(JSON.stringify({ lang: "en" }), {
          status: 200,
          headers: { "Content-Type": "application/json" },
        });
      }
      return new Response("{}", { status: 200 });
    }),
  );
}

function tabNames(): string[] {
  return screen.getAllByRole("tab").map((el) => el.textContent ?? "");
}

describe("WorkspaceTabStrip", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("renders Graph | Specs | ADR when showSpecs is true", () => {
    mockUiFetch();
    render(
      <WorkspaceTabStrip selected="graph" showSpecs onSelect={() => undefined} />,
    );

    expect(screen.getByRole("tablist")).toBeInTheDocument();
    expect(tabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.specs,
      messages.en.tabs.adr,
    ]);
    expect(screen.getByRole("tablist").closest("header")).toBeNull();
    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
    for (const tab of screen.getAllByRole("tab")) {
      expect(tab).not.toBeDisabled();
    }
  });

  it("omits Specs (does not disable it) when showSpecs is false", () => {
    mockUiFetch();
    render(
      <WorkspaceTabStrip selected="graph" showSpecs={false} onSelect={() => undefined} />,
    );

    expect(tabNames()).toEqual([messages.en.tabs.graph, messages.en.tabs.adr]);
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).toBeNull();
    expect(screen.queryByText(messages.en.specBoard.notSddSkill)).toBeNull();
  });

  it("renders Graph | Game | ADR when showGame is true", () => {
    mockUiFetch();
    render(
      <WorkspaceTabStrip
        selected="graph"
        showSpecs={false}
        showGame
        onSelect={() => undefined}
      />,
    );

    expect(tabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.game,
      messages.en.tabs.adr,
    ]);
    expect(screen.getByRole("tab", { name: "Game" })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).toBeNull();
    for (const tab of screen.getAllByRole("tab")) {
      expect(tab).not.toBeDisabled();
    }
  });

  it("omits Game (does not disable it) when showGame is false", () => {
    mockUiFetch();
    render(
      <WorkspaceTabStrip
        selected="graph"
        showSpecs
        showGame={false}
        onSelect={() => undefined}
      />,
    );

    expect(tabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.specs,
      messages.en.tabs.adr,
    ]);
    expect(screen.queryByRole("tab", { name: "Game" })).toBeNull();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.game })).toBeNull();
    for (const tab of screen.getAllByRole("tab")) {
      expect(tab).not.toBeDisabled();
    }
  });

  it("shows Game and omits Specs when both flags are true", () => {
    mockUiFetch();
    render(
      <WorkspaceTabStrip selected="graph" showSpecs showGame onSelect={() => undefined} />,
    );

    expect(tabNames()).toEqual([
      messages.en.tabs.graph,
      messages.en.tabs.game,
      messages.en.tabs.adr,
    ]);
    expect(screen.getByRole("tab", { name: "Game" })).toBeInTheDocument();
    expect(screen.queryByRole("tab", { name: messages.en.tabs.specs })).toBeNull();
  });

  it("marks the selected tab with aria-selected and aria-current=page", () => {
    mockUiFetch();
    render(
      <WorkspaceTabStrip selected="adr" showSpecs onSelect={() => undefined} />,
    );

    const graph = screen.getByRole("tab", { name: messages.en.tabs.graph });
    const specs = screen.getByRole("tab", { name: messages.en.tabs.specs });
    const adr = screen.getByRole("tab", { name: messages.en.tabs.adr });

    expect(adr).toHaveAttribute("aria-selected", "true");
    expect(adr).toHaveAttribute("aria-current", "page");
    expect(graph).toHaveAttribute("aria-selected", "false");
    expect(graph).not.toHaveAttribute("aria-current");
    expect(specs).toHaveAttribute("aria-selected", "false");
    expect(specs).not.toHaveAttribute("aria-current");
  });

  it("notifies onSelect with the workspace tab id", () => {
    mockUiFetch();
    const onSelect = vi.fn();
    render(<WorkspaceTabStrip selected="graph" showSpecs onSelect={onSelect} />);

    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.specs }));
    expect(onSelect).toHaveBeenCalledWith("specs");
    fireEvent.click(screen.getByRole("tab", { name: messages.en.tabs.adr }));
    expect(onSelect).toHaveBeenCalledWith("adr");
  });

  it("notifies onSelect with game when the Game tab is activated", () => {
    mockUiFetch();
    const onSelect = vi.fn();
    render(
      <WorkspaceTabStrip selected="graph" showSpecs={false} showGame onSelect={onSelect} />,
    );

    fireEvent.click(screen.getByRole("tab", { name: "Game" }));
    expect(onSelect).toHaveBeenCalledWith("game");
  });
});
