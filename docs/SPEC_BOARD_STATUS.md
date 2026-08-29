# Spec Board (Kanban) — Estatus completo y guía de continuación

**Última actualización:** este documento fue generado al final de una sesión de diseño+implementación con Claude (chat). A partir de aquí lo continúa un agente local (Cursor/Claude Code u otro).

**Repo de este proyecto:** `/Users/jmsolorzano/SWE/public-repositories/codebase-memory-mcp`
**Módulo afectado:** `graph-ui` (frontend embebido) + daemon C (`src/ui/`)

---

## 1. Objetivo final (el "por qué")

`codebase-memory-mcp` es un MCP + grafo de código (SQLite) que usan las skills `sdd-skill` y `gamedev-skill` de Juan como contexto de código (no de gestión de trabajo). Cada proyecto que usa `sdd-skill` mantiene, aparte del grafo, una carpeta `.sdd-skill/` en el filesystem con specs, tareas y estado de ciclo — pero **no hay ninguna vista visual de eso hoy**. Todo se sabe leyendo archivos markdown a mano.

**Objetivo:** agregar un tab **"Specs"** a la UI web que ya trae `codebase-memory-mcp` (`graph-ui`), que al abrirlo dé, de un vistazo:
- en qué spec está el proyecto ahora mismo,
- cuántas specs quedan pendientes,
- qué tarea se está trabajando ahora mismo dentro de esa spec,
- qué tareas de esa spec ya están hechas y cuáles faltan.

Todo esto **sin pedirle nada nuevo a los agentes de sdd-skill** — se deriva leyendo archivos que el ciclo de sdd-skill ya produce. Cero costo en tokens (el parseo lo hace el daemon en C, no un LLM).

**Alcance secuencial acordado con el usuario:**
1. Fase 1+2: este tab, funcionando sobre `sdd-skill`, **primero**.
2. Fase 3 (opcional, menor prioridad): que sdd-skill persista qué agentes participaron en una spec, al cerrarla — de forma barata (ver §7).
3. Fase 4: documentar el patrón/contrato para que sea trivial portarlo después a `gamedev-skill` como un tab separado y auto-detectado (ver §8).
4. **NO empezar gamedev-skill todavía.** Es explícitamente lo último, después de validar sdd-skill con uso real.

---

## 2. Decisiones de diseño (por qué se llegó a esto, no solo qué)

Se recorrió una evolución de ideas antes de llegar al diseño actual — importante que el agente que continúe **no reintroduzca** las ideas descartadas:

1. **❌ Épicas como agrupador.** Se evaluó agregar un concepto de "épica" (agrupando varias specs). Se descartó: sdd-skill no tiene ese concepto hoy, y meterlo exigía un campo nuevo o una jerarquía de carpetas nueva — el usuario decidió que era demasiada complejidad para el valor que aportaba.
2. **✅ La spec ES el agrupador.** Pivote del usuario: "la spec es la agrupadora, el detalle que queremos controlar es la tarea". Esto simplificó todo.
3. **❌ 4 columnas (con "In Review").** Se descartó — sdd-skill no persiste un estado "in review" real; el review es un paso transitorio dentro de `in_progress` (vía `state.md:role=@review`).
4. **✅ 3 columnas: Todo / In Progress / Done.** Mapeadas 1:1 a lo que ya existe en `active.json`.
5. **✅ "Propuesta A" — cero escritura.** Se evaluaron alternativas que pedían a los agentes de sdd-skill escribir campos nuevos durante el ciclo (más "ricas" en datos pero con costo en tokens y riesgo). Se eligió la alternativa de **solo lectura**: todo se deriva de archivos que el ciclo ya escribe de todas formas. Esto aplica a columnas, tarea actual, tareas hechas/pendientes, y % de checklist. **Únicamente** el histórico de "agentes involucrados al cerrar" (Fase 3, no implementada) requeriría una escritura mínima adicional — ver §7.

---

## 3. Mapeo exacto de datos (el contrato real)

