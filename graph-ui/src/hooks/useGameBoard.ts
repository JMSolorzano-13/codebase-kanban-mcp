/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-072 always-emit debt; live GET must keep the field
 * @sdd-why: parseGameBoard is a strict constructor; without parseDebtArray live GET debt never reaches GameBoardTab
 * @human-debug: If strip missing on live GET with debt[] → parseDebtArray dropped the key; if Specs shows Game ids → wrong hook
 */
import { useCallback, useEffect, useRef, useState } from "react";
import type {
  GameBoard,
  GameBoardBlocked,
  GameBoardCard,
  GameBoardDebt,
  GameBoardTask,
  GameBoardTrack,
  GameBoardWorkState,
  GamePhase,
} from "../lib/types";

export interface UseGameBoardResult {
  settled: boolean;
  present: boolean;
  board: GameBoard | null;
  refresh: () => Promise<void>;
}

function isGamePhase(value: unknown): value is GamePhase {
  return (
    value === "01-preproduction" ||
    value === "02-production" ||
    value === "03-postproduction"
  );
}

function isGameBoardTrack(value: unknown): value is GameBoardTrack {
  return value === "A" || value === "B" || value === "H";
}

function isGameBoardWorkState(value: unknown): value is GameBoardWorkState {
  return value === "pending" || value === "in_progress" || value === "done" || value === "blocked";
}

function parseGameBoardTask(raw: unknown): GameBoardTask | null {
  if (typeof raw !== "object" || raw === null) return null;
  const obj = raw as Record<string, unknown>;
  if (typeof obj.number !== "number" || !Number.isFinite(obj.number)) return null;
  return {
    number: obj.number,
    name: typeof obj.name === "string" ? obj.name : "",
    done: obj.done === true,
  };
}

function parseTaskArray(value: unknown): GameBoardTask[] {
  if (!Array.isArray(value)) return [];
  const tasks: GameBoardTask[] = [];
  for (const item of value) {
    const task = parseGameBoardTask(item);
    if (task !== null) tasks.push(task);
  }
  return tasks;
}

function parseBlockedRow(raw: unknown): GameBoardBlocked | null {
  if (typeof raw !== "object" || raw === null) return null;
  const obj = raw as Record<string, unknown>;
  if (typeof obj.owner !== "string") return null;
  return {
    owner: obj.owner,
    task: typeof obj.task === "string" ? obj.task : "",
    blocked_by: typeof obj.blocked_by === "string" ? obj.blocked_by : "",
  };
}

function parseBlockedArray(value: unknown): GameBoardBlocked[] {
  if (!Array.isArray(value)) return [];
  const rows: GameBoardBlocked[] = [];
  for (const item of value) {
    const row = parseBlockedRow(item);
    if (row !== null) rows.push(row);
  }
  return rows;
}

function parseGameBoardCard(raw: unknown): GameBoardCard | null {
  if (typeof raw !== "object" || raw === null) return null;
  const obj = raw as Record<string, unknown>;
  if (obj.kind !== "artifact" && obj.kind !== "epic") return null;
  const blockedBy = obj.blocked_by;
  return {
    kind: obj.kind,
    id: typeof obj.id === "string" ? obj.id : "",
    title: typeof obj.title === "string" ? obj.title : "",
    track: isGameBoardTrack(obj.track) ? obj.track : null,
    work_state: isGameBoardWorkState(obj.work_state) ? obj.work_state : null,
    owner: typeof obj.owner === "string" ? obj.owner : "",
    continue: typeof obj.continue === "string" ? obj.continue : "",
    summary: typeof obj.summary === "string" ? obj.summary : "",
    plan_title: typeof obj.plan_title === "string" ? obj.plan_title : "",
    blurb: typeof obj.blurb === "string" ? obj.blurb : "",
    tasks: parseTaskArray(obj.tasks),
    inputs: typeof obj.inputs === "string" ? obj.inputs : "",
    last_decision: typeof obj.last_decision === "string" ? obj.last_decision : "",
    open: typeof obj.open === "string" ? obj.open : "",
    recent: typeof obj.recent === "string" ? obj.recent : "",
    blocked_by: typeof blockedBy === "string" && blockedBy !== "" ? blockedBy : null,
    archived: obj.archived === true,
  };
}

