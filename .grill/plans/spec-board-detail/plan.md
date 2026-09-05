---
slug: spec-board-detail
title: Detalle de specs en el Kanban, archivo y Last indexed local
status: closed
entry_type: mixed
linked_sdd: .sdd-skill/ (referencia; ciclo anterior completed — este plan no depende de él)
linked_gamedev: none
created: 2026-08-29
last_updated: 2026-08-30
---

# Detalle de specs en el Kanban, archivo y Last indexed local

## Vision

El Dashboard y el workspace ya están en uso. El tablero de Specs funciona, pero da poco: las tarjetas de Todo y Done casi no se pueden abrir, las tareas y el título solo se leen de la spec activa, y Last indexed se pinta en UTC.

Este plan no rediseña el Kanban ni el Graph. Añade valor al tablero que ya existe: un expand ligero al clic, archivo manual de specs Done guardado en CBM, y Last indexed en el reloj de la máquina donde corre el navegador.

## Core Objective and Expected Result

Listo significa tres cosas visibles. Clic en cualquier spec abre el mismo expand de tarjeta de hoy, con un resumen de una o dos frases del objetivo y la lista de tareas según la columna. En Done, Archivar es inmediato y vive en CBM; el tablero puede mostrar u ocultar archivados (empiezan ocultos cada visita) y se puede desarchivar. Last indexed deja de verse en UTC y usa la zona horaria local en Dashboard, header del workspace y sello del ADR.

## Scope

**In:**
- Expand inline en las tres columnas; varias tarjetas pueden quedar abiertas (ADR-002, ADR-008)
- Blurb: 1–2 frases de `## Executive Summary`. Si falta, se omite; el expand sigue (ADR-003, ADR-006)
- Todo: solo tareas pendientes. In progress: todas + estatus. Done: lista completa (el botón Archivar es la épica 002)
- Archivo/desarchivo en CBM, sin tocar skills (ADR-001). Siguen en Done, ocultos por defecto, toggle de sesión, sin modal de confirmación (ADR-004, ADR-007, ADR-009)
- Last indexed en TZ local del navegador en las tres superficies que ya usan `formatIndexedAt` (ADR-005). El ISO almacenado no cambia

**Explicitly out:**
- Escribir, mover o renombrar archivos de skills (`.sdd-skill/` u otras). No se toca `active.json` ni el Status de `spec.md`
- Cuarta columna Archivados, overlay/flotante, accordion, diálogo de confirmar al archivar
- Recolorear o rediseñar el Graph 3D
- Cambiar el ciclo de agentes de sdd-skill

## Target Audience / User

Operador humano del workspace Specs en localhost:9749. Los agentes de sdd-skill siguen dueños del ciclo; este plan no les mueve archivos.

## Known Constraints

- `spec_board.c` hoy solo enriquece la spec activa. Hay que leer título, Executive Summary y `tasks.md` de todas las specs; sigue sin escribir skills.
- sdd-skill no tiene `archived`. Correcto: el archivo es CBM.
- `formatIndexedAt` fuerza UTC (SDD-ADR-003). Este plan lo cambia a TZ local.
- El plan `executive-ui-ia` dejó fuera rediseñar el Kanban y escribir skills. Este plan cubre el Kanban sin escribir skills.
- `history/test_results.log` es un log compartido; el PASS de `Task #N` puede cruzar specs. Limitación ya documentada; sigue siendo pregunta de implementación para columnas no activas.

## Prerequisites / Cross-Cutting Dependencies

- El tab Specs del workspace ya monta SpecBoardTab (spec-002).
- `Project.indexed_at` ya existe; no se inventa un segundo campo de frescura.
- El archivo (épica 002) vive en el expand de Done: no empieza hasta que 001 deje abrir esas tarjetas.

## Roadmap / Epics

| # | Epic | Summary (1 line) | Open questions | Status |
|---|------|------------------|----------------|--------|
| 001 | spec-card-expand | Expand inline en todo / in progress / done: blurb + tareas | 9 | detailed |
| 002 | spec-archive | Archivar/desarchivar en CBM; ocultos en Done; toggle de sesión | 9 | detailed |
| 003 | last-indexed-local | Last indexed en TZ local del navegador (3 superficies) | 5 | detailed |

Detalle: `epics/epic-001-spec-card-expand.md`, `epics/epic-002-spec-archive.md`, `epics/epic-003-last-indexed-local.md`.

Las preguntas abiertas son para quien implemente (sdd-skill u otro): parseo, persistencia, API, placement del toggle, etc. No se resolvieron aquí.

Orden: 001 → 002. 003 es independiente (paralelo o al final). 002 no empieza hasta que el expand de Done en 001 exista.

## Vocabulary and Decisions

See `glossary.md` and `decisions/` (ADR-001 … ADR-009).
