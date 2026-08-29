/**
 * @sdd-task: Task #4 - Dashboard page: list + Control + create-index
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-002 - Stacked Dashboard: list then Control, one ScrollArea
 * @sdd-why: ADR delete lived on StatsTab; isolated so Dashboard never mounts AdrButton
 * @human-debug: If POST body is not empty content → Delete handler did not call save("")
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { AdrButton } from "./AdrButton";

describe("AdrButton", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
  });

  it("posts empty ADR content when deleting an existing ADR", async () => {
    let saved: unknown = null;
    const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
      const url = String(input);
      if (url.startsWith("/api/ui-config")) {
        return new Response(JSON.stringify({ lang: "en" }), {
          status: 200,
          headers: { "Content-Type": "application/json" },
        });
      }
      if (url.startsWith("/api/adr")) {
        if (init?.method === "POST") {
          saved = JSON.parse(String(init.body));
          return new Response(JSON.stringify({ ok: true }), {
            status: 200,
            headers: { "Content-Type": "application/json" },
          });
        }
        return new Response(JSON.stringify({
          has_adr: true,
          content: "old decision text",
          updated_at: "2026-01-01",
        }), { status: 200, headers: { "Content-Type": "application/json" } });
      }
      return new Response("{}", { status: 200 });
    });
    vi.stubGlobal("fetch", fetchMock);

    render(<AdrButton project="demo" />);
    fireEvent.click(await screen.findByRole("button", { name: "ADR" }));
    expect(await screen.findByDisplayValue("old decision text")).toBeInTheDocument();

    fireEvent.click(screen.getByRole("button", { name: "Delete" }));

    await waitFor(() => {
      expect(saved).toEqual({ project: "demo", content: "" });
    });
  });
});
