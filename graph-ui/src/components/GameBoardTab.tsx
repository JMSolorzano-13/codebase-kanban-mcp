/**
 * @sdd-task: Task #3 - GameBoardTab strip + leftover Vitest
 * @sdd-spec: specs/spec-017-b4w-game-debt-chrome/spec.md
 * @sdd-decision: SDD-ADR-074 Game-only strip after BlockedStrip; reuse specBoard.openTechDebt
 * @sdd-why: Open debt:* chrome after Blocked; dead text; wrap titles; filters do not hide rows
 * @human-debug: If strip missing with debt[] → ?? [] or length gate; if strip on Graph → painted outside GameBoardTab; if title ellipsis → truncate leaked onto debt p
 */
import type { MouseEvent } from "react";
import { useRef, useState } from "react";
import { useUiMessages } from "../lib/i18n";
import type { GameBoard, GameBoardCard, GameBoardDebt, GamePhase } from "../lib/types";

export interface GameBoardTabProps {
  board: GameBoard;
  project?: string | null;
  refresh?: () => Promise<void>;
}

type ColumnKey = "inbox" | "preproduction" | "production" | "postproduction";
type GameTrackFilter = "A" | "B" | "all";

const COLUMN_KEYS: ColumnKey[] = ["inbox", "preproduction", "production", "postproduction"];

function chromeFilterClass(pressed: boolean): string {
  return `text-[10px] ${pressed ? "text-foreground/70" : "text-foreground/35"} hover:text-foreground/70`;
}

function phaseLabel(phase: GamePhase | null, t: ReturnType<typeof useUiMessages>): string | null {
  if (phase === "01-preproduction") return t.gameBoard.phasePreproduction;
  if (phase === "02-production") return t.gameBoard.phaseProduction;
  if (phase === "03-postproduction") return t.gameBoard.phasePostproduction;
  return null;
}

function columnTitle(key: ColumnKey, t: ReturnType<typeof useUiMessages>): string {
  if (key === "inbox") return t.gameBoard.columnInbox;
  if (key === "preproduction") return t.gameBoard.phasePreproduction;
  if (key === "production") return t.gameBoard.phaseProduction;
  return t.gameBoard.phasePostproduction;
}

function columnIsCurrent(key: ColumnKey, phase: GamePhase | null): boolean {
  if (key === "inbox" || phase === null) return false;
  if (phase === "01-preproduction") return key === "preproduction";
  if (phase === "02-production") return key === "production";
  return key === "postproduction";
}

function workStateLabel(
  state: GameBoardCard["work_state"],
  t: ReturnType<typeof useUiMessages>,
): string | null {
  if (state === "pending") return t.gameBoard.workStatePending;
  if (state === "in_progress") return t.gameBoard.workStateInProgress;
  if (state === "done") return t.gameBoard.workStateDone;
  if (state === "blocked") return t.gameBoard.workStateBlocked;
  return null;
}

function trackBadge(track: GameBoardCard["track"]): string | null {
  if (track === "A" || track === "B" || track === "H") return track;
  return null;
}

function visiblePhaseCards(
  cards: GameBoardCard[],
  showArchived: boolean,
  showDones: boolean,
  track: GameTrackFilter,
): GameBoardCard[] {
  return cards.filter((card) => {
    if (track === "A" && card.track !== "A") return false;
    if (track === "B" && card.track !== "B") return false;
    if (card.work_state === "done") {
      if (!showDones) return false;
      if (card.archived && !showArchived) return false;
    }
    return true;
  });
}

function archiveAction(card: GameBoardCard, expanded: boolean): "archive" | "unarchive" | null {
  if (!expanded || card.kind !== "artifact" || card.work_state !== "done") return null;
  return card.archived ? "unarchive" : "archive";
}

async function copyContinueText(text: string, target: HTMLElement): Promise<void> {
  try {
    const writeText = navigator.clipboard?.writeText;
    if (typeof writeText !== "function") {
      throw new Error("clipboard missing");
    }
    await writeText.call(navigator.clipboard, text);
  } catch {
    const range = document.createRange();
    range.selectNodeContents(target);
    const selection = window.getSelection();
    selection?.removeAllRanges();
    selection?.addRange(range);
  }
}