function parseCardArray(value: unknown): GameBoardCard[] {
  if (!Array.isArray(value)) return [];
  const cards: GameBoardCard[] = [];
  for (const item of value) {
    const card = parseGameBoardCard(item);
    if (card !== null) cards.push(card);
  }
  return cards;
}

function parseDebtArray(value: unknown): GameBoardDebt[] {
  if (!Array.isArray(value)) return [];
  const rows: GameBoardDebt[] = [];
  for (const item of value) {
    if (typeof item !== "object" || item === null) continue;
    const obj = item as Record<string, unknown>;
    if (typeof obj.id !== "string" || obj.id === "") continue;
    rows.push({
      id: obj.id,
      title: typeof obj.title === "string" ? obj.title : "",
    });
  }
  return rows;
}

export function parseGameBoard(data: unknown): GameBoard | null {
  if (typeof data !== "object" || data === null) return null;
  const body = data as {
    gamedev_skill_present?: unknown;
    phase?: unknown;
    focus?: unknown;
    continue?: unknown;
    blocked?: unknown;
    inbox?: unknown;
    preproduction?: unknown;
    production?: unknown;
    postproduction?: unknown;
    debt?: unknown;
  };
  return {
    gamedev_skill_present: body.gamedev_skill_present === true,
    phase: isGamePhase(body.phase) ? body.phase : null,
    focus: typeof body.focus === "string" ? body.focus : null,
    continue: typeof body.continue === "string" ? body.continue : "",
    blocked: parseBlockedArray(body.blocked),
    inbox: parseCardArray(body.inbox),
    preproduction: parseCardArray(body.preproduction),
    production: parseCardArray(body.production),
    postproduction: parseCardArray(body.postproduction),
    debt: parseDebtArray(body.debt),
  };
}

async function requestGameBoard(project: string): Promise<{ ok: true; parsed: GameBoard | null } | { ok: false }> {
  const res = await fetch(`/api/game-board?project=${encodeURIComponent(project)}`);
  if (res.status !== 200) return { ok: false };
  const data: unknown = await res.json();
  return { ok: true, parsed: parseGameBoard(data) };
}

export function useGameBoard(project: string | null): UseGameBoardResult {
  const [settled, setSettled] = useState(false);
  const [present, setPresent] = useState(false);
  const [board, setBoard] = useState<GameBoard | null>(null);
  const projectRef = useRef(project);
  projectRef.current = project;

  const refresh = useCallback(async () => {
    const current = projectRef.current;
    if (!current) return;
    try {
      const result = await requestGameBoard(current);
      if (projectRef.current !== current) return;
      if (!result.ok) return;
      if (result.parsed !== null) {
        setBoard(result.parsed);
        if (result.parsed.gamedev_skill_present === true) {
          setPresent(true);
        }
      }
    } catch {
      /* keep settled/present — silent-win must not unmount Game */
    }
  }, []);

  useEffect(() => {
    if (!project) {
      setSettled(false);
      setPresent(false);
      setBoard(null);
      return;
    }

    let cancelled = false;
    setSettled(false);
    setPresent(false);
    setBoard(null);

    void (async () => {
      try {
        const result = await requestGameBoard(project);
        if (cancelled) return;
        if (!result.ok) {
          setPresent(false);
          setBoard(null);
          setSettled(true);
          return;
        }
        setPresent(result.parsed !== null && result.parsed.gamedev_skill_present === true);
        setBoard(result.parsed);
        setSettled(true);
      } catch {
        if (!cancelled) {
          setPresent(false);
          setBoard(null);
          setSettled(true);
        }
      }
    })();

    return () => {
      cancelled = true;
    };
  }, [project]);

  return { settled, present, board, refresh };
}