Esto es lo más importante para cualquiera que edite `spec_board.c`. Todo sale de `<root_path>/.sdd-skill/`:

| Dato mostrado | Archivo fuente | Campo/formato |
|---|---|---|
| Spec activa | `specs/active.json` | `active_spec` (string o `null`) |
| Columna "Todo" | `specs/active.json` | `planned_specs[]` **y** `draft_specs[]` (⚠️ ver §6.1 — hoy solo se lee `planned_specs[]`) |
| Columna "Done" | `specs/active.json` | `completed_specs[]` |
| Columna "In Progress" | — | siempre y solo la spec de `active_spec` (nunca hay más de una — sdd-skill es secuencial) |
| Agente trabajando ahora | `state.md` | ver §3.1 — solo se lee para la spec activa |
| Tarea actual | `state.md` | ver §3.1 |
| Nota de bloqueo | `state.md` | ver §3.1 |
| Título "bonito" de la spec activa | `specs/<id>/spec.md` | primera línea (`# Spec-NNN: [Nombre]`), sin el `#` |
| Lista de tareas de la spec activa | `specs/<id>/tasks.md` | líneas `### Task #N — Nombre` |
| Tarea hecha / pendiente | `history/test_results.log` | líneas que contienen `Task #N` + `PASS` (⚠️ ver limitación en §6.2) |
| % de checklist de la spec activa | `specs/<id>/checklist.md` | fila `TOTAL` dentro de la sección `## Feature Status`, con un número seguido de `%` |

### 3.1 — `state.md` tiene DOS formatos posibles (¡importante!)

Se descubrió en esta sesión, probando contra un proyecto real (`bevy-tetris`), que `state.md` puede estar en:

- **Formato nuevo (compacto, post token-reduction, sdd-skill ≥ 1.4.0):** 2 líneas, `role=@x task="..." spec=... ...` / `notes=...`.
- **Formato viejo (pre-1.4.0, sigue en disco en cualquier proyecto que nadie ha vuelto a tocar desde la migración):** multilínea markdown, `current_role: @x`, `current_task: ...`, con notas de bloqueo como texto libre bajo una sección `## Notes`.

`spec_board.c` **ya soporta ambos** (fix aplicado en esta sesión, en `read_state_md()` — intenta el formato compacto primero, y si algún campo queda vacío, cae al formato viejo). **Esto nunca se probó contra datos reales de una spec REALMENTE activa** (ver §6.3 — el único proyecto de prueba disponible, `bevy-tetris`, está en `IDLE`, sin spec activa, así que este código jamás se ha ejecutado con datos reales todavía).

---

## 4. Archivos tocados / creados (estado exacto)

### Backend (C)

| Archivo | Tipo | Contenido |
|---|---|---|
| `src/ui/spec_board.h` | **nuevo** | Structs (`cbm_spec_task_t`, `cbm_spec_board_entry_t`, `cbm_spec_board_t`) y API pública (`cbm_spec_board_read`, `cbm_spec_board_to_json`, `cbm_spec_board_sdd_skill_present`, `cbm_spec_board_gamedev_skill_present` — este último es un stub, siempre `false`, reservado para cuando se porte a gamedev-skill) |
| `src/ui/spec_board.c` | **nuevo** | Parseo puro, tolerante (archivo faltante → campo vacío, nunca falla duro), cero escritura. Ver §3 para el mapeo exacto de cada función. |
| `src/ui/http_server.c` | **editado** | + `#include "ui/spec_board.h"`; + `resolve_project_root_path()` (helper que copia el patrón de `handle_repo_info` para resolver `root_path` de un proyecto desde el store); + `handle_spec_board()` → `GET /api/spec-board?project=X`; + `handle_skill_presence()` → `GET /api/skill-presence?project=X`; + registro de ambas rutas en `dispatch_request()` |
| `Makefile.cbm` | **editado** | Se agregó `src/ui/spec_board.c` a la variable `UI_SRCS` |