function ContinueControl({ text }: { text: string }) {
  return (
    <button
      type="button"
      className="text-[11px] font-mono text-foreground/50 select-text text-left bg-transparent border-0 p-0"
      onClick={(event: MouseEvent<HTMLButtonElement>) => {
        event.stopPropagation();
        void copyContinueText(text, event.currentTarget);
      }}
    >
      {text}
    </button>
  );
}

function BlockedByLine({
  blockedBy,
  t,
}: {
  blockedBy: string | null;
  t: ReturnType<typeof useUiMessages>;
}) {
  if (blockedBy === null) return null;
  return (
    <p className="text-[11px] text-foreground/55 mt-1">
      {t.gameBoard.blockedStrip} {blockedBy}
    </p>
  );
}

function ExpandBody({ card, t }: { card: GameBoardCard; t: ReturnType<typeof useUiMessages> }) {
  if (card.kind === "epic") return null;
  if (card.track === "A") {
    return (
      <div className="mt-2">
        {card.blurb !== "" ? (
          <p className="text-[11px] text-foreground/55 whitespace-pre-wrap">{card.blurb}</p>
        ) : null}
        {card.tasks.length === 0 ? (
          <p className="text-[10px] text-foreground/20 italic mt-2">{t.specBoard.noTasksYet}</p>
        ) : (
          <ul className="mt-2 flex flex-col gap-1">
            {card.tasks.map((task) => (
              <li key={task.number} className="text-[11px] text-foreground/50">
                #{task.number} {task.name}
              </li>
            ))}
          </ul>
        )}
        {card.inputs !== "" ? (
          <div className="mt-2">
            <p className="text-[10px] text-foreground/40">{t.gameBoard.inputs}</p>
            <p className="text-[11px] text-foreground/55 whitespace-pre-wrap">{card.inputs}</p>
          </div>
        ) : null}
      </div>
    );
  }
  return (
    <div className="mt-2">
      {card.last_decision !== "" ? (
        <p className="text-[11px] text-foreground/55 whitespace-pre-wrap">{card.last_decision}</p>
      ) : null}
      {card.open !== "" ? (
        <p className="text-[11px] text-foreground/55 whitespace-pre-wrap mt-1">{card.open}</p>
      ) : null}
      {card.recent !== "" ? (
        <p className="text-[11px] text-foreground/55 whitespace-pre-wrap mt-1">{card.recent}</p>
      ) : null}
    </div>
  );
}

function TitleControl({
  title,
  expanded,
  onToggle,
}: {
  title: string;
  expanded: boolean;
  onToggle: () => void;
}) {
  return (
    <button
      type="button"
      aria-expanded={expanded}
      onClick={onToggle}
      className="w-full text-left cursor-pointer bg-transparent border-0 p-0"
    >
      <p className="text-[12px] font-medium text-foreground/90 truncate">{title}</p>
    </button>
  );
}

function ArchiveControls({
  action,
  onSetArchived,
  t,
}: {
  action: "archive" | "unarchive" | null;
  onSetArchived: (archived: boolean) => void;
  t: ReturnType<typeof useUiMessages>;
}) {
  if (action === "archive") {
    return (
      <button
        type="button"
        className="mt-2 text-[10px] text-foreground/40 hover:text-foreground/70"
        onClick={() => {
          onSetArchived(true);
        }}
      >
        {t.specBoard.archive}
      </button>
    );
  }
  if (action === "unarchive") {
    return (
      <button
        type="button"
        className="mt-2 text-[10px] text-foreground/40 hover:text-foreground/70"
        onClick={() => {
          onSetArchived(false);
        }}
      >
        {t.specBoard.unarchive}
      </button>
    );
  }
  return null;
}

function ArtifactCard({
  card,
  expanded,
  onToggle,
  onSetArchived,
  t,
}: {
  card: GameBoardCard;
  expanded: boolean;
  onToggle: (id: string) => void;
  onSetArchived: (id: string, archived: boolean) => void;
  t: ReturnType<typeof useUiMessages>;
}) {
  const track = trackBadge(card.track);
  const state = workStateLabel(card.work_state, t);
  const action = archiveAction(card, expanded);
  return (
    <article draggable={false} className="rounded-lg border border-white/10 bg-white/[0.02] p-3">
      <TitleControl title={card.title} expanded={expanded} onToggle={() => onToggle(card.id)} />
      <div className="flex items-center gap-2 mt-1">
        {track ? <span className="text-[11px] text-foreground/60">{track}</span> : null}
        {state ? <span className="text-[11px] text-foreground/55">{state}</span> : null}
      </div>
      {card.owner !== "" ? <p className="text-[11px] text-foreground/50">{card.owner}</p> : null}
      <p className="text-[10px] text-foreground/30 font-mono truncate mt-0.5">{card.id}</p>
      <BlockedByLine blockedBy={card.blocked_by} t={t} />
      {card.continue !== "" ? <ContinueControl text={card.continue} /> : null}
      {expanded ? (
        <>
          <ExpandBody card={card} t={t} />
          <ArchiveControls
            action={action}
            onSetArchived={(archived) => {
              onSetArchived(card.id, archived);
            }}
            t={t}
          />
        </>
      ) : null}
    </article>
  );
}

