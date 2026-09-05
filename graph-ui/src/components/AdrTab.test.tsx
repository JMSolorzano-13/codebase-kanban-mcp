/**
 * @sdd-task: Task #3 - AdrTab generic chrome; no cycle stamp
 * @sdd-spec: specs/spec-013-r9w-adr-fill-gamedev-trio/spec.md
 * @sdd-decision: SDD-ADR-016 - indexed_at from list cache; stamp is chrome not blob
 * @sdd-why: US-006 — generic generated-at + replace warning; document has no gamedev-skill substring
 * @human-debug: If gamedev-skill appears → AdrTab or i18n grew a cycle stamp; if stamp missing → list cache miss
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { cleanup, fireEvent, render, screen, waitFor } from "@testing-library/react";
import { afterEach, describe, expect, it, vi } from "vitest";
import { formatIndexedAt } from "../lib/formatIndexedAt";
import { messages } from "../lib/i18n";
import { ADR_PLACEHOLDER, AdrTab } from "./AdrTab";

const GENERATED_ISO = "2026-08-30T12:00:00Z";
const GENERATED_BLOB =
  "<!-- CBM-GENERATED-START -->\n# Purpose\nPURPOSE-ALPHA\n<!-- CBM-GENERATED-END -->\n<!-- CBM-MANUAL-START -->\n# Existing ADR\n<!-- CBM-MANUAL-END -->\n";

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

function mockAdrFetch(options: {
  getBody?: unknown;
  postStatus?: number;
  onPost?: (body: unknown) => void;
  projects?: { name: string; root_path: string; indexed_at: string }[];
}) {
  const projects = options.projects ?? [];
  const fetchMock = vi.fn(async (input: RequestInfo | URL, init?: RequestInit) => {
    const url = String(input);
    if (url === "/rpc") {
      const tool = rpcToolName(init);
      if (tool === "get_graph_schema") {
        throw new Error("AdrTab must not call get_graph_schema");
      }
      if (tool === "list_projects") return okRpc({ projects });
      return okRpc({ projects: [] });
    }
    if (url.startsWith("/api/ui-config")) {
      return json({ lang: "en" });
    }
    if (url.startsWith("/api/adr")) {
      if (init?.method === "POST") {
        options.onPost?.(JSON.parse(String(init.body)));
        return json({ saved: true }, options.postStatus ?? 200);
      }
      return json(options.getBody ?? { has_adr: false });
    }
    return json({});
  });
  vi.stubGlobal("fetch", fetchMock);
  return fetchMock;
}

function postCalls(fetchMock: ReturnType<typeof vi.fn>) {
  return fetchMock.mock.calls.filter(
    ([input, init]) => String(input).startsWith("/api/adr") && (init as RequestInit | undefined)?.method === "POST",
  );
}

describe("AdrTab", () => {
  afterEach(() => {
    cleanup();
    vi.unstubAllGlobals();
  });

  it("loads an existing ADR blob into the textarea", async () => {
    mockAdrFetch({
      getBody: {
        has_adr: true,
        content: "# Existing ADR\n",
        updated_at: "2026-08-29T09:00:00Z",
      },
    });
    render(<AdrTab project="alpha" />);

    expect(await screen.findByDisplayValue(/# Existing ADR/)).toBeInTheDocument();
    expect(document.body.textContent).not.toContain("CBM-GENERATED");
    expect(document.body.innerHTML).not.toContain("CBM-GENERATED");
    expect(document.querySelector("[class*='fixed'][class*='inset-0']")).toBeNull();
  });

  it("POSTs { project, content } on Save and reports status", async () => {
    let saved: unknown = null;
    mockAdrFetch({
      getBody: { has_adr: true, content: "# Existing ADR\n" },
      onPost: (body) => { saved = body; },
    });
    const onDirtyChange = vi.fn();
    render(<AdrTab project="alpha" onDirtyChange={onDirtyChange} />);

    const textarea = await screen.findByDisplayValue(/# Existing ADR/);
    fireEvent.change(textarea, { target: { value: "# Edited ADR" } });
    await waitFor(() => {
      expect(onDirtyChange).toHaveBeenCalledWith(true);
    });

    fireEvent.click(screen.getByRole("button", { name: messages.en.common.save }));

    await waitFor(() => {
      expect(saved).toEqual({ project: "alpha", content: "# Edited ADR" });
    });
    expect(await screen.findByRole("status")).toHaveTextContent(messages.en.adr.saveSuccess);
    expect(screen.queryByRole("alert")).toBeNull();

    fireEvent.change(screen.getByRole("textbox"), { target: { value: "# Edited ADR more" } });
    await waitFor(() => {
      expect(screen.queryByRole("status")).toBeNull();
    });
  });

  it("shows the empty placeholder and does not POST until Save", async () => {
    const fetchMock = mockAdrFetch({
      getBody: { has_adr: false, content: "" },
    });
    render(<AdrTab project="alpha" />);

    const textarea = await screen.findByRole("textbox");
    await waitFor(() => {
      expect(textarea).toHaveValue("");
    });
    expect(textarea).toHaveAttribute("placeholder", ADR_PLACEHOLDER);
    expect((textarea as HTMLTextAreaElement).placeholder).toContain("Architecture Decision Record");
    expect(postCalls(fetchMock)).toHaveLength(0);

    fireEvent.click(screen.getByRole("button", { name: messages.en.common.save }));
    await waitFor(() => {
      expect(postCalls(fetchMock).length).toBeGreaterThan(0);
    });
  });

  it("keeps the draft and shows an alert when Save returns 500", async () => {
    mockAdrFetch({
      getBody: { has_adr: false },
      postStatus: 500,
    });
    render(<AdrTab project="alpha" />);

    const textarea = await screen.findByRole("textbox");
    fireEvent.change(textarea, { target: { value: "# Draft" } });
    fireEvent.click(screen.getByRole("button", { name: messages.en.common.save }));

    const alert = await screen.findByRole("alert");
    expect(alert.textContent).not.toBe("");
    expect(alert).toHaveTextContent(messages.en.adr.saveError);
    expect(screen.getByRole("textbox")).toHaveValue("# Draft");
    expect(screen.queryByRole("status")).toBeNull();
  });

  it("posts empty ADR content when deleting an existing ADR", async () => {
    let saved: unknown = null;
    mockAdrFetch({
      getBody: {
        has_adr: true,
        content: "old decision text",
        updated_at: "2026-01-01",
      },
      onPost: (body) => { saved = body; },
    });
    render(<AdrTab project="demo" />);

    expect(await screen.findByDisplayValue("old decision text")).toBeInTheDocument();
    fireEvent.click(screen.getByRole("button", { name: messages.en.common.delete }));

    await waitFor(() => {
      expect(saved).toEqual({ project: "demo", content: "" });
    });
  });

  it("shows generated-at from list indexed_at and a replace warning when markers are present", async () => {
    mockAdrFetch({
      getBody: { has_adr: true, content: GENERATED_BLOB },
      projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: GENERATED_ISO }],
    });
    render(<AdrTab project="alpha" />);

    const textarea = await screen.findByRole("textbox");
    expect(textarea).toHaveValue(GENERATED_BLOB);
    expect(screen.getAllByRole("textbox")).toHaveLength(1);

    const time = await screen.findByRole("time");
    expect(time).toHaveAttribute("dateTime", GENERATED_ISO);
    expect(time).toHaveAttribute("title", GENERATED_ISO);
    expect(time.textContent).toBe(formatIndexedAt(GENERATED_ISO, "en"));
    expect(time.textContent).not.toBe(GENERATED_ISO);
    expect((textarea as HTMLTextAreaElement).value).not.toContain(GENERATED_ISO);
    expect(screen.getByText(messages.en.adr.generatedAt)).toBeInTheDocument();
    expect(screen.getByRole("note")).toHaveTextContent(messages.en.adr.replaceWarning);
    expect(document.body.textContent).not.toContain("gamedev-skill");
    expect(document.body.innerHTML).not.toContain("gamedev-skill");
  });

  it("shows invalid indexed_at as raw on the generated stamp", async () => {
    mockAdrFetch({
      getBody: { has_adr: true, content: GENERATED_BLOB },
      projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: "not-a-date" }],
    });
    render(<AdrTab project="alpha" />);

    const textarea = await screen.findByRole("textbox");
    const time = await screen.findByRole("time");
    expect(time).toHaveAttribute("dateTime", "not-a-date");
    expect(time.textContent).toBe("not-a-date");
    expect((textarea as HTMLTextAreaElement).value).toBe(GENERATED_BLOB);
    expect((textarea as HTMLTextAreaElement).value).not.toMatch(/\d{1,2}\/\d{1,2}\/\d{4}/);
  });

  it("omits the generated-at stamp and replace warning when markers are absent", async () => {
    mockAdrFetch({
      getBody: { has_adr: true, content: "# Existing ADR\n" },
      projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: GENERATED_ISO }],
    });
    render(<AdrTab project="alpha" />);

    expect(await screen.findByDisplayValue(/# Existing ADR/)).toBeInTheDocument();
    expect(document.querySelector("time")).toBeNull();
    expect(screen.queryByText(messages.en.adr.generatedAt)).toBeNull();
    expect(screen.queryByRole("note")).toBeNull();
    expect(screen.queryByText(messages.en.adr.replaceWarning)).toBeNull();
  });

  it("POSTs the whole generated blob on Save without writing generated-at into content", async () => {
    let saved: unknown = null;
    mockAdrFetch({
      getBody: { has_adr: true, content: GENERATED_BLOB },
      projects: [{ name: "alpha", root_path: "/tmp/alpha", indexed_at: GENERATED_ISO }],
      onPost: (body) => { saved = body; },
    });
    render(<AdrTab project="alpha" />);

    await screen.findByRole("time");
    fireEvent.click(screen.getByRole("button", { name: messages.en.common.save }));

    await waitFor(() => {
      expect(saved).toEqual({ project: "alpha", content: GENERATED_BLOB });
    });
    expect(GENERATED_BLOB).not.toContain(GENERATED_ISO);
    expect(screen.getByRole("textbox")).toHaveValue(GENERATED_BLOB);
  });
});
