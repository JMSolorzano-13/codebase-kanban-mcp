/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-015 - Create POST is {root_path} only; 409 codes split redirect vs stay
 * @sdd-why: US-001/006 + Gherkin — no Project ID, no project key, path_exists vs name_exists
 * @human-debug: If path_exists calls onCreated → 409 treated as 202; if name_exists closes → code matched path_exists
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { messages } from "../lib/i18n";
import { CreateIndexModal } from "./CreateIndexModal";

function json(body: unknown, status = 200): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json" },
  });
}

function mockModalFetch(options: {
  browsePath?: string;
  onIndex?: (body: unknown) => Response;
} = {}) {
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/browse")) {
      return json({
        path: options.browsePath ?? "/tmp/alpha",
        parent: "/tmp",
        dirs: [],
        roots: ["/"],
      });
    }
    if (url === "/api/index") {
      const body = init?.body ? JSON.parse(String(init.body)) : {};
      if (options.onIndex) return options.onIndex(body);
      return json({ status: "indexing", slot: 0 }, 202);
    }
    return json({});
  });
  vi.stubGlobal("fetch", fetchMock);
  return fetchMock;
}

async function openAndIndex() {
  expect(await screen.findByDisplayValue("/tmp/alpha")).toBeInTheDocument();
  fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));
}

describe("CreateIndexModal 409 + path-only POST", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("has no Project ID field", async () => {
    mockModalFetch();
    render(
      <CreateIndexModal
        onClose={() => undefined}
        onCreated={() => undefined}
        onPathExists={() => undefined}
      />,
    );
    expect(await screen.findByDisplayValue("/tmp/alpha")).toBeInTheDocument();
    expect(screen.queryByLabelText(messages.en.index.projectName)).not.toBeInTheDocument();
    expect(screen.queryByText(messages.en.index.projectName)).not.toBeInTheDocument();
    expect(screen.queryByRole("button", { name: "Reindex" })).not.toBeInTheDocument();
  });

  it("POSTs {root_path} only with no project or project_name", async () => {
    let submitted: unknown = null;
    mockModalFetch({
      onIndex: (body) => {
        submitted = body;
        return json({ status: "indexing", slot: 0 }, 202);
      },
    });
    const onCreated = vi.fn();
    const onPathExists = vi.fn();
    const onClose = vi.fn();
    render(
      <CreateIndexModal onClose={onClose} onCreated={onCreated} onPathExists={onPathExists} />,
    );
    await openAndIndex();

    await waitFor(() => {
      expect(submitted).toEqual({ root_path: "/tmp/alpha" });
    });
    expect(submitted).not.toHaveProperty("project");
    expect(submitted).not.toHaveProperty("project_name");
    expect(onCreated).toHaveBeenCalledTimes(1);
    expect(onPathExists).not.toHaveBeenCalled();
    expect(onClose).toHaveBeenCalledTimes(1);
  });

  it("closes and calls onPathExists on 409 path_exists without onCreated", async () => {
    let submitted: unknown = null;
    mockModalFetch({
      onIndex: (body) => {
        submitted = body;
        return json({
          error: "path already indexed",
          code: "path_exists",
          existing_project: "alpha",
          indexed_at: "2026-08-29T10:00:00Z",
        }, 409);
      },
    });
    const onCreated = vi.fn();
    const onPathExists = vi.fn();
    const onClose = vi.fn();
    render(
      <CreateIndexModal onClose={onClose} onCreated={onCreated} onPathExists={onPathExists} />,
    );
    await openAndIndex();

    await waitFor(() => {
      expect(onPathExists).toHaveBeenCalledWith("alpha");
    });
    expect(submitted).toEqual({ root_path: "/tmp/alpha" });
    expect(onCreated).not.toHaveBeenCalled();
    expect(onClose).toHaveBeenCalledTimes(1);
  });

  it("stays open on 409 name_exists and shows body.error", async () => {
    mockModalFetch({
      browsePath: "/tmp/b/foo",
      onIndex: () => json({
        error: "name already used by foo",
        code: "name_exists",
        existing_project: "foo",
      }, 409),
    });
    const onCreated = vi.fn();
    const onPathExists = vi.fn();
    const onClose = vi.fn();
    render(
      <CreateIndexModal onClose={onClose} onCreated={onCreated} onPathExists={onPathExists} />,
    );
    expect(await screen.findByDisplayValue("/tmp/b/foo")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.index.indexThisFolder }));

    expect(await screen.findByText("name already used by foo")).toBeInTheDocument();
    expect(screen.getByText(messages.en.index.selectRepositoryFolder)).toBeInTheDocument();
    expect(onPathExists).not.toHaveBeenCalled();
    expect(onCreated).not.toHaveBeenCalled();
    expect(onClose).not.toHaveBeenCalled();
  });

  it("skips POST when the list already has that canonical_root", async () => {
    const fetchMock = mockModalFetch({
      onIndex: () => json({ status: "indexing", slot: 0 }, 202),
    });
    const onCreated = vi.fn();
    const onPathExists = vi.fn();
    const onClose = vi.fn();
    render(
      <CreateIndexModal
        onClose={onClose}
        onCreated={onCreated}
        onPathExists={onPathExists}
        existingProjects={[
          {
            name: "alpha",
            root_path: "/tmp/alpha",
            indexed_at: "2026-08-29T10:00:00Z",
            canonical_root: "/tmp/alpha",
          },
        ]}
      />,
    );
    await openAndIndex();

    await waitFor(() => {
      expect(onPathExists).toHaveBeenCalledWith("alpha");
    });
    expect(onCreated).not.toHaveBeenCalled();
    expect(onClose).toHaveBeenCalledTimes(1);
    const indexPosts = fetchMock.mock.calls.filter(([u]) => String(u) === "/api/index");
    expect(indexPosts).toHaveLength(0);
  });
});