function InboxCard({
  card,
  expanded,
  onToggle,
  t,
}: {
  card: GameBoardCard;
  expanded: boolean;
  onToggle: (id: string) => void;
  t: ReturnType<typeof useUiMessages>;
}) {
  return (
    <article draggable={false} className="rounded-lg border border-white/10 bg-white/[0.02] p-3">
      <div className="flex items-baseline gap-2 min-w-0">
        <span className="text-[var(--color-epic-mark)] text-[12px] font-medium shrink-0">E</span>
        <TitleControl title={card.title} expanded={expanded} onToggle={() => onToggle(card.id)} />
      </div>
      {card.summary !== "" ? (
        <p className="text-[11px] text-foreground/55 mt-1">{card.summary}</p>
      ) : null}
      {card.plan_title !== "" ? (
        <p className="text-[10px] text-foreground/40 mt-0.5">{card.plan_title}</p>
      ) : null}
      <p className="text-[10px] text-foreground/30 font-mono whitespace-normal break-all mt-0.5">{card.id}</p>
      <BlockedByLine blockedBy={card.blocked_by} t={t} />
      {card.continue !== "" ? <ContinueControl text={card.continue} /> : null}
      {expanded ? <ExpandBody card={card} t={t} /> : null}
    </article>
  );
}

function BlockedStrip({
  rows,
  t,
}: {
  rows: GameBoard["blocked"];
  t: ReturnType<typeof useUiMessages>;
}) {
  if (rows.length === 0) return null;
  return (
    <div role="region" aria-label={t.gameBoard.blockedStrip} className="flex flex-col gap-1">
      {rows.map((row, index) => (
        <div
          key={`${row.owner}-${index}`}
          className="text-[11px] text-foreground/55 px-1"
        >
          <span>{row.owner}</span>
          {row.task !== "" ? <span>{` ${row.task}`}</span> : null}
          {row.blocked_by !== "" ? <span>{` ${row.blocked_by}`}</span> : null}
        </div>
      ))}
    </div>
  );
}

function DebtStrip({ rows, label }: { rows: GameBoardDebt[]; label: string }) {
  if (rows.length === 0) return null;
  return (
    <div role="region" aria-label={label} className="flex flex-col gap-1">
      {rows.map((row) => (
        <p key={row.id} className="text-[11px] text-foreground/55 px-1 whitespace-normal break-words">
          {row.id} {row.title}
        </p>
      ))}
    </div>
  );
}

function PhaseColumn({
  columnKey,
  title,
  cards,
  current,
  expandedIds,
  onToggle,
  onSetArchived,
  t,
}: {
  columnKey: ColumnKey;
  title: string;
  cards: GameBoardCard[];
  current: boolean;
  expandedIds: Set<string>;
  onToggle: (id: string) => void;
  onSetArchived: (id: string, archived: boolean) => void;
  t: ReturnType<typeof useUiMessages>;
}) {
  return (
    <div className="min-w-0 flex flex-col" aria-current={current ? true : undefined}>
      <h3 className="text-[11px] font-semibold uppercase tracking-wider text-foreground/40 mb-3 px-1">
        {title}
      </h3>
      <div className="flex flex-col gap-2">
        {cards.map((card, index) =>
          columnKey === "inbox" || card.kind === "epic" ? (
            <InboxCard
              key={card.id !== "" ? card.id : `${columnKey}-${index}`}
              card={card}
              expanded={expandedIds.has(card.id)}
              onToggle={onToggle}
              t={t}
            />
          ) : (
            <ArtifactCard
              key={card.id !== "" ? card.id : `${columnKey}-${index}`}
              card={card}
              expanded={expandedIds.has(card.id)}
              onToggle={onToggle}
              onSetArchived={onSetArchived}
              t={t}
            />
          ),
        )}
      </div>
    </div>
  );
}

