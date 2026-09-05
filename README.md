# codebase-kanban-mcp

[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Languages](https://img.shields.io/badge/languages-158-orange)](https://github.com/JMSolorzano-13/codebase-kanban-mcp)
[![Hybrid LSP](https://img.shields.io/badge/Hybrid_LSP-10_languages-blue)](#hybrid-lsp)
[![Agents](https://img.shields.io/badge/agent_surfaces-43-purple)](https://github.com/JMSolorzano-13/codebase-kanban-mcp)
[![Pure C](https://img.shields.io/badge/pure_C-no_language_runtime-blue)](https://github.com/JMSolorzano-13/codebase-kanban-mcp)
[![Platform](https://img.shields.io/badge/macOS_%7C_Linux_%7C_Windows-supported-lightgrey)](https://github.com/JMSolorzano-13/codebase-kanban-mcp)
[![arXiv](https://img.shields.io/badge/arXiv-2603.27277-b31b1b?logo=arxiv)](https://arxiv.org/abs/2603.27277)

Fork of [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp) with a local operator UI and Specs/Game kanban. Source of truth: [github.com/JMSolorzano-13/codebase-kanban-mcp](https://github.com/JMSolorzano-13/codebase-kanban-mcp).

Local code intelligence for agents **and** a local operator UI. Full-indexes an average repository in milliseconds, the Linux kernel (28M LOC, 75K files) in 3 minutes. Answers structural queries in under 1ms. Ships as a native C executable — clone this repository, `scripts/build.sh --with-ui`, `install`, open `http://localhost:9749`. The command name remains `codebase-memory-mcp` (same as upstream).

The engine builds a persistent knowledge graph of functions, classes, call chains, HTTP routes, and cross-service links through [tree-sitter](https://tree-sitter.github.io/tree-sitter/) across 158 languages, plus [**Hybrid LSP**](#hybrid-lsp) type resolution for Python, TypeScript / JavaScript / JSX / TSX, PHP, C#, Go, C, C++, Java, Kotlin, Rust, and Perl. 16 MCP tools. No hosted service or API key. 43 automatic/conditional agent surfaces.

The same binary serves an executive workspace at `localhost:9749`: a Dashboard of indexed folders, a 3D Graph, optional Specs / Game boards that **read** skill trees on disk, and an ADR tab filled on reindex. CBM never writes `.sdd-skill/`, `.grill/`, or `.gamedev/`.

> **Research** — The design and benchmarks behind the engine are described in the preprint [*Codebase-Memory: Tree-Sitter-Based Knowledge Graphs for LLM Code Exploration via MCP*](https://arxiv.org/abs/2603.27277) (arXiv:2603.27277). Evaluated across 31 real-world repositories: 83% answer quality, 10× fewer tokens, 2.1× fewer tool calls vs. file-by-file exploration.

> **Security & Trust** — This tool reads your codebase and writes to your agent configuration files. That is what it is designed to do. Audit [this repository](https://github.com/JMSolorzano-13/codebase-kanban-mcp) before running. All processing happens 100% locally; your code never leaves your machine. Signed release archives, VirusTotal scans, and SLSA provenance are published by the [original project](https://github.com/DeusData/codebase-memory-mcp), not by this fork. Found a security issue in this tree? Open an issue here — see [SECURITY.md](SECURITY.md).

<p align="center">
  <img src="docs/graph-ui-screenshot.png" alt="3D knowledge graph in the project workspace at localhost:9749" width="800">
  <br>
  <em>Project workspace — Graph tab of the knowledge graph at localhost:9749. Home is the Dashboard; Specs or Game appear when the repo has those skill folders.</em>
</p>

## Contents

- [Scope](#scope)
- [Install from this repository](#install-from-this-repository)
- [Use the operator UI](#use-the-operator-ui)
- [Use with an agent](#use-with-an-agent)
- [Upstream engine binaries](#upstream-engine-binaries)
- [Features](#features)
- [Installation](#installation)
- [MCP tools](#mcp-tools)
- [Configuration](#configuration)
- [Fork and original project](#fork-and-original-project)

## Scope

Two surfaces, one process: a C daemon that indexes code into SQLite, plus an embedded React UI.

| Surface | What it is | Who uses it |
|---------|------------|-------------|
| MCP / CLI | 16 tools: index, search, trace, Cypher, coverage, ADR, … | Coding agents and scripts |
| Operator UI (`localhost:9749`) | Dashboard → project workspace (Graph, Specs **or** Game, ADR) | You, in the browser |

**In this tree**

- Index any folder into a local graph. Path and project are 1:1: indexing an already-owned path does not mint a second name.
- Dashboard: indexed projects + Control (CPU/RAM/logs) on one screen. Last indexed uses the browser’s local clock (stored ISO is unchanged).
- Workspace tabs: Graph always. Specs if the repo has `.sdd-skill/` or `.grill/` and it is not a Game path. Game if the repo has `.gamedev/` (Specs is then omitted). ADR always.
- Specs Kanban: Todo / In progress / Done. Todo mixes unconverted grill epics (letter E, full path) then planned/draft specs. Click a spec title to expand (blurb + tasks). Archive/unarchive Done specs into CBM-owned state. Open items from `.sdd-skill/baseline/TECH_DEBT.md` sit above the columns.
- Game board: Inbox + Pre-production + Production + Post-production & Launch. Cards are artifacts that exist on disk. Copy `/gamedev-skill continue` / `continue @role`. Expand in place. Archive done artifacts in CBM. Blocked strip from `state.md`. Open `debt:*` rows from `.gamedev/backlog.md`. Inbox hide uses `.gamedev/epics_registry.md` when that file exists; otherwise Companion-to / roadmap.
- ADR: user-triggered index (Dashboard Reindex, first create, or `index_repository`) fills generated Purpose/Stack/Decisions from the gamedev trio if `.gamedev/` is a directory, otherwise the sdd-skill trio. Hand-written notes in the manual region are kept. Watcher jobs do not fill.
- Skills are optional. A repo with none of those folders still indexes and still has Graph + ADR.

**Out of scope (by design)**

- CBM does not write, move, or rename files under `.sdd-skill/`, `.grill/`, or `.gamedev/`. It never creates `epics_registry.md` or `backlog.md`.
- No drag-and-drop on Specs or Game. No button that starts a skill. Game is a map, not a launcher.
- Specs and Game are never both shown for the same path (Game wins).
- The 3D graph color palette is not grayscale; only chrome (Dashboard, tabs, buttons) is dark gray.
- ADR is not filled by an LLM and not filled by the background watcher.

## Why this fork

- **Extreme indexing speed** — Linux kernel (28M LOC, 75K files) in 3 minutes. RAM-first pipeline: LZ4 compression, in-memory SQLite, fused Aho-Corasick pattern matching. Memory released after indexing.
- **Clone and run** — C compiler + zlib; Node.js 22+ only if you want the operator UI. No Docker, no API keys. `scripts/build.sh --with-ui` → `install` → `--ui=true`.
- **158 languages** — vendored tree-sitter grammars compiled into the binary. Nothing extra to install.
- **120x fewer tokens** — 5 structural queries: ~3,400 tokens vs ~412,000 via file-by-file search. One graph query replaces dozens of grep/read cycles.
- **43 supported automatic/conditional client surfaces** — `install` configures detected clients. See [Multi-Agent Support](#multi-agent-support).
- **Operator workspace** — Dashboard, 3D Graph, Specs (sdd-skill / grill-skill), Game (gamedev-skill), ADR. Skill folders are read-only.
- **Infrastructure-as-code indexing** — Dockerfiles, Kubernetes manifests, and Kustomize overlays as graph nodes with cross-references.
- **16 MCP tools** — search, trace, architecture, impact analysis, targeted index-coverage checks, Cypher, dead code, cross-service HTTP linking, ADR, and more.

## Install from this repository

This fork has no published release binaries. Clone this repository and build. Package-manager one-liners (`npm`, `pip`, Homebrew, AUR, …) still install the **upstream** engine from [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp) and do **not** include this kanban UI.

### Prerequisites

| Requirement | macOS | Linux (Debian/Ubuntu) |
|-------------|-------|------------------------|
| C/C++ compiler | `xcode-select --install` | `sudo apt install build-essential` |
| zlib | included | `sudo apt install zlib1g-dev` |
| Git | included | `sudo apt install git` |
| Node.js 22+ | [nodejs.org](https://nodejs.org/) or `brew install node` | needed only for `--with-ui` |

### Build and run (macOS / Linux)

```bash
git clone https://github.com/JMSolorzano-13/codebase-kanban-mcp.git
cd codebase-kanban-mcp
scripts/build.sh --with-ui
```

Binary: `build/c/codebase-memory-mcp`.

```bash
# Wire detected coding agents (Claude Code, Cursor, …) and copy onto PATH
./build/c/codebase-memory-mcp install

# Persist UI on and start the daemon (default port 9749)
./build/c/codebase-memory-mcp --ui=true --port=9749
```

If `install` put the binary in `~/.local/bin` and that directory is not on PATH:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

Open [http://localhost:9749](http://localhost:9749). You should see the Dashboard (empty until you index a folder).

`install` options: `--skip-config` (binary only, no agent setup), `--dir=<path>` (custom location).

Restart your coding agent after `install`. From the agent: **"Index this project"** — or index from the Dashboard (below).

### Windows

Build from this clone (C compiler + Node.js 22+; see `CONTRIBUTING.md`):

```powershell
git clone https://github.com/JMSolorzano-13/codebase-kanban-mcp.git
cd codebase-kanban-mcp
```

Then follow the same `scripts/build.sh --with-ui` / `install` / `--ui=true` flow as macOS/Linux if you have a Unix environment (WSL is the straightforward path). The upstream Windows zip installs the original engine without this fork’s kanban UI — see [Upstream engine binaries](#upstream-engine-binaries).

> **Antivirus note:** Microsoft Defender may flag a native CBM binary as
> `Trojan:Script/Wacatac.B!ml`. This is a known false positive on the upstream
> family. See [Antivirus False Positives](SECURITY.md#antivirus-false-positives).

### Verify

```bash
codebase-memory-mcp --version
codebase-memory-mcp cli list_projects
# echo '{}' | codebase-memory-mcp   → JSON on stdout (MCP stdio handshake)
```

UI not loading: confirm `--ui=true` and [http://localhost:9749](http://localhost:9749). Tabs missing Specs/Game: rebuild with `--with-ui` (a binary built without that flag has no operator boards).

## Use the operator UI

All of this is local HTTP on the loopback port (default **9749**). The UI is owned by the shared coordination daemon, so concurrent agent sessions do not start duplicate HTTP servers.

```bash
codebase-memory-mcp --ui=true --port=9749
```

`--ui` and `--port` persist. Later sessions reuse those values until you change them.

### Dashboard (account home)

Open `http://localhost:9749` (or `?tab=dashboard`). Old bookmarks `?tab=stats` / `?tab=control` land here too.

1. **New Index** — browse to a repository root, **Index This Folder**. The project name is derived from the path. There is no Project ID field.
2. Each row: name, path, last indexed (local timezone), health, **Enter**, **Reindex**, delete (confirm).
3. **Control** is on the same page: CPU/RAM, processes, logs.
4. Indexing a folder that is already owned does not create a clone: you are sent to that project’s Graph with a notice.
5. Leftover aliases from before Path 1:1 show as a **Path conflict**: Enter the newest; delete older only after confirm.
6. **Reindex** on a row refreshes that same name and stays on the Dashboard. That job (and MCP `index_repository`) can fill the ADR generated region.

### Project workspace

**Enter** opens Graph. Header: back to Dashboard, project name, last indexed.

| Tab | When it appears | What you do |
|-----|-----------------|-------------|
| Graph | Always | 3D knowledge graph. Colors of nodes/edges are unchanged. |
| Specs | `.sdd-skill/` **or** `.grill/` is present, and Game is not shown | Kanban of specs + leftover grill epics. |
| Game | `.gamedev/` is a directory | Four-column phase board. Specs is hidden. |
| ADR | Always | Generated excerpts + a manual region you can edit. |

URL: `?tab=graph|specs|game|adr&project=<name>`. A leftover `?tab=specs` on a gamedev path becomes Game.

### Specs board

Three columns: Todo, In progress, Done. CBM **reads** `.sdd-skill/` and `.grill/`; it does not move specs.

- **Todo** lists unconverted grill epics first (mark **E**, title, summary, plan, wrapping path), then planned/draft specs. An epic leaves Todo when sdd-skill creates that spec (`Companion to:` / `source.grill_epic` exact path). Epic cards do not expand or archive.
- Click a **spec title** to expand in place (several cards may stay open). Blurb is 1–2 sentences from `## Executive Summary` when present. Todo shows pending tasks only; In progress shows all with status; Done shows the full list.
- **Archive** / **Unarchive** on an expanded Done spec. Flags live in CBM’s project database, not in the skill tree. Archived cards stay in Done, hidden until **Show archived** (session only; defaults off each visit). No confirm dialog.
- **Open tech debt** (chrome above the columns) lists open items from `.sdd-skill/baseline/TECH_DEBT.md` when any exist. Dead text: no click action. Specs does not read gamedev’s registry or backlog.

### Game board

Map of an existing `.gamedev/` tree. CBM does not launch gamedev-skill.

- Columns always visible: **Inbox** | **Pre-production** | **Production** | **Post-production & Launch**. Empty column = header only.
- Artifact cards: track (A/B/H), work-state, owner. Title expands: Track A gets blurb + tasks + Inputs; Track B gets the header. Inbox cards are leftover grill epics (letter E).
- Copy **`/gamedev-skill continue @role`** from a card (clipboard, or select-text fallback). Chrome continue is selectable text, not a launch button.
- **Show Dones** (off by default) reveals done artifacts. **Track A / B / All** filters phase columns only (All includes H). Inbox is never filtered. Filters are UI state: changing project or remounting resets them; a refetch after Archive does not.
- **Archive** a done artifact the same way as Specs (CBM `game_archive`, not skill files). An archived done card needs **Show Dones** and **Show archived** both on.
- **Blocked** strip: live `blocked-by` lines from `state.md`.
- **Open tech debt** after Blocked: open `debt:*` rows from `.gamedev/backlog.md` (closed when the entry contains `resolved-by`).
- Inbox hide: if `.gamedev/epics_registry.md` is a regular file, that table is the only hide rule (`in_progress` / `closed` / `parked`). If the file is absent, conversion is Companion-to exact path or roadmap slug + NNN. CBM never creates the registry.

### ADR tab

On a **user-triggered** index only:

- If `{repo}/.gamedev/` is a directory → fill from `.gamedev/game_context.md`, `.gamedev/baseline/TECH_STACK.md`, `.gamedev/baseline/ARCHITECTURE_ADR.md`.
- Else if `{repo}/.sdd-skill/` is a directory → fill from `.sdd-skill/context_ai.md`, `.sdd-skill/baseline/TECH_STACK.md`, `.sdd-skill/baseline/ARCHITECTURE_ADR.md`.
- Neither directory → existing ADR blob unchanged.

Generated blocks are replaced on the next user-triggered index. The manual region is not. Agents can still read/write the whole document via `manage_adr`. There is no MCP archive tool.

## Use with an agent

After `install` and a client restart, the agent talks MCP stdio to the same daemon.

```
You: Index this project
You: What calls ProcessOrder?
```

The agent should call tools such as `index_repository`, `list_projects`, `search_graph`, `trace_path`, `check_index_coverage`. There is no LLM inside CBM: the client you already use translates questions into those tools.

```bash
# Same tools, one-shot, no standing daemon
codebase-memory-mcp cli list_projects
codebase-memory-mcp cli index_repository --repo-path /absolute/path/to/repo
codebase-memory-mcp cli search_graph --project my-project --name-pattern '.*Handler.*' --label Function
```

Enable auto-index on MCP session start:

```bash
codebase-memory-mcp config set auto_index true
```

Previously indexed projects then register with the background watcher (`auto_watch` defaults true). `config set auto_watch false` keeps a session from attaching the watcher.

`index_repository` without `name` on a folder that already has one owner is a reindex of that project, not a second identity.

## Upstream engine binaries

This fork is installed from source. The commands below download **DeusData/codebase-memory-mcp** releases: the original graph engine, **without** this repository’s Dashboard / Specs / Game kanban. Use them only if you want upstream, not this fork.

```bash
curl -fsSL https://raw.githubusercontent.com/DeusData/codebase-memory-mcp/main/install.sh | bash
```

Then `codebase-memory-mcp --ui=true --port=9749` if that upstream release embeds a UI (it will not be this fork’s kanban boards).

<details>
<summary>Manual install</summary>

1. **Download** the archive for your platform from the [latest release](https://github.com/DeusData/codebase-memory-mcp/releases/latest):
   - `codebase-memory-mcp-<os>-<arch>.tar.gz` (macOS/Linux) or `.zip` (Windows)

2. **Extract and install** (each archive includes `install.sh` or `install.ps1`):

   macOS / Linux:
   ```bash
   tar xzf codebase-memory-mcp-*.tar.gz
   ./install.sh
   ```

   Windows (PowerShell):
   ```powershell
   Expand-Archive codebase-memory-mcp-windows-amd64.zip -DestinationPath .
   Unblock-File .\install.ps1
   .\install.ps1
   ```

3. **Restart** your coding agent.

The `install` command automatically strips macOS quarantine attributes and ad-hoc signs the binary — no manual `xattr`/`codesign` needed.
</details>

The `install` command auto-detects installed coding agents and configures their documented MCP entries plus durable instructions, skills, and lifecycle hooks where supported.

### Session Coordination Daemon

CBM automatically shares one per-account coordination daemon across Claude Code, Codex, OpenCode, and every other configured client. There is no opt-in setting for MCP servers or hook clients: the first daemon-backed CBM session starts it, each session registers its own work, and the final session shuts it down. The daemon owns long-lived background services such as watchers, shared indexing jobs, and the optional UI. Closing one session cancels work owned only by that session, while work still needed by another session continues.

The detached daemon does not depend on an MCP frontend's stderr. It keeps owner-only durable records under the canonical `${CBM_CACHE_DIR}/logs` directory (default `~/.cache/codebase-memory-mcp/logs`):

| File | Contents |
|------|----------|
| `cbm-daemon.log` | Daemon lifecycle, watcher/indexing, UI, resource, and error events. |
| `daemon-conflicts.ndjson` | Exact-build, coordination-ABI, and cache-root admission conflicts. |
| `activation-events.ndjson` | Install/update/uninstall activation progress and outcomes. |

Thin frontends still write immediate startup and session-specific errors to their own stderr; MCP JSON-RPC stdout remains clean.

All active CBM processes must run the exact same version, executable build, coordination ABI, and canonical cache root. Equivalent `CBM_CACHE_DIR` aliases resolve to the same root; a genuinely different root is rejected while any CBM process is active. MCP servers, hooks, one-shot CLI commands, temporary index workers, and the daemon share a crash-safe OS admission barrier; starting an ordinary conflicting process fails before doing work and records an explicit conflict in `${CBM_CACHE_DIR}/logs/daemon-conflicts.ndjson`.

The native `install`, `update`, and `uninstall` commands are the deliberate exception to that conflict rule. Download, verification, and private same-filesystem staging happen first so a bad candidate never disrupts active work. Activation then publishes account-wide maintenance intent, asks the daemon and every temporary local operation to cancel, and waits to a finite deadline for all coordinated CBM processes to exit. It holds the admission and lifetime barriers exclusively while changing the active binary, configuration, PATH, or indexes. New CBM work cannot enter during this window. Activation progress and results are recorded in `${CBM_CACHE_DIR}/logs/activation-events.ndjson`, and a successful command tells you to restart open coding-agent sessions so they launch the activated build.

Package-manager setup (npm, PyPI, or Go) verifies and publishes a coherent private cached runtime set. Sidecars are replaced before the executable with per-file atomic renames; an interrupted multi-file publication is detected and repaired on the next launch rather than being described as one crash-atomic filesystem transaction. It does not replace the active native installation and therefore does not stop running CBM sessions. When that cached binary is executed, it still enters the same exact-build admission barrier. The shell and PowerShell installers invoke the verified candidate's native `install` command, so they do receive the full account-wide activation guarantee.

The ordinary `cli` mode is intentionally separate: it runs one command locally and never starts or connects to the coordination daemon, registers a daemon session, or starts watchers/UI. Its only shared state is the OS admission barrier plus per-project locks for graph mutations. While the command is running, a temporary monitor lets activation cancel that operation and its supervised worker safely; the monitor exits with the command and never becomes a standing daemon. See [CLI Mode](#cli-mode) for details.

### Auto-Index

Enable automatic indexing on MCP session start:

```bash
codebase-memory-mcp config set auto_index true
```

When enabled, new projects are indexed automatically on first connection. Previously-indexed projects are registered with the background watcher for ongoing git-based change detection. Configurable file limit: `config set auto_index_limit 50000`.

Watcher registration is controlled separately by `auto_watch` (default `true`). Set `config set auto_watch false` to keep a session from registering its project with the background watcher — useful when working across many projects and you want each session contained to explicit indexing.

### Keeping Up to Date

**This fork:** `git pull` in your clone of [codebase-kanban-mcp](https://github.com/JMSolorzano-13/codebase-kanban-mcp), rebuild with `scripts/build.sh --with-ui`, then run `./build/c/codebase-memory-mcp install` so agents pick up the new binary.

`codebase-memory-mcp update` prints an install-script command. That script downloads **upstream** [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp) releases, not this fork. Use it only if you intend to replace this build with the original engine.

**Updates of an upstream install** run from the install script on every platform, not from inside the running binary. `codebase-memory-mcp update` validates your flags and then prints the exact command to run:

```bash
# macOS / Linux
bash "<install-dir>/install.sh"
```

```powershell
# Windows
powershell -ExecutionPolicy Bypass -File "<install-dir>\install.ps1"
```

The install script is placed next to the binary at install time, so the printed path resolves beside the executable. It is idempotent, so re-running it *is* the update: it stops the daemon, retires the running binary, installs the new one, and cleans up.

Why it works this way. On Windows it is a hard requirement — a running executable cannot replace its own image, so the swap has to happen from a process that is not the binary being replaced. On macOS and Linux it is a deliberate choice: an in-process updater is structurally a downloader (fetch an archive, verify it, unpack it, mark a file executable, run it), and shipping that composite in every binary to serve a command most people run a handful of times is a poor trade. The release archives now carry no download URLs at all, and **cbm makes no network request of its own accord** — it does not check for new versions in the background, and nothing phones home. You find out about releases from the install script, your package manager, or GitHub.

If PowerShell refuses to run the script because the file came from the internet, `Unblock-File` it first.

Installed through **npm or pip** (upstream packages)? Update with your package manager (`npm install -g codebase-memory-mcp@latest` / `pip install -U codebase-memory-mcp`). Those packages are not this fork.

### Uninstall

```bash
codebase-memory-mcp uninstall
```

Removes owned agent config entries, skills, hooks, instructions, and the installed binary. Existing graph indexes are listed and deleted only after confirmation.

The install script placed beside the binary is **reported, not deleted** — uninstall prints its path and the `rm` command for it. It is left alone on purpose: it may be your own copy, a symlink into a checkout, or managed by a package manager, and an uninstaller should not delete a file it cannot prove it owns.

## Features

### Operator workspace
- **Dashboard** — indexed folders + Control on one screen; Path 1:1; Reindex stays home
- **Graph** — 3D constellation (node/edge colors unchanged)
- **Specs** — Kanban when `.sdd-skill/` or `.grill/` is present; mixed Todo (grill epics then specs); expand; CBM archive; open TECH_DEBT.md
- **Game** — four phase columns when `.gamedev/` is present (Specs hidden); Inbox; copy continue; expand; CBM archive; blocked strip; backlog debt; registry hide
- **ADR** — generated region filled on user-triggered index (gamedev trio XOR sdd trio); manual region kept
- Skill trees are read-only. See [Use the operator UI](#use-the-operator-ui).

### Graph & analysis
- **Architecture overview**: `get_architecture` returns languages, packages, entry points, routes, hotspots, boundaries, layers, and clusters in a single call
- **Architecture Decision Records**: `manage_adr` persists architectural decisions across sessions
- **Louvain community detection**: Discovers functional modules by clustering call edges
- **Git diff impact mapping**: `detect_changes` maps uncommitted changes to affected symbols with risk classification
- **Call graph**: Resolves function calls across files and packages (import-aware, type-inferred)
- **Dead code detection**: Finds functions with zero callers, excluding entry points
- **Cypher-like queries**: `MATCH (f:Function)-[:CALLS]->(g) WHERE f.name = 'main' RETURN g.name`

### Search
- **Semantic search** (`semantic_query`): vector search across the entire graph, powered by bundled Nomic `nomic-embed-code` embeddings (40K tokens, 768d int8) compiled into the binary — no API key, no Ollama, no Docker. 11-signal combined scoring (TF-IDF, RRI, API/Type/Decorator signatures, AST profiles, data flow, Halstead-lite, MinHash, module proximity, graph diffusion).
- **BM25 full-text search** via SQLite FTS5 with `cbm_camel_split` tokenizer (camelCase / snake_case aware)
- **Structural search** (`search_graph`): regex name patterns, label filters, min/max degree, file scoping
- **Code search** (`search_code`): graph-augmented grep over indexed files only

### Cross-service linking
- **HTTP** route ↔ call-site matching with confidence scoring
- **gRPC, GraphQL, tRPC** service detection with protobuf Route extraction
- **Channel detection** (`EMITS` / `LISTENS_ON`) for Socket.IO, EventEmitter, and generic pub-sub patterns across 8 languages with constant resolution

### Cross-repo intelligence
- **`CROSS_*` edges** link nodes across multiple repos indexed under the same store
- **Multi-galaxy 3D UI layout** for cross-repo architecture visualization
- **Cross-repo architecture summary** combining services, routes, and dependencies across the indexed fleet

### Edge types (selected)
- `CALLS` — a callable is invoked at the source site
- `CALL_REFERENCE` — a callable is used at a supported reference site (for example, a direct value argument) and resolves to one exact target
- `USAGE` — an identifier is used, but a unique callable target is not proven (including ambiguous or complex expressions)
- `IMPORTS`, `DEFINES`, `IMPLEMENTS`, `INHERITS`
- `HTTP_CALLS`, `ASYNC_CALLS` (cross-service)
- `EMITS`, `LISTENS_ON` (channels)
- `DATA_FLOWS` with arg-to-param mapping + field access chains
- `SIMILAR_TO` (MinHash + LSH near-clone detection, Jaccard scored)
- `SEMANTICALLY_RELATED` (vocabulary-mismatch, same-language, score ≥ 0.80)

### Indexing pipeline
- **158 vendored tree-sitter grammars** compiled into the binary
- **Generic package / module resolution** — bare specifiers like `@myorg/pkg`, `github.com/foo/bar`, `use my_crate::foo` resolved via manifest scanning (`package.json`, `go.mod`, `Cargo.toml`, `pyproject.toml`, `composer.json`, `pubspec.yaml`, `pom.xml`, `build.gradle`, `mix.exs`, `*.gemspec`)
- **Infrastructure-as-code indexing** — Dockerfiles, Kubernetes manifests, Kustomize overlays as graph nodes
- **[Hybrid LSP semantic type resolution](#hybrid-lsp)** for Python, TypeScript / JavaScript / JSX / TSX, PHP, C#, Go, C, C++, Java, Kotlin, Rust, and Perl — a lightweight C implementation of language type-resolution algorithms, structurally inspired by and compatible with major language servers including tsserver / typescript-go, pyright, gopls, Roslyn, Eclipse JDT, and rust-analyzer (parameter binding, return-type inference, generic substitution, JSX component dispatch, JSDoc inference for plain JS files, namespace + trait + late-static-binding resolution for PHP, file-scoped namespaces + records + LINQ method syntax for C#, class-hierarchy + overload + lambda resolution for Java, extension-function + scope-function resolution for Kotlin, trait-method + UFCS resolution for Rust)
- **RAM-first pipeline**: LZ4 compression, in-memory SQLite, single dump at end. Memory released after.

### Distribution & operation
- **Native runtime set, zero infrastructure services**: SQLite-backed, persists to `~/.cache/codebase-memory-mcp/`
- **Auto-sync**: Background watcher detects file changes and re-indexes automatically
- **Route nodes**: REST endpoints are first-class graph entities
- **CLI mode**: `codebase-memory-mcp cli search_graph '{"project": "my-project", "name_pattern": ".*Handler.*"}'`
- **Operator UI**: `codebase-memory-mcp --ui=true --port=9749` → `http://localhost:9749`
- **This fork**: clone [JMSolorzano-13/codebase-kanban-mcp](https://github.com/JMSolorzano-13/codebase-kanban-mcp) and `scripts/build.sh --with-ui`
- **Upstream packages** (engine only, no this kanban UI): npm, PyPI, Homebrew, Scoop, Winget, Chocolatey, AUR, `go install` from [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp)

## Team-Shared Graph Artifact

Commit a single compressed file to your repo and your teammates skip the reindex.

`.codebase-memory/graph.db.zst` is a zstd-compressed snapshot of the knowledge graph that lives next to your source. When you index, the artifact is written or refreshed; when a teammate clones the repo and runs `codebase-memory-mcp` for the first time, the artifact is decompressed and incremental indexing fills in their local diff.

- **Format**: SQLite database, indexes stripped, `VACUUM INTO` compacted, then zstd 1.5.7 compressed (8–13:1 ratio typical)
- **Two tiers**:
  - **Best** (`zstd -9` + index strip + `VACUUM INTO`) — written on explicit `index_repository`
  - **Fast** (`zstd -3`) — written by the watcher for low-latency incremental updates
- **Bootstrap**: when no local DB exists but the artifact is present, `index_repository` imports the artifact first, then runs incremental indexing — avoiding the full reindex cost
- **No merge pain**: a `.gitattributes` line with `merge=ours` is auto-created on first export, so concurrent edits don't produce conflicts on the binary artifact
- **Optional**: never committed unless you want it. Add `.codebase-memory/` to `.gitignore` if you prefer everyone to reindex from scratch.

The result is similar in spirit to graphify's `graphify-out/` directory, but as a single compressed file with explicit two-tier export, integrity-checked import, and zero merge friction.

## How It Works

codebase-memory-mcp is a **structural analysis backend** — it builds and queries the knowledge graph. It does **not** include an LLM. Your MCP client is the intelligence layer for questions. The operator UI is a second client of the same daemon: it lists projects, shows the graph, and **reads** skill folders on disk.

```
You: "what calls ProcessOrder?"

Agent calls: trace_path(function_name="ProcessOrder", direction="inbound")

codebase-memory-mcp: executes graph query, returns structured results

Agent: presents the call chain in plain English
```

The browser at `localhost:9749` talks HTTP to the same process (Dashboard, Graph, Specs/Game, ADR). Archive flags live in the project SQLite file. Skill trees stay owned by sdd-skill, grill-skill, and gamedev-skill.

**Why no built-in LLM?** Other code graph tools embed an LLM for natural language → graph query translation. This means extra API keys, extra cost, and another model to configure. With MCP, the agent you're already talking to *is* the query translator. ADR fill on reindex is a bounded file parse, not a model.

## Performance

Benchmarked on Apple M3 Pro:

| Operation | Time | Notes |
|-----------|------|-------|
| **Linux kernel full index** | **3 min** | 28M LOC, 75K files → 4.81M nodes, 7.72M edges |
| Linux kernel fast index | 1m 12s | 1.88M nodes |
| Django full index | ~6s | 49K nodes, 196K edges |
| Cypher query | <1ms | Relationship traversal |
| Name search (regex) | <10ms | SQL LIKE pre-filtering |
| Dead code detection | ~150ms | Full graph scan with degree filtering |
| Trace call path (depth=5) | <10ms | BFS traversal |

**RAM-first pipeline**: All indexing runs in memory (LZ4 HC compressed read, in-memory SQLite, single dump at end). Memory is released back to the OS after indexing completes.

**Token efficiency**: Five structural queries consumed ~3,400 tokens via codebase-memory-mcp versus ~412,000 tokens via file-by-file grep exploration — a **99.2% reduction**.

## Troubleshooting & Diagnostics

codebase-memory-mcp runs **100% locally and collects no telemetry** — your code, queries, environment, and usage never leave your machine. That privacy guarantee also means that when you hit something we can't reproduce on our side (a slow memory climb over hours, a performance regression, a leak that only appears after days of real use), **we have no data at all unless you choose to send it.** Here is how to capture it yourself.

### Capture a diagnostics log

Set `CBM_DIAGNOSTICS=1` before the first daemon-backed MCP session starts, then reproduce the problem (let it run as long as it takes — a slow leak needs time to show in the trend). The shared daemon captures this setting from the session that starts it. If it is already running, close all daemon-backed sessions so it exits before changing the setting. The daemon creates a fresh owner-private `cbm-diagnostics-<pid>-<random>` directory below the system temp directory (`$TMPDIR` or `/tmp` on macOS/Linux, `%TEMP%` on Windows). The exact paths are recorded by the `diagnostics.start` event in `${CBM_CACHE_DIR}/logs/cbm-daemon.log`:

| File | What it is |
|------|------------|
| `trajectory.ndjson` | **The memory trajectory** — one JSON line every 5 s with `rss`, `committed` (Windows commit charge), `peak_*`, `page_faults`, `fd`, and `queries`. **This is the file we need for memory/leak reports** — the *trend over time* is what pinpoints a leak. It is **kept on disk after the server exits** (so you can grab it post-mortem) and rotates to `trajectory.ndjson.1` past ~8 MB. |
| `snapshot.json` | The latest snapshot only — handy for a quick live check. Removed on clean exit. |

The private randomized directory prevents another local account from pre-placing a link or special file at a predictable diagnostics path. Its `<pid>` component is the shared daemon's process ID, also recorded by the `daemon.start` event. Set the variable consistently in the `env` block of each agent's MCP server config, or export it before launching the first session.

### What to share

When you open a memory/performance issue, **attach the `.ndjson` trajectory** — it contains no source code or query text, only resource counters. If you'd rather not attach a file, paste it (or an agent's summary of it) into the issue: your assistant can read the NDJSON directly and report whether `rss`/`committed` grow monotonically, how fast, and relative to query count — which is exactly what we need to find the cause.

## Installation

Clone and build this fork: [Install from this repository](#install-from-this-repository). The tables below describe **upstream** DeusData release archives and package managers. They do not install [codebase-kanban-mcp](https://github.com/JMSolorzano-13/codebase-kanban-mcp).

### Upstream pre-built binaries

| Platform | Archive |
|----------|---------|
| macOS (Apple Silicon) | `codebase-memory-mcp-darwin-arm64.tar.gz` |
| macOS (Intel) | `codebase-memory-mcp-darwin-amd64.tar.gz` |
| Linux (x86_64) | `codebase-memory-mcp-linux-amd64.tar.gz` |
| Linux (ARM64) | `codebase-memory-mcp-linux-arm64.tar.gz` |
| Windows (x86_64) | `codebase-memory-mcp-windows-amd64.zip` |

Every release includes `checksums.txt` with SHA-256 hashes. The executable is self-contained — no adjacent data file is required. Linux `-portable` archives contain the fully static builds; ordinary platform archives use their native system ABI.

> **Windows note**: SmartScreen may show a warning for unsigned software. Click **"More info"** → **"Run anyway"**. Verify integrity with `checksums.txt`.

### Setup Scripts (upstream engine only)

These download [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp), not this fork.

<details>
<summary>Automated download + install</summary>

**macOS / Linux:**

```bash
curl -fsSL https://raw.githubusercontent.com/DeusData/codebase-memory-mcp/main/scripts/setup.sh | bash
```

**Windows (PowerShell):**

```powershell
irm https://raw.githubusercontent.com/DeusData/codebase-memory-mcp/main/scripts/setup-windows.ps1 | iex
```

</details>

### AUR (Arch Linux) — upstream engine only

```bash
yay -S codebase-memory-mcp-bin
```

```bash
paru -S codebase-memory-mcp-bin
```

The `codebase-memory-mcp-bin` package is available at: https://aur.archlinux.org/packages/codebase-memory-mcp-bin

### Install via Claude Code

```
You: "Install this MCP server: https://github.com/JMSolorzano-13/codebase-kanban-mcp"
```

### Build from Source

<details>
<summary>Prerequisites: C compiler + zlib</summary>

| Requirement | Check | Install |
|-------------|-------|---------|
| **C compiler** (gcc or clang) | `gcc --version` or `clang --version` | macOS: `xcode-select --install`, Linux: `apt install build-essential` |
| **C++ compiler** | `g++ --version` or `clang++ --version` | Same as above |
| **zlib** | — | macOS: included, Linux: `apt install zlib1g-dev` |
| **Git** | `git --version` | Pre-installed on most systems |
| **Node.js 22+** | `node --version` | Only for `--with-ui` (operator workspace) |

</details>

```bash
git clone https://github.com/JMSolorzano-13/codebase-kanban-mcp.git
cd codebase-kanban-mcp
scripts/build.sh --with-ui          # shipped composition: engine + operator UI
scripts/build.sh                    # engine only (no Dashboard / Specs / Game)
# Binary at: build/c/codebase-memory-mcp   (codebase-memory-mcp.exe on Windows)
./build/c/codebase-memory-mcp install
./build/c/codebase-memory-mcp --ui=true --port=9749
```

Every platform ships **one self-contained executable**: the graph UI and the agent integration templates are linked into the binary, so an extracted archive is immediately complete.

Run the test suite (6,768 tests across 120 suites):

```bash
scripts/test.sh                     # full: clean sanitizer build + all suites + guards
scripts/test.sh --suites <name>     # one suite, incremental, seconds
build/c/test-runner --list-suites   # what is available
```

`scripts/test.sh` is the same entry the CI gates run, so a local pass means the same thing a CI pass does. Packaging a release archive locally uses the same canonical script the release pipeline calls:

```bash
scripts/package-release.sh <linux|darwin|windows> <amd64|arm64>
```

### Manual MCP Configuration

<details>
<summary>If you prefer not to use the install command</summary>

Add to `~/.claude.json` (user scope) or project `.mcp.json`:

```json
{
  "mcpServers": {
    "codebase-memory-mcp": {
      "command": "/path/to/codebase-memory-mcp",
      "args": []
    }
  }
}
```

Restart your agent. Verify with `/mcp` — you should see `codebase-memory-mcp` with 16 tools.

</details>

## Multi-Agent Support

`install` configures 43 client surfaces: 37 detected automatically and 6
conditional or explicit. “Conditional” means the installer writes only when the
documented platform or an explicit, already-existing config path proves the
target is active. It never flips experimental feature flags, enables plugins,
YOLO modes, global permission bypasses, or third-party instruction trust.

Where a client has a documented custom-agent format, the installer creates three
exact-owned definitions from one canonical contract:

- **Scout (Tier 1)** — about 3–4 narrow calls for fast positive, provisional discovery; no absence, exhaustive-impact, or dead-code claims.
- **Verify (Tier 2, default)** — task-directed graph evidence, exact source checks, path coverage for every cited file, and scope coverage before negative claims.
- **Auditor (Tier 3)** — bounded scope, current index generation, complete relevant pagination, broader relationship checks, and explicit unresolved limitations.

Every direct tier batches `check_index_coverage` for its evidence paths and reads
flagged ranges or skipped/excluded files directly. A clean coverage result means
only “no recorded gap,” never proof of completeness. Clients without safe child
MCP access receive the same three tiers as parent-handoff agents; the parent must
supply project, generation, pagination state, graph evidence, and coverage
results. Updates migrate only byte-identical prior Verify definitions and never
overwrite user-modified agents.

| Agent | Activation | MCP config | Durable context / augmentation |
|-------|------------|------------|--------------------------------|
| Claude Code | Detected | `~/.claude.json` | Skill + three exact-tool graph agents; `SessionStart`, `SubagentStart`, non-blocking `PreToolUse` for `Grep`/`Glob`, and post-`Read` coverage |
| Codex CLI | Detected | `$CODEX_HOME/config.toml` | `AGENTS.md`, skill, three read-only agents; `SessionStart` + `SubagentStart` |
| Gemini CLI | Detected | `.gemini/settings.json` | `GEMINI.md`, three explicit read/graph-tool subagents; `BeforeTool`, `AfterTool` `read_file` coverage, and `SessionStart` |
| Zed | Detected | platform `settings.json` (JSONC) | `AGENTS.md` + shared skill |
| OpenCode | Detected | `$OPENCODE_CONFIG` or resolved global config | `AGENTS.md`, skill, three deny-by-default read-only agents |
| Antigravity | Detected | `.gemini/config/mcp_config.json` | `.gemini/GEMINI.md` |
| Aider | Detected | — | `CONVENTIONS.md` via `.aider.conf.yml` |
| KiloCode | Detected | `.config/kilo/kilo.jsonc` | Rule + three graph-tool subagents with deny-by-default permissions |
| VS Code | Detected | platform `Code/User/mcp.json` | `~/.copilot/skills`, three read-only agents, `sessionStart` + `subagentStart` |
| Cursor | Detected | `.cursor/mcp.json` | Skill + three read-only parent-handoff agents; context hooks withheld because session injection races and `readonly` blocks MCP |
| Windsurf | Detected | `~/.codeium/windsurf/mcp_config.json` | Always-on `global_rules.md` |
| Augment / Auggie | Detected | `~/.augment/settings.json` | Rule, three read-only handoff subagents, `SessionStart` + post-`view` coverage |
| OpenClaw | Detected | `$OPENCLAW_CONFIG_PATH` or state `openclaw.json` | Active-workspace `AGENTS.md` + `TOOLS.md`; compaction reinjection |
| Kiro | Detected | `$KIRO_HOME/settings/mcp.json` | Steering, skill, three JSON agents with isolated Scout/Analysis-profile MCP and explicit graph-tool selectors (`includeMcpJson: false`) |
| Junie | Detected | `.junie/mcp/mcp.json` | Skill + three graph subagents for EAP-capable builds; Scout and Analysis server aliases hard-limit the tier tool surfaces; no ineffective EAP `SessionStart` hook |
| Hermes | Detected | `$HERMES_HOME/config.yaml` | Skill + fail-open `pre_llm_call` context augmentation |
| OpenHands | Detected | `.openhands/mcp.json` | Shared `.agents/skills/codebase-memory/SKILL.md` |
| Cline | Detected | `~/.cline/mcp.json` + `${CLINE_DATA_DIR:-~/.cline/data}/settings/cline_mcp_settings.json` | Rule + skill; automatic file hooks withheld because they auto-activate and their output is not reliably consumed; child agents cannot use MCP |
| Warp | Detected, skill only | UI, Warp Drive, or per invocation (manual) | Shared `~/.agents/skills/codebase-memory/SKILL.md` |
| Qwen Code | Detected | `.qwen/settings.json` | `QWEN.md`, skill, three explicit read/graph-tool agents; `SessionStart`, `SubagentStart`, and post-`ReadFile` coverage |
| GitHub Copilot CLI | Detected | `$COPILOT_HOME/mcp-config.json` | Instructions, skill, three read-only agents; `sessionStart` + `subagentStart` |
| Factory Droid | Detected | `.factory/mcp.json` | `AGENTS.md`, skill, three droids with exact per-tier graph-tool lists (without additive whole-server exposure); `SessionStart` + post-`Read` coverage on macOS/Linux, withheld on Windows |
| Crush | Detected | `.config/crush/crush.json` | Managed context path with explicit parent-to-child handoff |
| Goose | Detected | `.config/goose/config.yaml` | `.goosehints` |
| Mistral Vibe | Detected | `$VIBE_HOME/config.toml` | `AGENTS.md`, skill, and three matched agent/prompt pairs with explicit read-only graph-tool allowlists |
| Qoder CLI | Detected | `~/.qoder/settings.json` | Skill, three directly MCP-attached agents with named-server scoping and exact per-tier graph-tool lists; `SessionStart`, `SubagentStart`, and post-`Read` coverage, including documented PowerShell execution on Windows |
| Kimi Code CLI | Detected | `$KIMI_CODE_HOME/mcp.json` (default `~/.kimi-code`) | Same-root `AGENTS.md` + skill; fail-open `UserPromptSubmit` hook in `config.toml` |
| GitLab Duo CLI | Detected | `$GLAB_CONFIG_DIR/duo/mcp.json` or platform fallback | Fail-open user `SessionStart` on macOS/Linux; hook withheld on Windows; no experimental global skill enablement |
| Rovo Dev CLI | Detected | configured override or `~/.rovodev/mcp.json` | Global `AGENTS.md`, skill + three read-only handoff subagents; no undocumented hook |
| Amp | Detected | `~/.config/agents/skills/codebase-memory/mcp.json` | Colocated skill + `~/.config/amp/AGENTS.md`; no plugin |
| Devin CLI / Local | Detected | `~/.config/devin/config.json` (platform app-data path on Windows) | Same-root `AGENTS.md` + skill; macOS/Linux `UserPromptSubmit` + `PostCompaction`, and `SessionStart` only when Claude does not already provide it; hooks withheld on Windows |
| Tabnine | Detected | `~/.tabnine/mcp_servers.json` | MCP only; no experimental/YOLO setting |
| Continue / cn | Conditional | Existing `~/.continue/config.yaml` or `$CBM_CONTINUE_CONFIG_PATH` | MCP only |
| Visual Studio | Conditional, Windows | `~/.mcp.json` | MCP only |
| TRAE | Conditional | Existing `$CBM_TRAE_CONFIG_PATH` | MCP only |
| Roo Code | Conditional | Existing `$CBM_ROO_CONFIG_PATH` | MCP only |
| Amazon Q Developer IDE | Detected | `~/.aws/amazonq/default.json` (preserves an existing `agents/default.json` or legacy `mcp.json`) | MCP only |
| CodeBuddy Code CLI | Detected | `~/.codebuddy/.mcp.json` (preserves an active deprecated/legacy file) | `CODEBUDDY.md`, skill, three read-only graph agents; beta hooks are not auto-installed |
| IBM Bob Shell | Detected by `bob` | `~/.bob/mcp_settings.json` | Shared rule; no invented hook or agent |
| Pochi | Detected | `~/.pochi/config.jsonc` (`mcp`) | `README.pochi.md`, skill, and three `readFile`-only parent-handoff agents |
| Pi | Detected | — | `~/.pi/agent/AGENTS.md` + skill; MCP/subagents require an explicit reviewed extension |
| IBM Bob IDE | Conditional | Existing `~/.bob/mcp.json` | Shared rule + IDE skill; no invented hook or agent |
| Sourcegraph Cody | Explicit opt-in | Existing `$CBM_CODY_CONFIG_PATH` | MCP only |

### Sessions, compaction, and subagents

Hooks installed by this project are fail-open and context-only. Claude Code's
`PreToolUse` observes `Grep`/`Glob` and injects matching graph symbols as
`additionalContext`; `PostToolUse` on `Read` adds targeted coverage context when
the graph could not fully parse or index that file. It never denies or replaces
the requested tool call.

Claude Code, Codex CLI, Qwen Code, GitHub Copilot CLI, and VS Code's Copilot
runtime receive paired session/subagent context where the vendor exposes a
documented context-output contract. Codex users must review and trust installed
hooks through `/hooks`; changing a hook definition changes its trust hash, so an
update can require re-trust. Qoder uses `SessionStart`, `SubagentStart`, and
post-`Read` coverage, including its documented PowerShell executor on Windows.
Kimi uses `UserPromptSubmit`, while Hermes uses `pre_llm_call`; both retain their
documented Windows execution paths. Devin installs
`UserPromptSubmit` and `PostCompaction` on macOS/Linux and adds `SessionStart`
only when Claude's equivalent managed hook is not present. GitLab Duo gets a
narrowly scoped macOS/Linux user `SessionStart` entry on its experimental hook
surface. GitLab Duo, Devin, and Factory hooks are withheld on Windows
because those vendors do not document a deterministic shell/executor contract
there. Gemini CLI, Factory Droid, and Augment also add documented post-read/view
coverage context but expose no equivalent documented child-start context.

For runtimes without a stable context-producing lifecycle event, durable files
carry the contract across fresh sessions and compaction: verify the graph project
and index freshness, query structural facts in the parent, then pass the project,
qualified symbols, paths, and call-chain evidence in every delegated task.
Claude, Codex, Gemini, Kiro, Qwen, Copilot, CodeBuddy, OpenCode, Kilo, Vibe,
Qoder, Junie, and Factory receive Scout, Verify, and Auditor graph profiles.
Kiro embeds this MCP server with `--tool-profile scout` for Scout and
`--tool-profile analysis` for Verify/Auditor. Junie registers equivalent named
server aliases because its subagent schema filters by server rather than by
individual tool. Both process profiles use positive allowlists: Scout exposes
seven fast inspection tools, Analysis exposes eleven, and future or mutating
tools remain unavailable until explicitly reviewed. If either Junie alias
collides with user configuration, the installer preserves it and installs
parent-handoff profiles instead. Qoder combines its documented named-server
selection with exact tier-specific MCP tool IDs. Factory uses exact registered
MCP tool IDs without its additive `mcpServers` field, which would expose the
whole server. Codex, Kilo, Vibe, and other capable formats likewise enumerate
the narrowest supported tool set. Rovo, Cursor, Augment, Pochi, and Cline use parent handoff where direct
child MCP is unavailable or unsafe; Pochi is limited to `readFile`, and Cline
child agents cannot use MCP.

Cline's file hooks auto-activate when present, and current Cline does not
reliably consume their context output, so automatic adapters are withheld and
older owned adapters are cleaned up. CodeBuddy's beta, version-gated hooks are
not auto-installed. Junie's EAP
`SessionStart` output is documented as ignored, so no context hook is installed.
Junie custom agents remain EAP-dependent. Qoder can resolve higher-priority
project or plugin agents before user agents with the same name; reload the
client after installation or profile changes.
Cursor context
hooks are withheld: session context injection has a known race, `subagentStart`
is control-only, and read-only subagents cannot safely receive MCP access. Rovo
has no documented session context-output hook, and Bob
documents neither a suitable hook nor a custom-agent surface. Those surfaces are
not approximated with invented augmentation. Kimi plugins, Amp plugins, and
GitLab experimental global skills remain opt-in.

OpenClaw reinjects the `Codebase Knowledge Graph (codebase-memory-mcp)` AGENTS
section after compaction and places the same guidance in `TOOLS.md`, the bootstrap
files inherited by its subagents. Automatic augmentation covers the active/default
workspace. Separate `agents.list[].workspace` directories require making that
workspace active for installation or copying the managed block there.

The installed Claude shim is named `cbm-code-discovery-gate` for backward
compatibility; despite the legacy name, it never gates or blocks.

### Manual or UI-managed integrations

These are intentionally not counted as automatic installs: Qodo MCP is added
through its UI and may be governed by enterprise allowlists; Warp MCP is managed
through Warp Drive/UI or per invocation (only the shared skill is automatic);
JetBrains AI Assistant / ACP is IDE-managed; GitHub Copilot coding agent, Jules,
and CodeRabbit are cloud/repository-managed; Replit exposes a remote/service
integration rather than a stable local user-global client; BLACKBOX AI does not
document a stable arbitrary user-global MCP/instruction/agent schema; Plandex has
no stable global registry safe to mutate; and SWE-agent uses explicit YAML and is
no longer a suitable automatic global target.

## CLI Mode

Every MCP tool can be invoked as a local, one-shot command. CLI tools neither start nor connect to the coordination daemon and leave no standing process behind. They hold a crash-safe exact-build admission lease only for the command lifetime. `index_repository` is the only exception internally: it starts a temporary, exact-build supervised worker for the index, then stops that worker before the CLI command exits; the worker holds its own lease until exit.

Commands that mutate graph data use shared OS-backed, per-project locks. This serializes conflicting work from CLI and MCP sessions on the same project while allowing unrelated projects to proceed independently.

When stderr is an interactive terminal, the CLI automatically shows lifecycle and indexing progress. Pass `--progress` to force the same feedback when stderr is redirected or the command is run non-interactively. Progress is written only to stderr; stdout remains reserved for the command result, so pipes and scripts stay machine-safe. Pass `--json` when the full MCP result envelope is needed.

Use `cli <tool> --help` to see the flags generated from that tool's input schema:

```bash
codebase-memory-mcp cli index_repository --repo-path /path/to/repo
codebase-memory-mcp cli list_projects

# Use the "name" returned by list_projects as the project value.
codebase-memory-mcp cli search_graph --project my-project --name-pattern '.*Handler.*' --label Function
codebase-memory-mcp cli trace_path --project my-project --function-name Search --direction both
codebase-memory-mcp cli query_graph --project my-project --query 'MATCH (f:Function) RETURN f.name LIMIT 5'

# Force human-readable progress without contaminating stdout.
codebase-memory-mcp cli --progress index_repository --repo-path /path/to/repo
codebase-memory-mcp cli search_graph --project my-project --label Function | jq '.results[].name'
```

JSON arguments can also be piped on stdin. Inline JSON remains accepted for backward compatibility but is deprecated in favor of flags, `--args-file`, or stdin.

## MCP Tools

### Indexing

| Tool | Description |
|------|-------------|
| `index_repository` | Index a repository into the graph. Auto-sync keeps it fresh after that. |
| `list_projects` | List all indexed projects with node/edge counts. |
| `delete_project` | Remove a project and all its graph data. |
| `index_status` | Check indexing status of a project. |
| `check_index_coverage` | Coverage for cited paths and bounded scopes. Best-effort; a clean result is not proof of completeness. |

### Querying

| Tool | Description |
|------|-------------|
| `search_graph` | Structured search by label, name pattern, file pattern, degree filters. Pagination via limit/offset. |
| `trace_path` | BFS traversal — who calls a function and what it calls (alias: `trace_call_path`). Depth 1-5. |
| `detect_changes` | Map git diff to affected symbols + blast radius with risk classification. |
| `query_graph` | Execute Cypher-like graph queries (read-only). |
| `get_graph_schema` | Node/edge counts, relationship patterns, property definitions per label. Run this first. |
| `get_code_snippet` | Read source code for a function by qualified name. |
| `get_architecture` | Codebase overview: languages, packages, routes, hotspots, clusters, ADR. |
| `search_code` | Grep-like text search within indexed project files. |
| `manage_adr` | CRUD for Architecture Decision Records. Query modes do not wait behind a same-project reindex; writes remain serialized. |
| `ingest_traces` | Ingest runtime traces to validate HTTP_CALLS edges. |

`manage_adr` query modes (`get` and `sections`) use the server's cached query store so they can proceed while a same-project reindex is running. If another process publishes a replacement store during reindexing, they can return the pre-publication ADR until idle eviction refreshes that cache. Updates remain serialized through the project mutation guard.

There is no MCP tool for Specs/Game archive. Those flags are HTTP-only (`POST /api/spec-board`, `POST /api/game-board`) and never written into skill trees.

## Graph Data Model

### Node Labels

`Project`, `Package`, `Folder`, `File`, `Module`, `Class`, `Function`, `Method`, `Interface`, `Enum`, `Type`, `Route`, `Resource`

### Edge Types

`CONTAINS_PACKAGE`, `CONTAINS_FOLDER`, `CONTAINS_FILE`, `DEFINES`, `DEFINES_METHOD`, `IMPORTS`, `CALLS`, `CALL_REFERENCE`, `HTTP_CALLS`, `ASYNC_CALLS`, `IMPLEMENTS`, `HANDLES`, `USAGE`, `CONFIGURES`, `WRITES`, `MEMBER_OF`, `TESTS`, `USES_TYPE`, `FILE_CHANGES_WITH`

### Qualified Names

`get_code_snippet` uses qualified names: `<project>.<path_parts>.<name>`. Use `search_graph` to discover them first.

### Supported Cypher (openCypher read subset)

`query_graph` is a read-only openCypher subset:

- **Clauses**: `MATCH`, `OPTIONAL MATCH`, multiple `MATCH`, `WHERE`, `WITH` (+ `WITH … WHERE`), `RETURN`, `ORDER BY`, `SKIP`, `LIMIT`, `DISTINCT`, `UNWIND`, `UNION` / `UNION ALL`, `CASE`.
- **Patterns**: labelled nodes, label alternation `(n:A|B)`, relationship types/direction, variable-length paths `[*1..3]`, inline property maps.
- **WHERE**: `= <> < <= > >=`, `AND/OR/XOR/NOT`, `IN`, `CONTAINS`, `STARTS WITH`, `ENDS WITH`, `IS [NOT] NULL`, regex `=~`, label test `n:Label`, and `EXISTS { (n)-[:TYPE]->() }` (single-hop existence — great for dead-code, e.g. `WHERE NOT EXISTS { (f)<-[:CALLS]-() }`).
- **Aggregates**: `count` (+`DISTINCT`), `sum`, `avg`, `min`, `max`, `collect`.
- **Functions**: `labels`, `type`, `id`, `keys`, `properties`; `toLower/toUpper/toString/toInteger/toFloat/toBoolean`; `size`, `length`, `trim/ltrim/rtrim`, `reverse`; `coalesce`, `substring`, `replace`, `left`, `right`.

Anything outside this subset (write/`MERGE`/`CALL` clauses, unsupported functions, list/map literals, comprehensions, path functions, parameters) **fails with a clear `unsupported …` error** rather than returning empty results.

## Ignoring Files

Layered: hardcoded patterns (`.git`, `node_modules`, etc.) → `.gitignore` hierarchy → `.cbmignore` (project-specific, gitignore syntax). Symlinks are always skipped.

See [docs/cbmignore.md](docs/cbmignore.md) for the full `.cbmignore` how-to: syntax, precedence across the ignore layers, and negation semantics.

## Configuration

```bash
codebase-memory-mcp config list                          # show all settings
codebase-memory-mcp config set auto_index true           # auto-index on session start
codebase-memory-mcp config set auto_index_limit 50000    # max files for auto-index
codebase-memory-mcp config set auto_watch false          # don't register background git watcher (default: true)
codebase-memory-mcp config reset auto_index              # reset to default
```

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `CBM_ALLOWED_ROOT` | *(unset)* | Confine `index_repository` to paths within this directory. When set, a `repo_path` that resolves (after symlink / `..` resolution) outside this root is refused, and the same check now applies to the graph UI's `POST /api/index` route rather than only to the MCP tool. Unset imposes no *containment* restriction — but see the always-on limits below, which apply whether or not this is set. Useful when the server may be driven by an untrusted caller, e.g. agentic or multi-tenant deployments. |
| `CBM_CACHE_DIR` | `~/.cache/codebase-memory-mcp` | Override the database storage directory. All project indexes and config are stored here. One account can use only one canonical cache root at a time; close active CBM sessions/commands before switching it. |
| `CBM_DIAGNOSTICS` | `false` | Set to `1` or `true` to enable the shared daemon's periodic `snapshot.json` and retained `trajectory.ndjson` below a fresh owner-private directory in the system temp directory. Exact paths are logged by `diagnostics.start`. |
| `CBM_DOWNLOAD_URL` | *(GitHub releases)* | Override the download URL for updates. Used for testing or self-hosted deployments. |
| `CBM_LOG_LEVEL` | `info` | Set the minimum log level. Accepted values (case-insensitive): `debug`, `info`, `warn`, `error`, `none` — or their numeric equivalents `0`–`4` matching the internal enum. Thin-frontend messages go to that session's stderr; detached daemon events go to `${CBM_CACHE_DIR}/logs/cbm-daemon.log`. Stdout is reserved for MCP JSON-RPC. |
| `CBM_WORKERS` | *(detected)* | Override the parallel-indexing worker count returned by `cbm_default_worker_count`. Useful inside containers where `sysconf(_SC_NPROCESSORS_ONLN)` reports host CPUs rather than the cgroup's effective quota. Range 1–256; invalid values are ignored with a warning. |
| `CBM_MEM_BUDGET_MB` | *(detected)* | Override the in-memory graph budget with an explicit cap in MiB, taking precedence over the `ram_fraction × total_RAM` default. Useful on bare-metal hosts without a cgroup limit, or to pin a budget *below* the cgroup limit so headroom is left for sibling processes. Must be a positive integer; it is clamped to detected total RAM (logged as `mem.budget.clamped`), and non-numeric or non-positive values are ignored with a warning (`mem.budget.env.invalid`). |
| `CBM_DUMP_VERIFY_MIN_RATIO` | `0.5` | After indexing, compare persisted SQLite node count to the in-memory dump count. When persisted nodes fall below this fraction of committed nodes (and committed > 50), `index_repository` returns `status:"degraded"` instead of silent `indexed`. Range 0–1; set `0` to disable. Invalid values are ignored with a warning. |

Environment used by daemon-owned components—such as diagnostics, daemon logging, and process-wide indexing resource limits—is captured from the first daemon-backed session that starts the daemon. Later sessions join that process and cannot replace those values. To change them, close all daemon-backed sessions, update the relevant agent configurations consistently, and restart a session. `CBM_ALLOWED_ROOT` remains session-specific, a conflicting `CBM_CACHE_DIR` is rejected, and one-shot CLI commands read their own environment without starting the daemon.

```bash
# Store indexes in a custom directory
export CBM_CACHE_DIR=~/my-projects/cbm-data
```

## Custom File Extensions

The JSON config files support a single key, `extra_extensions`, which maps additional file extensions to supported languages. Useful for framework-specific extensions like `.blade.php` (Laravel) or `.mjs` (ES modules). (For other tunables, see [Environment Variables](#environment-variables) and the `config` subcommand above.)

Need the full config-file reference? See [docs/CONFIGURATION.md](docs/CONFIGURATION.md).

**Per-project** (in your repo root):
```json
// .codebase-memory.json
{"extra_extensions": {".blade.php": "php", ".mjs": "javascript"}}
```

**Global** (applies to all projects):
```json
// ~/.config/codebase-memory-mcp/config.json  (or $XDG_CONFIG_HOME/...)
{"extra_extensions": {".twig": "html", ".phtml": "php"}}
```

Each entry maps an extension (which **must** start with `.`) to a language name. Language names are matched **case-insensitively**. Accepted values (aliases in parentheses) are:

`bash` (`sh`), `c`, `c++` (`cpp`), `c#` (`csharp`), `clojure`, `cmake`, `cobol`, `common lisp` (`commonlisp`, `lisp`), `css`, `cuda`, `dart`, `dockerfile`, `elixir`, `elm`, `emacs lisp` (`emacslisp`), `erlang`, `f#` (`fsharp`), `form`, `fortran`, `glsl`, `go`, `graphql`, `groovy`, `haskell`, `hcl` (`terraform`), `html`, `ini`, `java`, `javascript`, `json`, `julia`, `kotlin`, `lean`, `lua`, `magma`, `makefile`, `markdown`, `matlab`, `meson`, `nix`, `objective-c` (`objc`), `ocaml`, `perl`, `php`, `protobuf`, `python`, `r`, `ruby`, `rust`, `scala`, `scss`, `sql`, `svelte`, `swift`, `toml`, `tsx`, `typescript`, `verilog`, `vimscript`, `vue`, `wolfram`, `xml`, `yaml`, `zig`.

Project config overrides global for conflicting extensions. An entry whose language name is unknown, or whose extension does not start with `.`, is skipped and a warning is logged to stderr (shown at the default `info` log level). Missing config files are ignored.

## Persistence

SQLite databases stored at `~/.cache/codebase-memory-mcp/`. Persists across restarts (WAL mode, ACID-safe). To reset: `rm -rf ~/.cache/codebase-memory-mcp/`.

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `/mcp` doesn't show the server | Check `.mcp.json` path is absolute. Restart agent. Test: `echo '{}' \| /path/to/binary` should output JSON. |
| `index_repository` fails | Pass absolute path: `index_repository(repo_path="/absolute/path")` |
| `trace_path` returns 0 results | Use `search_graph(name_pattern=".*PartialName.*")` first to find the exact name. |
| Queries return wrong project results | Add `project="name"` parameter. Use `list_projects` to see names. |
| Binary not found after install | Add to PATH: `export PATH="$HOME/.local/bin:$PATH"` |
| UI not loading | `--ui=true`. Open `http://localhost:9749`. Rebuild `--with-ui` if the binary has no UI. |
| No Specs tab | Repo needs `.sdd-skill/` or `.grill/`, and must not be a `.gamedev/` path. |
| No Game tab | Repo needs a `.gamedev/` directory. Rebuild `--with-ui`. |
| Specs and Game both showing | Should not happen: Game wins. Leftover `?tab=specs` on gamedev becomes Game. |
| Indexing the same folder twice | Path is 1:1. You land on the existing Graph; use Dashboard Reindex to refresh. |
| ADR empty after watcher | Fill runs on user-triggered index only (Reindex, create, `index_repository`). |

## Hybrid LSP

**Semantic type resolution beyond tree-sitter.**

Tree-sitter alone gives a syntactic AST. That handles naming, structure, and call sites well, but it can't tell you that `user.profile.display_name()` resolves to `Profile.display_name` declared three modules away — tree-sitter doesn't track imports, generics, inheritance, or stdlib types.

codebase-memory-mcp ships a **lightweight C implementation of language type-resolution algorithms, structurally inspired by and compatible with major language servers** (tsserver / typescript-go, pyright, gopls, Roslyn, Eclipse JDT, rust-analyzer), embedded directly into the native executable. No language server process, no per-project setup, no API key. We call this layer **Hybrid LSP**: it runs alongside tree-sitter on every parse and refines invocation resolution (`CALLS` / `RESOLVED_CALLS`) and callable-value resolution (`CALL_REFERENCE`, with ambiguous values retained as `USAGE`) using type information, so the resulting graph mirrors what an IDE "Go to Definition" would resolve.

**Languages with full Hybrid LSP:**

| Language | What it handles |
|----------|-----------------|
| **Python** *(new in v0.7.0)* | imports + dotted submodule walks, dataclasses, `Self` return types, generics, `@property`, `match/case` class patterns, SQLAlchemy 2.0 `Mapped[T]`, Pydantic `BaseModel`, `typing.Annotated` / `ClassVar` / `Final` / `InitVar`, async/await, classmethod/staticmethod, narrowing (`isinstance` / `is not None` / walrus), `typing.cast` / `assert_type`, common stdlib (logging, pathlib, json, functools). Target ~95% resolution on idiomatic code. |
| **TypeScript / JavaScript / JSX / TSX** | generics, JSX component dispatch, JSDoc inference for plain JS, `.d.ts` declarations, module re-exports, method chaining via return-type propagation, per-file overlay chained to a shared cross-file registry |
| **PHP** *(new in v0.7.0)* | namespaces, traits, late-static-binding, PHPDoc inference, parameter binding, return-type inference |
| **C#** *(new in v0.7.0)* | global usings, file-scoped namespaces, records (incl. C# 12 primary constructors), LINQ method syntax, `async Task<T>` / `ValueTask<T>` unwrap, generic methods, `this` / `base` dispatch, `var` inference, common BCL stdlib |
| **Go** *(sharpened in v0.7.0)* | pre-built per-package cross-file registry, generics, embedded structs, interface satisfaction, package-aware import resolution |
| **C / C++** *(sharpened in v0.7.0)* | pre-built per-language cross-file registry shared across C and C++; C side handles macros + `typedef` chains + header-vs-source linking; C++ side handles templates, namespaces, `auto` inference, and method resolution via class hierarchy |
| **Java** *(new in v0.8.0)* | imports (single-type, on-demand, static), class hierarchies with `this` / `super` dispatch, generics, annotations, overload matching by arity and parameter types, lambdas / method references bound to functional interfaces, field-type inference, common JDK stdlib |
| **Kotlin** *(new in v0.8.0)* | imports + same-package resolution, classes / objects / companion objects, extension functions, data classes, nullable-type unwrapping, scope functions (`let` / `apply` / `run` / `also` / `with`), infix calls, common stdlib |
| **Rust** *(new in v0.8.0)* | `use` declarations + module paths, `impl` blocks and trait methods, struct fields, generics with trait bounds, operator-trait desugaring, derive-macro method synthesis, UFCS static paths, common std prelude |
| **Perl** | packages + `@ISA` / `use parent` / `use base` inheritance with method-resolution-order dispatch, `SUPER::` calls, Exporter (`use Foo qw(...)`) import maps, `bless` / `ref($class)\|\|$class` self-type inference, qualified `Pkg::sub` static calls, curated perlfunc + CPAN OOP stdlib; unresolved receivers emit no edge (zero-edge guarantee) |

**Two-layer architecture:**

1. **Tree-sitter pass** — fast, syntactic, runs for every one of the 158 languages. Extracts definitions, calls, imports.
2. **Hybrid LSP pass** — type-aware, runs above the tree-sitter pass per-language. Refines call edges using the import graph plus a per-file or pre-built cross-file definition registry. Languages without a Hybrid LSP pass yet fall back to textual resolution, so you always get *some* answer.

The result is a knowledge graph accurate enough to drive `trace_path` across packages, inheritance hierarchies, and stdlib calls — without paying for a language server process per project.

## Language Support

158 languages, all parsed via vendored tree-sitter grammars compiled into the binary. Benchmarked against 64 real open-source repositories (78 to 49K nodes):

| Tier | Score | Languages |
|------|-------|-----------|
| **Excellent** (>= 90%) | | Lua, Kotlin, C++, Perl, Objective-C, Groovy, C, Bash, Zig, Swift, CSS, YAML, TOML, HTML, SCSS, HCL, Dockerfile |
| **Good** (75-89%) | | Python, TypeScript, TSX, Go, Rust, Java, R, Dart, JavaScript, Erlang, Elixir, Scala, Ruby, PHP, C#, SQL |
| **Functional** (< 75%) | | OCaml, Haskell |

Also supported (not yet benchmarked): Ada, Agda, Apex, Assembly (NASM), Astro, AWK, Beancount, BibTeX, Bicep, Bitbake, Blade, Cairo, Cap'n Proto, Clojure, CMake, COBOL, Common Lisp, Crystal, CSV, CUDA, D, Devicetree, Diff, .env, Elm, Emacs Lisp, F#, Fennel, Fish, FORM, Fortran, FunC, GDScript, .gitattributes, .gitignore, Gleam, GLSL, GN, Go module, Go template, GraphQL, Hare, HLSL, Hyprlang, INI, ISPC, Janet, Jinja2, JSDoc, JSON, JSON5, Jsonnet, Julia, Just, Kconfig, KDL, Lean 4, Linker Script, Liquid, LLVM IR, Luau, Magma, Makefile, Markdown, MATLAB, Mermaid, Meson, Move, Nickel, Nim, Nix, Odin, Pascal, Pkl, PO (gettext), Pony, PowerShell, Prisma, .properties, Protobuf, Puppet, PureScript, Racket, Regex, requirements.txt, ReScript, RON, reStructuredText, Scheme, Slang, Smali, Smithy, Solidity, SOQL, SOSL, Squirrel, SSH config, Starlark, Svelte, Sway, SystemVerilog, TableGen, Tcl, Teal, Templ, Thrift, TLA+, Typst, Verilog, VHDL, Vim script, Vue, WGSL, WIT, Wolfram, XML, Zsh.

## Architecture

```
src/
  main.c              Entry point (MCP stdio server + CLI + install/update/config)
  daemon/             Per-account session coordination, IPC, lifecycle, shared jobs/watchers
  mcp/                MCP server (16 tools, JSON-RPC 2.0, session detection, auto-index)
  adr/                XOR trio parse on user-triggered index (gamedev dir wins, else sdd)
  cli/                Install/uninstall/update/config (43 client surfaces, hooks, instructions)
  store/              SQLite graph + spec_archive + game_archive
  pipeline/           Multi-pass indexing (structure → definitions → calls → HTTP links → config → tests)
  cypher/             Cypher query lexer, parser, planner, executor
  discover/           File discovery (.gitignore, .cbmignore, symlink handling; .sdd-skill skipped)
  watcher/            Background auto-sync (git polling; does not fill ADR)
  traces/             Runtime trace ingestion
  ui/                 HTTP + spec_board + game_board (read skill trees; archive flags only)
  foundation/         Platform abstractions (threads, filesystem, logging, memory)
graph-ui/             Dashboard, Graph, Specs, Game, ADR (embedded when built --with-ui)
internal/cbm/         Vendored tree-sitter grammars (158 languages) + AST extraction engine
```

## Security

This fork is meant to be compiled from [JMSolorzano-13/codebase-kanban-mcp](https://github.com/JMSolorzano-13/codebase-kanban-mcp). It does not publish signed GitHub Releases yet. Libraries are vendored at compile time; there is no language-runtime download chain for the engine itself.

The original project’s published archives use a multi-layer pipeline (VirusTotal on extracted members, SLSA Level 3, Sigstore cosign, SHA-256 checksums, CodeQL). That pipeline applies to [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp) releases, not to a local build of this fork. Verify upstream artifacts with:

```bash
gh attestation verify <file> --repo DeusData/codebase-memory-mcp --signer-workflow DeusData/codebase-memory-mcp/.github/workflows/_build.yml
```

### Upstream v0.7.0 VirusTotal scans

These hashes are from the original project’s v0.7.0 release, not from this fork.

| Binary | SHA-256 | VirusTotal |
|--------|---------|-----------|
| `linux-amd64` | `8e12bb2d6ead7f20a6d3...` | [0/72 ✅](https://www.virustotal.com/gui/file/8e12bb2d6ead7f20a6d3bf2be1e51f978c38acce810f0734f510d134b039d152/detection) |
| `linux-arm64` | `10f7136bfbf3950c6b2a...` | [0/72 ✅](https://www.virustotal.com/gui/file/10f7136bfbf3950c6b2a1a950bbf85e88b97ee55ab00b4dfbc2a5e9c2ede8672/detection) |
| `darwin-arm64` | `7062a7408906344bf4f8...` | [0/72 ✅](https://www.virustotal.com/gui/file/7062a7408906344bf4f835e9580048af85d12dd2b7cec0edf869df93ad9a0592/detection) |
| `darwin-amd64` | `28c6d640e1a0ac7bfcab...` | [0/72 ✅](https://www.virustotal.com/gui/file/28c6d640e1a0ac7bfcab5094c2186eced5264a20dcdffcb4455a1b28c5df2171/detection) |
| `windows-amd64` | `9c3ddcf78368fd4fa891...` | [0/72 ✅](https://www.virustotal.com/gui/file/9c3ddcf78368fd4fa89156a553641bf1e03640b4fb6dd29a12c84aa5bc98cd86/detection) |

Scan links for every upstream release are included in that project’s GitHub Release notes.

## License

MIT. See [LICENSE](LICENSE).

Copyright (c) 2026 Juan M. Solórzano I. Portions of the engine are Copyright (c) 2025 DeusData.

## Fork and original project

This repository — [JMSolorzano-13/codebase-kanban-mcp](https://github.com/JMSolorzano-13/codebase-kanban-mcp) — is a **fork** of [DeusData/codebase-memory-mcp](https://github.com/DeusData/codebase-memory-mcp).

The knowledge-graph engine, MCP tools, Hybrid LSP layer, tree-sitter grammars, CLI/install surfaces, and signed-release pipeline come from that project. This fork adds the executive Dashboard and the Specs / Game kanban operator UI (read-only skill boards). The installed binary is still named `codebase-memory-mcp`.

| | URL |
|--|-----|
| This fork | https://github.com/JMSolorzano-13/codebase-kanban-mcp |
| Original project | https://github.com/DeusData/codebase-memory-mcp |
| Engine research preprint | https://arxiv.org/abs/2603.27277 |

Use this fork when you want the kanban operator UI. Use the original project for published binaries, npm/PyPI/Homebrew packages, and supply-chain attestations.
