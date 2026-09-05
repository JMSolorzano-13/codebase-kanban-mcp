---
slug: add-epics-plans-kanban
title: Épicas de grill-skill en el Kanban de Specs
status: closed
entry_type: mixed
linked_sdd: spec-008-g8r-grill-epic-todo, spec-009-t4x-specs-tab-grill-presence
linked_gamedev: none
created: 2026-08-30
last_updated: 2026-08-30
---

# Épicas de grill-skill en el Kanban de Specs

## Vision

El tab Specs de cada proyecto ya mostraba el detalle de las specs de sdd-skill. Faltaba el otro lado del mismo flujo: grill-skill genera planes y épicas en `.grill/` que después se convierten en specs. El Kanban debe mostrar esas épicas en Todo, mezcladas con specs que aún no arrancan, sin rediseñar el tablero ni orquestar el ciclo de sdd-skill.

## Core Objective and Expected Result

Listo significa: el operador abre Specs. Si el path tiene `.grill/`, Todo mezcla specs sdd (planned/draft) con épicas grill (título, summary, plan al que pertenecen). In progress y Done siguen siendo solo specs. Una épica deja Todo y aparece como spec cuando sdd-skill crea esa spec en disco; el tablero refleja el filesystem. CBM no escribe `.grill/` ni `.sdd-skill/` para “mover”. El tab Specs aparece si hay `.sdd-skill/` o `.grill/`.

Eso ya está construido: spec-008 (Todo mezclado) y spec-009 (tab si grill, aunque no haya sdd).

## Scope

**In:**
- Mixed Todo: specs planned/draft + épicas grill no convertidas (ADR-001)
- Match de conversión: `Companion to:` en spec.md + `active.json.source.grill_epic`; ruta exacta (ADR-002)
- Tab Specs visible si existe `.sdd-skill/` o `.grill/`
- Tarjeta de épica: título, summary, plan; marca kind = letra E (ADR-004)
- Elegibilidad: todas las épicas no convertidas, todos los planes, pending y detailed (ADR-003)
- Orden en Todo: épicas (por plan, luego NNN) y después specs planned/draft (ADR-005)
- Tope propio de épicas, misma magnitud que specs; overflow se omite (ADR-007)
- Mismo GET `/api/spec-board`, campos aditivos; tab sigue “Specs” (ADR-008)
- Presence/kind aditivos para un plan futuro de gamedev (ADR-006)

**Explicitly out:**
- Drag o botón en el Kanban que cree/mueva specs o borre épicas
- Escribir, mover o renombrar archivos de `.grill/` o `.sdd-skill/`
- Match por nombre/kebab o tabla CBM epic↔spec
- Rediseñar el Kanban de tres columnas o el Graph 3D
- Leer `.gamedev/`, pintar o ocultar por gamedev (otro plan)
- Decidir si un path puede tener sdd y gamedev a la vez
- Chrome “has more” cuando hay overflow del tope

## Target Audience / User

Operador humano del workspace Specs en localhost:9749. sdd-skill y grill-skill siguen dueños de sus árboles; este plan solo lee.

## Known Constraints

- Constitución I.2: el indexer/graph-ui no escribe archivos de ciclo de skills.
- El Kanban no tiene drag. Columnas de specs = `active.json`. Épicas solo en Todo.
- Un path puede tener varios planes grill independientes.
- Phase 4 (preguntas abiertas Architect/PM) no se corrió: el usuario pidió esperar y sdd-skill armó las specs directo desde las épicas pending.

## Prerequisites / Cross-Cutting Dependencies

- Tab Specs y SpecBoardTab ya existían (spec-002 / spec-005 / spec-006).
- grill-skill filesystem: `index.md` + `plans/<slug>/epics/epic-NNN-<name>.md`.

## Roadmap / Epics

| # | Epic | Summary (1 line) | Open questions | Status |
|---|------|------------------|----------------|--------|
| 001 | grill-epic-todo | Todo mezcla épicas grill no convertidas con specs planned/draft | 0 (Phase 4 skipped) | detailed — spec-008 closed |
| 002 | specs-tab-grill-presence | Tab Specs si hay `.grill/` aunque no haya sdd | 0 (Phase 4 skipped) | detailed — spec-009 closed |

Detalle: `epics/epic-001-grill-epic-todo.md`, `epics/epic-002-specs-tab-grill-presence.md`.

Orden entregado: 001 (spec-008) → 002 (spec-009).

## Vocabulary and Decisions

See `glossary.md` and `decisions/` (ADR-001 … ADR-008).

## Reconciliation Notes (update mode)

2026-08-30 close: usuario `/grill-skill close` — ambas épicas finalizadas. `.sdd-skill/specs/active.json` completed incluye spec-008-g8r-grill-epic-todo y spec-009-t4x-specs-tab-grill-presence. `source.grill_epic` apunta a epic-002. No hubo split/merge ni ADR revertido. Scope gamedev sigue fuera (otro plan).
