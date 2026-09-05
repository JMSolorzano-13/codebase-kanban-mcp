/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-074 Game strip not in WorkspaceHeader
 * @sdd-why: Header never hosts TD-005 or debt:gate-preproduction
 * @human-debug: If header shows debt:gate → strip leaked into WorkspaceHeader
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { formatIndexedAt } from "../lib/formatIndexedAt";
import { messages } from "../lib/i18n";
import { WorkspaceHeader } from "./WorkspaceHeader";

function rpcToolName(init?: RequestInit): string | undefined {
  if (!init?.body) return undefined;
  const body = JSON.parse(String(init.body)) as { params?: { name?: string } };
  return body.params?.name;
}

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

function mockHeaderFetch(projects: { name: string; root_path: string; indexed_at: string }[]) {
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    if (url === "/rpc") {
      const tool = rpcToolName(init);
      if (tool === "get_graph_schema") {
        throw new Error("workspace header must not call get_graph_schema");
      }
      if (tool === "list_projects") return okRpc({ projects });
      return okRpc({ projects: [] });
    }
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/index-status")) {
      throw new Error("workspace header must not call /api/index-status");
    }
    return json({});
  });
  vi.stubGlobal("fetch", fetchMock);
  return fetchMock;
}

describe("WorkspaceHeader", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("shows listed project name and a time derived from indexed_at", async () => {
    mockHeaderFetch([
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" },
    ]);
    render(<WorkspaceHeader projectName="alpha" onLeave={() => undefined} />);

    const iso = "2026-08-29T10:00:00Z";
    expect(await screen.findByText("alpha")).toBeInTheDocument();
    await waitFor(() => {
      const time = document.querySelector("time");
      expect(time).toBeTruthy();
      expect(time).toHaveAttribute("dateTime", iso);
      expect(time).toHaveAttribute("title", iso);
      expect(time?.textContent).toBe(formatIndexedAt(iso, "en"));
    });
    expect(screen.getByText(messages.en.projects.lastIndexed)).toBeInTheDocument();
  });

  it("shows invalid indexed_at as raw on the header time", async () => {
    mockHeaderFetch([
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "not-a-date" },
    ]);
    render(<WorkspaceHeader projectName="alpha" onLeave={() => undefined} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    await waitFor(() => {
      const time = document.querySelector("time");
      expect(time).toHaveAttribute("dateTime", "not-a-date");
      expect(time?.textContent).toBe("not-a-date");
    });
  });

  it("omits the time element when the name is not in the list", async () => {
    mockHeaderFetch([]);
    render(<WorkspaceHeader projectName="ghost" onLeave={() => undefined} />);

    expect(await screen.findByText("ghost")).toBeInTheDocument();
    await waitFor(() => {
      expect(document.querySelector("time")).toBeNull();
    });
  });

  it("calls onLeave from the back control", async () => {
    const onLeave = vi.fn();
    mockHeaderFetch([]);
    render(<WorkspaceHeader projectName="ghost" onLeave={onLeave} />);

    fireEvent.click(await screen.findByRole("button", { name: messages.en.graph.backToDashboard }));
    expect(onLeave).toHaveBeenCalledTimes(1);
  });

  it("has no Reindex control", async () => {
    mockHeaderFetch([
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" },
    ]);
    render(<WorkspaceHeader projectName="alpha" onLeave={() => undefined} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    expect(screen.queryByRole("button", { name: messages.en.projects.reindex })).not.toBeInTheDocument();
    expect(screen.queryByRole("button", { name: "Reindex" })).not.toBeInTheDocument();
  });

  // Gherkin: Open heading item — WorkspaceHeader does not show TD-005
  it("does not show TD-005 or an Open tech debt region", async () => {
    mockHeaderFetch([
      { name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" },
    ]);
    render(<WorkspaceHeader projectName="alpha" onLeave={() => undefined} />);

    expect(await screen.findByText("alpha")).toBeInTheDocument();
    expect(screen.queryByText("TD-005")).toBeNull();
    expect(screen.queryByText("debt:gate-preproduction")).toBeNull();
    expect(screen.queryByRole("region", { name: messages.en.specBoard.openTechDebt })).toBeNull();
    expect(screen.queryByRole("region", { name: "Open tech debt" })).toBeNull();
  });
});
