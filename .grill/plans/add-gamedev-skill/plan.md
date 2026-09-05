---
slug: add-gamedev-skill
title: Tab y tablero de gamedev-skill
status: closed
entry_type: mixed
linked_sdd: none
linked_gamedev: none
created: 2026-08-30
last_updated: 2026-08-31
---

# Tab y tablero de gamedev-skill

## Vision

El workspace de CBM ya lee el ciclo de sdd-skill (Specs) y las épicas de grill-skill (Todo mezclado). gamedev-skill es otro ciclo: tres fases, muchos agentes en paralelo, tracks A/B/híbrido, y el rigor tipo SDD ya vive dentro de Track A. Falta una pestaña y un tablero propios que dejen ver fase, track, paralelismo y dependencias — no reutilizar el Kanban de tres columnas de Specs.

## Core Objective and Expected Result

El operador abre un proyecto con `.gamedev/` y ve la pestaña Game (Specs no aparece). El tablero tiene cuatro columnas: Inbox | Pre-producción | Producción | Post-producción y lanzamiento. Cada carta es un artefacto que existe, con track, work-state, agente dueño y `continue @rol` copiable. Varias cartas de una fase pueden ir en paralelo. Inbox muestra épicas grill no convertidas; salen con Companion-to o el mapa de `roadmap.md`. Expand in-place; archive solo de done (CBM). Bloqueos vivos = `blocked-by` de `state.md` más una franja del tablero. CBM no escribe `.gamedev/` ni lanza la skill. La pestaña ADR se llena del trío gamedev en reindex. Paths sin `.gamedev/` siguen Specs+grill y el trío sdd.

## Scope

**In:**
- XOR por path: `.gamedev/` ⇒ pestaña Game; Specs omitida (ADR-001)
- Tablero: 4 columnas Inbox + 3 fases; track = insignia; work-state en la carta; paralelo dentro de fase (ADR-003)
- Grill Inbox; convertida = Companion-to exacto o mapa roadmap slug+NNN (ADR-002, ADR-006)
- CBM lee el ciclo; única mutación = archive de done (ADR-004)
- Carta = artefacto que existe + agente dueño (ADR-005)
- Expand in-place: Track A blurb+tasks; Track B header; Archive en done (ADR-007)
- Pestaña ADR: trío gamedev en reindex; no merge con sdd (ADR-008)
- Cómo usar: mapa + `continue` / `continue @rol` copiable; CBM no lanza (ADR-009)
- Deps: blocked-by + franja; Inputs en expand Track A; no grafo de roadmap (ADR-010)

**Explicitly out:**
- Dos pestañas Specs+Game en el mismo path
- Banner de conflicto si coexisten las carpetas
- Meter el ciclo gamedev en el Kanban de tres columnas de Specs (status ≠ eje)
- Arrastrar / marcar done / invocar la skill desde CBM; escribir `.gamedev/`
- Parsear `roadmap.md` a un grafo; pintar Needs de agents.md como aristas
- Dejar de soportar sdd+grill en paths sin `.gamedev/`
- Mezclar el trío sdd en el generated ADR de un path gamedev

## Target Audience / User

Operador humano del workspace en localhost:9749. gamedev-skill y grill-skill siguen dueños de `.gamedev/` y `.grill/`. CBM solo lee (constitución I.2).

## Known Constraints

- Constitución I.2: indexer/graph-ui no escriben archivos de ciclo de skills.
- Path = Project 1:1.
- spec-board read/to_json no emite gamedev → el tablero Game no puede ir en GET /api/spec-board.
- `cbm_spec_board_gamedev_skill_present` y `/api/skill-presence` ya existen.

## Prerequisites / Cross-Cutting Dependencies

- Contrato de pestañas extensible (spec-002 / types.ts WORKSPACE_TABS).
- Presence aditiva ya prevista (ADR-006 del plan add-epics-plans-kanban).
- GET nuevo para el tablero Game (no alargar spec-board). Detalle de ruta = spec-level.

## Roadmap / Epics

| # | Epic | Summary (1 line) | Open questions | Status |
|---|------|------------------|----------------|--------|
| 001 | game-tab-silent-win | Path con `.gamedev/` muestra Game y oculta Specs; chrome de fase/focus/`continue`; GET nuevo | 17 | detailed |
| 002 | game-phase-board | Cuatro columnas con cartas de artefacto, Inbox grill y ocultar convertidas | 21 | detailed |
| 003 | expand-archive-deps | Expand in-place, archive de done, franja blocked-by e Inputs Track A | 20 | detailed |
| 004 | adr-fill-gamedev-trio | Reindex llena ADR del trío `.gamedev/`; no merge con sdd | 14 | detailed |

Detalle: `epics/epic-001-game-tab-silent-win.md`, `epic-002-game-phase-board.md`, `epic-003-expand-archive-deps.md`, `epic-004-adr-fill-gamedev-trio.md`.

Orden: 001 → 002 → 003 → 004. 004 no depende de 002/003.

## Vocabulary and Decisions

See `glossary.md` and `decisions/` (ADR-001 … ADR-010).

## Reconciliation Notes (update mode)

2026-08-31 close: usuario `/grill-skill close` — entrevista terminada, 4 épicas detailed (72 open-qs para sdd-skill). Aún no hay spec vinculada (`linked_sdd: none`). No hubo split/merge ni ADR revertido.
