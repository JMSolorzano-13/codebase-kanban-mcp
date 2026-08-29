import { useCallback, useEffect, useState } from "react";
import type { SpecBoard } from "../lib/types";

interface UseSpecBoardResult {
  board: SpecBoard | null;
  loading: boolean;
  error: string | null;
  refresh: () => void;
}

/* Polls /api/spec-board — the active spec's current task/agent can change
 * between agent turns, so this behaves like ControlTab's process poll rather
 * than a one-shot fetch. 4s: frequent enough for a "what's happening now"
 * board, cheap enough for a read that's just a few small file parses. */
const POLL_MS = 4000;

export function useSpecBoard(project: string | null): UseSpecBoardResult {
  const [board, setBoard] = useState<SpecBoard | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchBoard = useCallback(async () => {
    if (!project) {
      setBoard(null);
      setLoading(false);
      setError(null);
      return;
    }
    try {
      const res = await fetch(`/api/spec-board?project=${encodeURIComponent(project)}`);
      const data = (await res.json()) as SpecBoard | { error: string };
      if (!res.ok || "error" in data) {
        throw new Error("error" in data ? data.error : "Failed to fetch spec board");
      }
      setBoard(data);
      setError(null);
    } catch (e) {
      setError(e instanceof Error ? e.message : "Failed to fetch spec board");
    } finally {
      setLoading(false);
    }
  }, [project]);

  useEffect(() => {
    setLoading(true);
    fetchBoard();
    if (!project) return;
    const interval = setInterval(fetchBoard, POLL_MS);
    return () => clearInterval(interval);
  }, [project, fetchBoard]);

  return { board, loading, error, refresh: fetchBoard };
}