**Endpoints nuevos:**
- `GET /api/spec-board?project=<nombre>` → JSON con `{sdd_skill_present, specs: [...]}` (ver el shape exacto reflejado en `types.ts`, §5)
- `GET /api/skill-presence?project=<nombre>` → `{sdd_skill: bool, gamedev_skill: bool}` — **construido pero no consumido aún por el frontend** (ver §6.4)

### Frontend (graph-ui, React + TS + Vite)

| Archivo | Tipo | Contenido |
|---|---|---|
| `graph-ui/src/lib/types.ts` | **editado** | `TabId` ahora incluye `"specs"`; nuevos tipos `SpecTask`, `SpecColumn`, `SpecBoardEntry`, `SpecBoard` (reflejan 1:1 el JSON de `spec_board.c`) |
| `graph-ui/src/lib/i18n.ts` | **editado** | Label `tabs.specs` (en/zh) + bloque `specBoard` de mensajes (en/zh) |
| `graph-ui/src/hooks/useSpecBoard.ts` | **nuevo** | Hook que hace poll a `/api/spec-board?project=X` cada 4s (mismo patrón que el poll de `ControlTab`) |
| `graph-ui/src/components/SpecBoardTab.tsx` | **nuevo** | UI completa: `ProjectPicker` (si no hay proyecto seleccionado, reutiliza `useProjects`), 3 columnas, `SpecCard` (expandible solo para la spec activa, muestra agente actual / tareas hechas-total / % checklist / nota de bloqueo), `TaskList` (detalle de tareas con punto verde=hecha, pulsante=actual, blanco=pendiente) |
| `graph-ui/src/App.tsx` | **editado** | `TAB_IDS` reordenado a `["specs","graph","stats","control"]`; tab por defecto cambiado de `"stats"` a `"specs"`; wireado el render y la navegación |

---

## 5. Shape exacto del JSON de `/api/spec-board` (contrato frontend↔backend)

```json
{
  "sdd_skill_present": true,
  "specs": [
    {
      "id": "spec-008-eqi-android-playable",
      "title": "Android Playable Build",
      "column": "done",
      "active": false,
      "current_agent": "",
      "blocked_note": "",
      "task_count": 0,
      "tasks_done": 0,
      "checklist_percent": -1.0,
      "tasks": []
    },
    {
      "id": "spec-009-xyz-example",
      "title": "Ejemplo de spec activa",
      "column": "in_progress",
      "active": true,
      "current_agent": "@implementer",
      "blocked_note": "",
      "task_count": 5,
      "tasks_done": 2,
      "checklist_percent": 40.0,
      "tasks": [
        {"number": 1, "name": "Setup", "done": true, "current": false},
        {"number": 2, "name": "Core logic", "done": true, "current": false},
        {"number": 3, "name": "Tests", "done": false, "current": true},
        {"number": 4, "name": "Docs", "done": false, "current": false}
      ]
    }
  ]
}
```

Notas sobre el shape:
- `checklist_percent: -1` significa "no hay dato" (no error) — el frontend lo debe tratar como "sin mostrar", no como 0%.
- Para specs que **no** son la activa (planned/completed), **todos** los campos de detalle (`title`, `task_count`, `tasks`, `current_agent`, etc.) quedan vacíos/cero a propósito — solo se abre y parsea el folder de la spec **activa**, para mantener el costo de lectura mínimo. Esto es intencional, no un bug.

---

## 6. Estado de verificación — QUÉ FALTA PROBAR Y CÓMO

### 6.0 — Build: ✅ verificado
- Backend: `scripts/build.sh --with-ui` compiló limpio (0 errores/warnings bajo `-Werror`) tras limpiar un residuo de build previo con `scripts/clean.sh`.
- Frontend: `npm run build` (`tsc -b && vite build`) compiló limpio, sin errores de TypeScript.

### 6.1 — ✅ FIX: `draft_specs[]` → columna Todo

`read_active_json()` ahora lee `planned_specs` **y** `draft_specs` como `todo` (sdd-skill no documenta columna distinta; ambos son backlog no-activo). Verificado por `tests/test_spec_board.c` (`spec_board_idle_planned_draft_done`, `spec_board_active_compact_state`).

