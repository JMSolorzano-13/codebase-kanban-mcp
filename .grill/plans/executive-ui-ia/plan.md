---
slug: executive-ui-ia
title: Dashboard ejecutivo y workspace por proyecto
status: closed
entry_type: mixed
linked_sdd: spec-001-w3q-executive-dashboard
linked_gamedev: none
created: 2026-08-29
last_updated: 2026-08-29
---

# Dashboard ejecutivo y workspace por proyecto

## Vision

codebase-memory-mcp ya tiene un motor de grafo y una UI en localhost:9749, pero esa UI se siente de laboratorio: tema verde/negro, pestañas globales (Specs por defecto, Graph, Projects, Control) y un listado de proyectos que mezcla salud del índice con totales de nodos, edges, chips de labels y un modal de ADR.

Este plan cambia la cara a un producto ejecutivo: un Dashboard de cuenta y, al entrar, un workspace por proyecto que pueda crecer con más pestañas. El grafo 3D no se rediseña; el chrome sí.

## Core Objective and Expected Result

Listo significa dos niveles. En casa: lista de proyectos y Control en la misma pantalla, sin secciones de nodes/edges, con fecha de último index/reindex. Al entrar a un proyecto: Graph por defecto, Specs solo si existe `.sdd-skill/`, ADR siempre (editor a mano). Un Path es un Project (1:1); intentar indexar uno que ya existe bloquea y redirige. Los duplicados viejos se muestran, se entra al más reciente y se pide autorización para borrar el anterior.

Fase 2 (después del diseño): el ADR se llena o actualiza en el reindex manual parseando tres documentos de sdd-skill, sin LLM, como se parsean Specs y nodos. El bloque generado se regenera; el bloque manual no se toca.

## Scope

**In:**
- Tema gris oscuro solo en chrome (Dashboard, tabs, botones, ligas, Control, encabezados, modales). Contraste entre grises. Rojo/ámbar para error y health.
- Dashboard: proyectos + Control juntos. Sin agregados ni chips de nodes/edges. Sin ADR en las tarjetas.
- Detalle de proyecto en lista: nombre, path, fecha de último index, entrar, borrar, alta de índice. Health se decide en spec (excepción semántica ya prevista).
- Workspace por proyecto con pestañas escalables: Graph, Specs condicional, ADR.
- Fecha de último index/reindex también en el encabezado del workspace.
- Identidad Path↔Project 1:1. Sin campo Project ID opcional en el modal. Alta duplicada = bloqueo + redirect.
- Duplicados ya existentes: conflicto visible, entrar al más reciente, pedir borrar el anterior.
- Fase 2: parse de `.sdd-skill/context_ai.md`, `baseline/TECH_STACK.md`, `baseline/ARCHITECTURE_ADR.md` solo en reindex manual.

**Explicitly out:**
- Recolorear nodos/edges del graph 3D.
- Rediseñar el canvas 3D, el Kanban de Specs o el motor C de indexación de código.
- Llenar ADR con LLM o en el watcher/auto-sync.
- Parsear DEV_LOG, TECH_DEBT, constitution, human/*, specs/* o history/* hacia el ADR.
- Escribir de vuelta a `.sdd-skill/` o al source (Specs y nodos siguen solo lectura).
- Renombrar proyectos custom que ya existen (solo se resuelven choques).
- Dependencia de `.sdd/` o `.gamedev/` en este repo (no hay).

## Target Audience / User

Quien opera CBM desde el navegador (humano) y los agentes que después implementen este plan. El Dashboard es para el operador; el ADR también lo consumen agentes vía `manage_adr`.

## Known Constraints

- UI actual: React en `graph-ui/`, servida por el binario, i18n en/zh.
- Identidad hoy: nombre de proyecto = archivo `.db`, no hay unique de `root_path`.
- ADR hoy: markdown en SQLite `project_summaries`, MCP `manage_adr`, UI `/api/adr`.
- Specs ya se parsean en C (`spec_board.c`) sin escribir la skill.
- `indexed_at` ya viene en `list_projects`.
- sdd-skill prohíbe indexar `.sdd-skill/` como grafo de código; el parse de ADR lee archivos, no el graph.
- Este checkout no tiene `.sdd/` ni `.gamedev/`.

## Prerequisites / Cross-Cutting Dependencies

- Tokens de chrome gris: los introduce la épica 001 y los reutilizan 002 y 003.
- Paleta 3D (`colorForLabel` y layout) no se toca.
- Contrato de pestañas extensible: lo define 002; pestañas futuras (p. ej. gamedev) no se construyen aquí.

## Roadmap / Epics

| # | Epic | Summary (1 line) | Open questions | Status |
|---|------|------------------|----------------|--------|
| 001 | executive-dashboard | Home de cuenta: proyectos + Control, chrome gris, sin nodes/edges | 11 | detailed |
| 002 | project-workspace | Interior del proyecto: Graph / Specs? / ADR, fecha en header | 11 | detailed |
| 003 | path-project-identity | 1:1 Path↔Project, redirect, resolver duplicados viejos | 11 | detailed |
| 004 | adr-parse-on-reindex | Fase 2: parse del trio sdd-skill en reindex manual | 12 | detailed |

Detalle en `epics/epic-NNN-<name>.md`. Las preguntas abiertas son para el agente que implemente (sdd-skill u otro), no están resueltas aquí.

Orden: 001 → 002 → 003 → 004. 004 no empieza hasta que el tab ADR de 002 exista.

## Vocabulary and Decisions

Ver `glossary.md` y `decisions/` (ADR-001 a ADR-012). No se duplican aquí.

## Reconciliation Notes (update mode)

(vacío — este plan no ha pasado por `/grill-skill update`)
