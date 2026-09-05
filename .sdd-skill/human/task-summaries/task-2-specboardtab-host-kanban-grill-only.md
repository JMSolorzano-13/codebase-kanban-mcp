# Task #2 — SpecBoardTab host Kanban on grill-only
Reading time: 2-3 min
Last updated: 2026-08-30 — spec-009-t4x-specs-tab-grill-presence | Path=full | Patterns: ✓

## What changed (plain language)

The Specs pane now shows the three-column board when the project has grill-skill even if it has no sdd-skill. A grill-only repo no longer gets the "doesn't use sdd-skill" empty message.

If a later refresh says both skills are gone, or the board never arrived after load, that same empty message still appears. While the first board is still loading, the pane still says Loading — not the empty-skill copy.

## Files modified

Working-tree `git diff` vs last commit on `SpecBoardTab.tsx` also carries spec-005..008 (expand, archive, EpicCard). This task is the host gate + inverted host tests.

| File | What it does | Lines ± |
|---|---|---|
| `graph-ui/src/components/SpecBoardTab.tsx` | Kanban when `sdd === true` OR `grill === true`; `notSddSkill` last-resort | this task: breadcrumbs + `:344-347` (file 399 lines) |
| `graph-ui/src/components/SpecBoardTab.test.tsx` | Invert spec-008 grill→notSddSkill; grill-only Kanban + empty + both-false + `!board` | host block `:315-464`; file 1018 lines |

`EpicCard`, expand Set, archive filter, Todo order, cap, `useSpecBoard`, `formatIndexedAt`, `colors.ts`, `i18n.ts`, C/HTTP/MCP were not edited. No new i18n key. No host poll.

## Key decisions

| Decision | Why | ADR |
|---|---|---|
| Kanban when `board` and (`sdd_skill_present === true` OR `grill_skill_present === true`) | Grill-only must reach Todo / In progress / Done | SDD-ADR-040; US-002 |
| Keep `notSddSkill` when `!board` after load or both flags false | One-shot strip vs 4s poll; direct host mount; stale both-false | SDD-ADR-040 |
| Loading (`loading && !board`) stays `t.common.loading` | Must not look like "no skill" | SDD-ADR-040 |
| Existing `notSddSkill` copy; no new i18n key | No Gherkin Then for a new string | SDD-ADR-040; II.3 |
| Invert spec-008 host its that asserted notSddSkill when grill is true | Old Then must not stay green | plan Testing Strategy |
| Missing `grill_skill_present` on an sdd-true board still paints | Same OR as the strip hook; sdd alone is enough | SDD-ADR-039 / 040 |
| Do not edit EpicCard / expand / archive / `useSpecBoard` | Mixed Todo and 4s poll stay spec-008 / spec-006 | IX.2; SDD-ADR-041 |

## How the host gate works

```mermaid
flowchart TB
  Mount["SpecBoardTab project set"] --> Poll["useSpecBoard 4s same GET"]
  Poll --> Load{"loading && !board?"}
  Load -->|yes| Wait["t.common.loading"]
  Load -->|no| Gate{"board AND (sdd === true OR grill === true)?"}
  Gate -->|yes| Kanban["Todo / In progress / Done"]
  Gate -->|!board or both false| Last["t.specBoard.notSddSkill"]
  Kanban --> GrillOnly{"grill true, sdd false, specs []"}
  GrillOnly -->|epics has inbox| TodoE["Todo: E + title + id; other cols noSpecs"]
  GrillOnly -->|epics []| AllEmpty["all three cols noSpecs; not Last"]
```

`pendingCount` and EpicCard paint are unchanged. In progress / Done still do not receive `epics`. App strip / `?tab=specs` is Task #3.

## Debugging

| If this fails | File:line | Cause | Fix |
|---|---|---|---|
| Grill-only shows "doesn't use sdd-skill" | `SpecBoardTab.tsx:345` | Host still `!board.sdd_skill_present` | OR `grill_skill_present === true`. Test `:372` |
| Grill-only + empty `epics`/`specs` shows notSddSkill | `SpecBoardTab.tsx:345` | Empty lists treated as absent | Kanban + three `noSpecs`. Test `:404` |
| Both flags false still paints Todo / epic id | `SpecBoardTab.tsx:345` | Last-resort dropped | `!(sdd \|\| grill)` → `notSddSkill`. Test `:443` |
| `board` null after load paints columns | `SpecBoardTab.tsx:345` | `!board` skipped | Last-resort. Test `:427` |
| First load shows notSddSkill | `SpecBoardTab.tsx:340-342` | Loading branch lost or ordered after the gate | `loading && !board` → `t.common.loading`. Test `:338` |
| sdd-true board missing `grill_skill_present` hides the pane | `SpecBoardTab.tsx:345` | Grill required | sdd OR; missing grill is false. Test `:355` |
| In progress / Done shows an epic id | `SpecBoardTab.tsx:238`, `:372-393` | `epics` passed off Todo | Unchanged spec-008. Only Todo gets `board.epics ?? []` |
| Interval or `/api/skill-presence` from this host | `useSpecBoard.ts` (untouched) | Second poll leaked into the pane | 4s stays board refresh. SDD-ADR-041 |

## Project fit

- Before: Task #1 made the strip hook `present` on sdd OR grill. The host still replaced the board with `notSddSkill` whenever sdd was false, so a grill-only tab (once Task #3 mounts it) would open an empty-skill pane.
- After this task: grill-only paints Kanban (epic in Todo, or three `noSpecs`). Last-resort copy stays for `!board` / both-false. Loading copy unchanged. EpicCard / expand / archive / poll unchanged.
- Next: Task #3 App strip, deep-link `?tab=specs`, Enter, omit Gherkin. Then @review on this task's code.

## Pattern Notes

Patterns: ✓. Host OR matches Task #1 `bodyHasSkill` (strict `=== true`; missing grill is false). Last-resort kept — same `notSddSkill` key, no second empty-state (II.3, SDD-ADR-040). Loading branch still runs first. Same GET; no `/api/skill-presence`; `useSpecBoard` 4000 ms untouched (IV.3, SDD-ADR-041). EpicCard / expand Set / archive / `formatIndexedAt` / `colors.ts` untouched (IX.2). English Then text (II.3). Breadcrumbs on `SpecBoardTab.tsx` and `SpecBoardTab.test.tsx` (VII.2).

Constitution has I–IX only (no Section X). IX.2 still says spec-008 "tab still omit-until sdd" — this spec supersedes that host/strip rule; IX append waits for spec close. No constitution gap this task.

## Quick refs

- Spec US-002: `.sdd-skill/specs/spec-009-t4x-specs-tab-grill-presence/spec.md`
- Plan host gate: `.sdd-skill/specs/spec-009-t4x-specs-tab-grill-presence/plan.md` (SDD-ADR-040)
- ADR: SDD-ADR-040 (Kanban on sdd OR grill; `notSddSkill` last-resort); SDD-ADR-039 (hook OR); SDD-ADR-041 (graph-ui only)
- Tests: `cd graph-ui && npx vitest run src/components/SpecBoardTab.test.tsx`
- Gherkin owners: grill-only Kanban + empty-epics pane → this file; strip / deep-link / Enter → Task #3
- Constitution: II.2–3, IV.3, V.1/V.4, VII.2, IX.2 (EpicCard / expand / archive stay)