### 6.2 — Limitación conocida y documentada (no es un bug, es un trade-off aceptado)

`history/test_results.log` es un log único, append-only, **compartido por todas las specs que un proyecto ha tenido alguna vez** (no hay separación por spec). `read_test_results()` determina "tarea hecha" buscando `Task #N` + `PASS` y quedándose con la entrada más reciente por número de tarea. Si una spec vieja tuvo un `Task #3` y la spec activa actual también tiene un `Task #3`, en teoría podría "heredar" el estado de la spec vieja. Está documentado como comentario en el código (encabezado de `spec_board.c`). **No se ha visto en la práctica** — evaluar si es un problema real una vez haya datos de uso.

### 6.3 — ✅ Parser verificado con spec activa (unit); ⏳ HTTP E2E manual pendiente

Unit tests (`tests/test_spec_board.c`, suite `spec_board`, **5/5 PASS**) cubren:
- sin `.sdd-skill/`
- idle: planned + draft → todo, completed → done
- activa + `state.md` compacto: agent, tasks, PASS/FAIL, checklist %, current task, JSON shape
- activa + `state.md` legacy + blocker note
- presencia `.gamedev/`

**Fix extra hallado en tests:** `cbm_spec_board_t` no debe vivir en stack (~MB). Caps bajados a 64 specs / 48 tasks; `handle_spec_board` y tests usan `calloc`.

**HTTP E2E** (daemon + `/api/spec-board`) queda como checklist manual del usuario (cache aislado, sin tocar `bevy-tetris`, sin `install`). Ver §10.

### 6.4 — Diferido a propósito (no es un olvido)

- `/api/skill-presence` existe en el backend pero **no está conectado al frontend**. Hoy el tab "Specs" **siempre se muestra**, y si el proyecto no tiene `.sdd-skill/`, el contenido simplemente dice "este proyecto no usa sdd-skill" (mensaje, no tab oculto). Se decidió así para v1 por simplicidad. Si se quiere ocultar el tab de verdad, el endpoint ya está listo — solo falta el `fetch` + condicional en `App.tsx`.
- `tests/test_spec_board.c` — **existe**, wired en `TEST_UI_SRCS` + `suite_spec_board` en `test_main.c`. Correr: `./build/c/test-runner spec_board`.

---

## 7. Fase 3 (NO iniciada) — Histórico de agentes al cerrar spec

Diseño ya acordado con el usuario, pendiente de implementar — **y esto sí toca `sdd-skill`, no `codebase-memory-mcp`**:

- Ubicación real de sdd-skill: `/Users/jmsolorzano/SWE/_SKILLs/sdd-skill/source/sdd-skill/`
- Agregar una key acumulativa `agents=` a `references/templates/state.md` — cada agente, al reescribir `state.md` en su turno (algo que YA hace siempre), se auto-agrega a esa lista si no está (dedupe, orden de aparición). Costo marginal: ~0 tokens.
- En el paso `[8 CLOSE @planner]` de `references/cycle.md`, copiar el valor acumulado de `agents=` a una nueva línea del header de `references/templates/spec.md` (ej. `Agents Involved: @planner,@architect,...`).
- Esto es **independiente** del board de solo-lectura — el board funciona hoy sin esto; solo le faltaría mostrar el histórico de agentes en las tarjetas de "Done".
- **No empezar esto** hasta que la Fase 1/2 (este documento) esté 100% verificada y validada por el usuario en uso real.

---

## 8. Fase 4 (NO iniciada) — Documentación del contrato, para portar a gamedev-skill