export function GameBoardTab({
  board,
  project,
  refresh = async () => undefined,
}: GameBoardTabProps) {
  const t = useUiMessages();
  const mapped = phaseLabel(board.phase, t);
  const continueCmd = board.continue;
  const missingState = board.phase === null && board.focus === null;
  const [expandedIds, setExpandedIds] = useState<Set<string>>(() => new Set());
  const [showArchived, setShowArchived] = useState(false);
  const [showDones, setShowDones] = useState(false);
  const [trackFilter, setTrackFilter] = useState<GameTrackFilter>("all");
  const lastProjectRef = useRef(project);

  if (project !== undefined && lastProjectRef.current !== project) {
    lastProjectRef.current = project;
    setExpandedIds(new Set());
    setShowArchived(false);
    setShowDones(false);
    setTrackFilter("all");
  }

  const onToggle = (id: string) => {
    setExpandedIds((prev) => {
      const next = new Set(prev);
      if (next.has(id)) next.delete(id);
      else next.add(id);
      return next;
    });
  };

  const persistArchive = async (cardId: string, archived: boolean): Promise<void> => {
    if (!project) return;
    try {
      const res = await fetch("/api/game-board", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ project, card_id: cardId, archived }),
      });
      if (!res.ok) return;
      await refresh();
    } catch {
      /* GET board stays the source of truth */
    }
  };

  return (
    <section role="region" aria-label={t.tabs.game} className="h-full bg-card p-6 overflow-auto">
      <div className="flex flex-col gap-4">
        <div className="flex flex-col gap-3">
          {missingState ? (
            <p className="text-[12px] text-foreground/40">{t.gameBoard.stateMdMissing}</p>
          ) : mapped ? (
            <p className="text-[12px] font-medium text-foreground/90">{mapped}</p>
          ) : null}
          {!missingState && board.focus !== null ? (
            <p className="text-[12px] text-foreground/70">{board.focus}</p>
          ) : null}
          {continueCmd !== "" ? (
            <p className="text-[12px] font-mono text-foreground/50 select-text">{continueCmd}</p>
          ) : null}
          <div className="flex flex-wrap items-center gap-2 self-start">
            <button
              type="button"
              aria-pressed={showArchived}
              onClick={() => setShowArchived((v) => !v)}
              className={chromeFilterClass(showArchived)}
            >
              {t.specBoard.showArchived}
            </button>
            <span aria-hidden="true">|</span>
            <button
              type="button"
              aria-pressed={showDones}
              onClick={() => setShowDones((v) => !v)}
              className={chromeFilterClass(showDones)}
            >
              {t.gameBoard.showDones}
            </button>
            <span aria-hidden="true">|</span>
            <button
              type="button"
              aria-pressed={trackFilter === "A"}
              onClick={() => setTrackFilter("A")}
              className={chromeFilterClass(trackFilter === "A")}
            >
              {t.gameBoard.trackA}
            </button>
            <button
              type="button"
              aria-pressed={trackFilter === "B"}
              onClick={() => setTrackFilter("B")}
              className={chromeFilterClass(trackFilter === "B")}
            >
              {t.gameBoard.trackB}
            </button>
            <button
              type="button"
              aria-pressed={trackFilter === "all"}
              onClick={() => setTrackFilter("all")}
              className={chromeFilterClass(trackFilter === "all")}
            >
              {t.gameBoard.trackAll}
            </button>
          </div>
        </div>
        <BlockedStrip rows={board.blocked} t={t} />
        <DebtStrip rows={board.debt ?? []} label={t.specBoard.openTechDebt} />
        <div className="grid grid-cols-4 gap-4">
          {COLUMN_KEYS.map((key) => (
            <PhaseColumn
              key={key}
              columnKey={key}
              title={columnTitle(key, t)}
              cards={
                key === "inbox"
                  ? board.inbox
                  : visiblePhaseCards(board[key], showArchived, showDones, trackFilter)
              }
              current={columnIsCurrent(key, board.phase)}
              expandedIds={expandedIds}
              onToggle={onToggle}
              onSetArchived={(id, archived) => {
                void persistArchive(id, archived);
              }}
              t={t}
            />
          ))}
        </div>
      </div>
    </section>
  );
}
