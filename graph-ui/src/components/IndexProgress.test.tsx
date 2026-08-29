/**
 * @sdd-task: Task #4 - Dashboard page: list + Control + create-index
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-002 - Stacked Dashboard: list then Control, one ScrollArea
 * @sdd-why: IndexProgress extract keeps existing poll/done/error contract
 * @human-debug: If onDone fires on [] → empty list treated as success; backend lists done/error
 */
/* @vitest-environment jsdom */
import "@testing-library/jest-dom/vitest";
import { act, fireEvent, render, screen } from "@testing-library/react";
import { afterEach, beforeEach, describe, expect, it, vi } from "vitest";
import { messages } from "../lib/i18n";
import { IndexProgress } from "./IndexProgress";

describe("IndexProgress", () => {
  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    vi.useRealTimers();
    vi.unstubAllGlobals();
    vi.restoreAllMocks();
  });

  it("polls and shows indexing in progress when active", async () => {
    const fetchMock = vi.fn().mockImplementation(() =>
      Promise.resolve({
        json: () => Promise.resolve([
          { slot: 1, status: "indexing", path: "/path/to/project1" },
        ]),
      } as unknown as Response),
    );
    vi.stubGlobal("fetch", fetchMock);

    const onDone = vi.fn();
    render(<IndexProgress onDone={onDone} />);

    await act(async () => {
      await vi.advanceTimersByTimeAsync(0);
    });

    expect(fetchMock).toHaveBeenCalledWith("/api/index-status");
    expect(screen.getByText(messages.en.projects.indexingInProgress)).toBeInTheDocument();
    expect(screen.getByText("/path/to/project1")).toBeInTheDocument();
    expect(onDone).not.toHaveBeenCalled();
  });

  it("stops polling and calls onDone when indexing finishes successfully", async () => {
    let mockData = [
      { slot: 1, status: "indexing", path: "/path/to/project" },
    ];
    const fetchMock = vi.fn().mockImplementation(() =>
      Promise.resolve({
        json: () => Promise.resolve(mockData),
      } as unknown as Response),
    );
    vi.stubGlobal("fetch", fetchMock);

    const onDone = vi.fn();
    render(<IndexProgress onDone={onDone} />);

    await act(async () => {
      await vi.advanceTimersByTimeAsync(0);
    });
    expect(onDone).not.toHaveBeenCalled();

    mockData = [
      { slot: 1, status: "done", path: "/path/to/project" },
    ];

    await act(async () => {
      await vi.advanceTimersByTimeAsync(2000);
    });

    expect(onDone).toHaveBeenCalled();
  });

  it("keeps waiting and does NOT call onDone while the jobs list is empty", async () => {
    let mockData: { slot: number; status: string; path: string }[] = [];
    const fetchMock = vi.fn().mockImplementation(() =>
      Promise.resolve({
        json: () => Promise.resolve(mockData),
      } as unknown as Response),
    );
    vi.stubGlobal("fetch", fetchMock);

    const onDone = vi.fn();
    render(<IndexProgress onDone={onDone} />);

    await act(async () => {
      await vi.advanceTimersByTimeAsync(0);
    });
    await act(async () => {
      await vi.advanceTimersByTimeAsync(2000);
    });
    await act(async () => {
      await vi.advanceTimersByTimeAsync(2000);
    });
    expect(onDone).not.toHaveBeenCalled();
    expect(fetchMock).toHaveBeenCalledTimes(3);

    mockData = [{ slot: 1, status: "done", path: "/path/to/project" }];
    await act(async () => {
      await vi.advanceTimersByTimeAsync(2000);
    });
    expect(onDone).toHaveBeenCalled();
  });

  it("renders error banner and does NOT call onDone when indexing fails with error status", async () => {
    const fetchMock = vi.fn().mockImplementation(() =>
      Promise.resolve({
        json: () => Promise.resolve([
          { slot: 1, status: "error", path: "/path/to/failed-project", error: "OOM Error" },
        ]),
      } as unknown as Response),
    );
    vi.stubGlobal("fetch", fetchMock);

    const onDone = vi.fn();
    render(<IndexProgress onDone={onDone} />);

    await act(async () => {
      await vi.advanceTimersByTimeAsync(0);
    });

    expect(screen.getByText(messages.en.projects.indexingFailed)).toBeInTheDocument();
    expect(screen.getByText("/path/to/failed-project")).toBeInTheDocument();
    expect(screen.getByText("OOM Error")).toBeInTheDocument();
    expect(onDone).not.toHaveBeenCalled();

    const dismissBtn = screen.getByRole("button", { name: messages.en.common.dismiss });
    expect(dismissBtn).toBeInTheDocument();

    await act(async () => {
      fireEvent.click(dismissBtn);
    });

    expect(onDone).toHaveBeenCalled();
  });
});