- Crear `codebase-memory-mcp/docs/kanban-adapter-contract.md`: contrato genérico, agnóstico de skill — qué campos mínimos necesita cualquier skill para alimentar un board tipo Kanban (id, título, columna, tipo de item, agente actual, etc.), sin asumir nombres de archivo específicos de sdd-skill.
- Crear `sdd-skill/references/kanban-adapter.md`: el mapeo específico de sdd-skill a ese contrato (exactamente la tabla de la §3 de este documento, pero viviendo en el repo de la skill).
- Cuando llegue el turno de gamedev-skill: mismo patrón, pero mapeando su propia estructura (`.gamedev/`, Track A/B, `level-folder/changelog.md`, etc. — estructura distinta a sdd-skill, **no asumir que el mismo parser sirve**). Vivirá como un **tab separado**, auto-detectado vía `cbm_spec_board_gamedev_skill_present()` (ya stubbed, retorna `false` siempre por ahora) — un proyecto podría tener AMBAS skills a la vez y por tanto ambos tabs.

---

## 9. ⚠️ El obstáculo que bloqueó la verificación en esta sesión (leer antes de repetir el error)

**Esto no es un bug de código — es un conflicto de despliegue**, y quien continúe debe resolverlo primero o va a perder tiempo pensando que el código está roto:

- El Cursor de Juan (`~/.cursor/mcp.json`) lanza `codebase-memory-mcp` desde **`/Users/jmsolorzano/.local/bin/codebase-memory-mcp`** — un binario **instalado**, completamente separado del que se compila en este repo (`build/c/codebase-memory-mcp`).
- Mientras Cursor está abierto, ese binario viejo (sin los endpoints nuevos) levanta su propio daemon "session-managed" en el puerto 9749. Confirmado con `curl`: tanto `/api/spec-board` como `/api/skill-presence` devuelven `404 not found` cuando ese es el proceso que responde — el body literal `not found` es el 404 genérico de ruta no reconocida, prueba de que el binario activo no tiene el código nuevo, no de un crash.
- Correr el binario recién compilado (`build/c/codebase-memory-mcp daemon start --open`) mientras Cursor sigue abierto genera choque de puerto/"generación" (`daemon: already active (session-managed, pid ...) ... error: the active daemon generation could not be authenticated`).

**Procedimiento correcto para probar (en orden, sin saltarse pasos):**
1. Cerrar Cursor por completo (esto mata el daemon session-managed — su propio mensaje dice "it stops with its last session").
2. `cd /Users/jmsolorzano/SWE/public-repositories/codebase-memory-mcp && build/c/codebase-memory-mcp daemon start --open`
3. Verificar en el navegador que el tab Specs aparece y funciona, usando el fixture/proyecto de prueba de §6.3.
4. **Solo cuando esté validado:** instalar de verdad con el instalador transaccional propio del binario (evita copiar archivos a mano mientras hay sesiones activas):
   ```bash
   build/c/codebase-memory-mcp install -y --force --dir="$HOME/.local/bin"
   ```
5. Recién ahí reabrir Cursor — el daemon que lance ya será la versión nueva.

**No ejecutar el paso 4 sin haber confirmado el paso 3** — desplegaría una build no verificada al binario del que dependen Cursor y los agentes reales de Juan.

**Nota aparte, no relacionada al bug:** en la lista de proyectos indexados aparece `bevy-tetris` registrado **dos veces** bajo nombres distintos (`bevy-tetris` y `Users-jmsolorzano-SWE-bevy-tetris`), ambos apuntando al mismo `root_path`. No se investigó si es un problema real de duplicación en el store o solo dos registros legítimos con distinta convención de nombre — vale la pena revisarlo si genera confusión al elegir el proyecto en el picker.

---

## 10. TL;DR — estado al cerrar esta pasada

Hecho en código (sin tocar skills):
1. `draft_specs[]` → Todo.
2. Board en heap + caps 64/48 (evita stack overflow ASan).
3. `tests/test_spec_board.c` + wiring Makefile/`test_main.c` — **5/5 PASS**.
4. Doc de estatus actualizado.

Pendiente **manual del usuario** (no `install` hasta validar):
1. Conflicto binarios §9 si quieres UI en Cursor.
2. HTTP E2E con fixture aislado (no tocar `bevy-tetris/.sdd-skill`).
3. Solo después: `build/c/codebase-memory-mcp install -y --force --dir="$HOME/.local/bin"`.

Fase 3/4 / gamedev: **no iniciadas**.
