/**
 * @sdd-task: Task #1 - useProjects list-only
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-006 - useProjects is list_projects only
 * @sdd-why: Gherkin spy: zero get_graph_schema; indexed_at from list_projects
 * @human-debug: If spy sees get_graph_schema → hook or a consumer still calls callTool with that name
 */
import { renderHook, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { useProjects } from "./useProjects";

function rpcToolName(init?: RequestInit): string | undefined {
  if (!init?.body) return undefined;
  const body = JSON.parse(String(init.body)) as {
    params?: { name?: string };
  };
  return body.params?.name;
}

function rpcToolNames(fetchMock: ReturnType<typeof vi.fn>): string[] {
  return fetchMock.mock.calls
    .filter((call) => String(call[0]) === "/rpc")
    .map((call) => rpcToolName(call[1] as RequestInit | undefined))
    .filter((name): name is string => typeof name === "string");
}

function listProjectsPayload() {
  return {
    projects: [
      {
        name: "alpha",
        root_path: "/tmp/alpha",
        indexed_at: "2026-08-29T10:00:00Z",
      },
      {
        name: "beta",
        root_path: "/tmp/beta",
        indexed_at: "2026-08-29T11:30:00Z",
      },
    ],
  };
}

function okRpc(payload: unknown): Response {
  return new Response(
    JSON.stringify({
      result: { content: [{ text: JSON.stringify(payload) }] },
    }),
    { status: 200, headers: { "Content-Type": "application/json" } },
  );
}

describe("useProjects", () => {
  afterEach(() => {
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("hydrates indexed_at from list_projects and never calls get_graph_schema", async () => {
    const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
      const url = String(input);
      if (url === "/rpc") {
        const tool = rpcToolName(init);
        if (tool === "list_projects") return okRpc(listProjectsPayload());
        throw new Error(`unexpected RPC tool: ${tool ?? "unknown"}`);
      }
      return new Response("{}", { status: 200 });
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useProjects());

    await waitFor(() => {
      expect(result.current.loading).toBe(false);
    });

    expect(result.current.error).toBeNull();
    expect(result.current.projects).toEqual(listProjectsPayload().projects);
    expect(result.current.projects[0]?.indexed_at).toBe("2026-08-29T10:00:00Z");
    expect(result.current.projects[1]?.indexed_at).toBe("2026-08-29T11:30:00Z");

    const tools = rpcToolNames(fetchMock);
    expect(tools.filter((name) => name === "get_graph_schema")).toHaveLength(0);
    expect(tools.filter((name) => name === "list_projects").length).toBeGreaterThan(0);

    result.current.refresh();
    await waitFor(() => {
      expect(rpcToolNames(fetchMock).filter((name) => name === "list_projects").length).toBeGreaterThan(1);
    });
    expect(rpcToolNames(fetchMock).filter((name) => name === "get_graph_schema")).toHaveLength(0);
  });

  it("sets a non-empty error when list_projects RPC fails", async () => {
    const fetchMock = vi.fn(async (input: RequestInfo | URL) => {
      if (String(input) === "/rpc") {
        return new Response("fail", { status: 500, statusText: "Internal Server Error" });
      }
      return new Response("{}", { status: 200 });
    });
    vi.stubGlobal("fetch", fetchMock);

    const { result } = renderHook(() => useProjects());

    await waitFor(() => {
      expect(result.current.loading).toBe(false);
    });

    expect(result.current.error).toBeTruthy();
    expect(result.current.error?.length).toBeGreaterThan(0);
    expect(result.current.projects).toEqual([]);
    expect(rpcToolNames(fetchMock).filter((name) => name === "get_graph_schema")).toHaveLength(0);
  });
});
