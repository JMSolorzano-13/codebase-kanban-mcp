# Task #4 — AdrTab pane
Reading time: 2-3 min
Last updated: 2026-08-29 — spec-002-p8w-project-workspace | Patterns: ✓

## What changed (plain language)

The project ADR is now a full editor pane: load the existing markdown, save the whole document, or post an empty document to match today's Delete. Save failures stay on the draft and show an error. The old modal button is still on disk and not used here; App will mount this pane in Task #5.

## Files modified

| File | What it does | Lines ± |
|---|---|---|
| `graph-ui/src/components/AdrTab.tsx` | GET/POST `/api/adr`; alert/status; `onDirtyChange` | new |
| `graph-ui/src/components/AdrTab.test.tsx` | load / save / empty / 500 / delete | new |
| `graph-ui/src/lib/i18n.ts` | `tabs.adr`, `adr.saveSuccess`, `adr.saveError`, `adr.unsavedConfirm` | + |
| `graph-ui/src/lib/i18n.test.ts` | Locks those keys en+zh | + |

`AdrButton.tsx` left for Task #5 to delete.

## Key decisions

| Decision | Why | ADR |
|---|---|---|
| Check `res.ok` | AdrButton closed the modal on HTTP 500 | SDD-ADR-011 |
| POST `""` is Delete; do not assert `has_adr:false` | C upserts empty | plan API note |
| English placeholder even in zh | Gherkin asserts "Architecture Decision Record" | US-004 |
| `onDirtyChange` only | `window.confirm` is App Task #5 | SDD-ADR-012 |

## Save path

```mermaid
sequenceDiagram
  participant Pane as AdrTab
  participant API as GET/POST /api/adr
  Pane->>API: GET project
  API-->>Pane: has_adr + content
  Pane->>Pane: textarea; dirty if content != lastClean
  Pane->>API: POST on Save
  alt not res.ok
    API-->>Pane: alert; draft stays
  else ok
    API-->>Pane: status; lastClean = saved
  end
```

## Debugging

| If this fails | File:line | Cause | Fix |
|---|---|---|---|
| Save 500 looks clean | `AdrTab.tsx` persist | `lastClean` set before `res.ok` | Return on `!res.ok`; keep textarea |
| Placeholder posted on open | `AdrTab.tsx` fetchAdr | Empty GET wrote placeholder into value | Value stays `""`; placeholder is attribute only |
| Delete hidden after save | `AdrTab.tsx` hasAdr | `has_adr` not set true on non-empty POST | Set true when `nextContent !== ""` |
| Modal overlay appears | `AdrTab.tsx` root | AdrButton mounted | Pane only; no `fixed inset-0` |

## Project fit

- Before: ADR was `AdrButton` modal, unmounted on Dashboard.
- After: reusable pane. Next: Task #5 mounts it, confirms dirty leave, deletes AdrButton.

## Pattern Notes

Patterns: ✓. Same `/api/adr` body as AdrButton. Same grayscale chrome. Textarea is text, not HTML. No C change.

## Quick refs

- Spec US-004: `.sdd-skill/specs/spec-002-p8w-project-workspace/spec.md`
- Tests: `cd graph-ui && npx vitest run src/components/AdrTab.test.tsx`
