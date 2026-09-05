---
slug: tech-debt-and-epics-registry
title: Deuda técnica en tableros e Inbox Game vía epics_registry
status: draft
entry_type: mixed
linked_sdd: none
linked_gamedev: none
created: 2026-09-01
last_updated: 2026-09-01
---

# Deuda técnica en tableros e Inbox Game vía epics_registry

## Vision

Los tableros Specs y Game ya leen el ciclo de cada skill y las épicas de grill. Faltan dos registros que las skills ya escriben en disco (deuda técnica, y el Epic Tracking de gamedev) y una ficha de épica cuyo path se corta.

## Core Objective and Expected Result

Listo significa: el operador abre Specs y ve, en el chrome del tablero (no en una columna nueva, no mezclado con Todo), los ítems abiertos de `.sdd-skill/baseline/TECH_DEBT.md`. Abre Game y ve lo mismo para las líneas `debt:*` abiertas de `.gamedev/backlog.md`. En Game, Inbox muestra solo las épicas grill que aún faltan: si existe `.gamedev/epics_registry.md`, salen de Inbox las que están `in_progress`, `closed` o `parked`; si el archivo no existe, sigue valiendo Companion-to / mapa de `roadmap.md`. En Specs, la conversión por Companion-to no cambia. En Todo (Specs) e Inbox (Game), la ficha de épica mantiene el nombre corto y debajo el path completo, sin recortar. CBM solo lee.

## Scope

**In:**
- Lista de deuda abierta en chrome de Specs (TECH_DEBT.md, Status ≠ resolved) y de Game (backlog.md `debt:*` sin `resolved-by`) — ADR-001, ADR-002
- Inbox Game: hide por `epics_registry` (Plan+NNN; hide = in_progress|closed|parked; sin fila = se queda) — ADR-003
- Sin `epics_registry.md` → hide anterior (Companion-to / roadmap slug+NNN) — ADR-004
- Specs: matcher Companion-to / `active.json` intacto — ADR-005
- Path completo con wrap bajo el nombre corto, Todo e Inbox — ADR-006
- Mismos GET `/api/spec-board` y `/api/game-board`, campos aditivos; hide en servidor — ADR-007
- Specs: franja de deuda encima de las 3 columnas; fila = TD-NNN + título — ADR-008
- Game: franja de deuda después de BlockedStrip; fila = tag `debt:*` + descripción — ADR-009

**Explicitly out:**
- Columna nueva de deuda, o tarjetas de deuda en Todo/Inbox
- Leer `epics_registry` para ocultar Todo en Specs
- Escribir `.grill/`, `.sdd-skill/` o `.gamedev/` (incluido crear el registry)
- Cambiar el XOR Specs/Game, las columnas del Kanban, o el archive
- Ítems de deuda resueltos, o líneas de backlog sin tag `debt:`
- Match fuzzy por kebab/nombre

## Target Audience / User

Operador humano del workspace CBM en localhost. CBM solo lee; sdd-skill, gamedev-skill y grill-skill siguen dueños de sus árboles.

## Known Constraints

- Constitución I.2: CBM no escribe archivos de ciclo de skills.
- Specs y Game son XOR por path (silent win gamedev).
- gamedev no tiene TECH_DEBT.md; la deuda vive en backlog.md.
- `epics_registry.md` es lazy: muchos juegos no lo tendrán.
- Inbox Game hoy usa Companion-to o el mapa de roadmap.md (ADR-006 del plan add-gamedev-skill).

## Prerequisites / Cross-Cutting Dependencies

- GET /api/spec-board y GET /api/game-board ya existen.
- EpicCard / InboxCard ya pintan id + nombre corto.
- gamedev-skill v1.12.0 define epics_registry.md.

## Roadmap / Epics

Confirmado 2026-09-01. Orden: 001 → 002 → 003. 003 no espera a 002.

| # | Epic | Summary (1 line) | Open questions | Status |
|---|------|------------------|----------------|--------|
| 001 | [specs-debt-and-path](epics/epic-001-specs-debt-and-path.md) | Specs: lista TD abierta en chrome + path completo en Todo | parse/cap (handoff) | detailed |
| 002 | [game-inbox-registry](epics/epic-002-game-inbox-registry.md) | Game Inbox: hide por registry (o fallback ADR-006) + path completo | parse (handoff) | detailed |
| 003 | [game-debt-chrome](epics/epic-003-game-debt-chrome.md) | Game: lista debt:* abierta en chrome | parse/cap (handoff) | detailed |

## Vocabulary and Decisions

See `glossary.md` and `decisions/` (ADR-001 … ADR-009).

## Reconciliation Notes (update mode)

(none yet)
