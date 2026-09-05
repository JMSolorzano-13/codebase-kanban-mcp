plan: add-gamedev-skill
date: 2026-08-30
skill: /Users/jmsolorzano/SWE/_SKILLs/gamedev-skill skill_version 1.10.0
inspected: SKILL.md, filesystem.md, cycle.md, instructions.md, templates/state.md, CONTEXT.md, bevy-tetris/.gamedev/

identity:
- One .gamedev/ = one game. No portfolio layer. Filesystem = memory.
- 15 specialists + @director (producer, not router). Parallel inside a phase; spine of phases is linear + human gates.

phases (user said pre-prod / prod / prod+launch — skill names the third Post-production & Launch):
- 01-preproduction: 6 agents parallel (game-designer, narrative, art-director, tech-architect, audio-director, producer). Gate1: gdd + style-guide + tech-architecture approved.
- 02-production: 6 + continuous qa-lead. Gate2: vertical slice / feature-complete per roadmap.md.
- 03-postproduction: mostly sequential (perf → platform → release); marketing can start early parallel. Gate3: cert + release checklist + ship. Then @analyst postmortem.

tracks (independent of phase):
- A code/technical SDD-style: spec folder {spec,tasks,review}.md. Agents: tech-architect, gameplay-engineer, performance-engineer, platform-integrator, release-engineer. Also pre-prod tech-specs/ if substantial.
- B creative/design document-driven: phase docs, changelog in-doc, no Gherkin. Quality = @director + Playtest Loop.
- Hybrid: @level-designer (LVL-0XX + changelog, Track B docs + A-like traceability). @qa-lead bridges: playtest log vs escalate /bug to Track A. CONTEXT.md labels this Track H.

parallel + deps (must be visible on a board if CBM is to match the skill):
- state.md: phase + N non-idle agent lines (in_progress|blocked|needs_review|done) with blocked-by.
- filesystem.md INPUT→OUTPUT map: gdd→tech/style/audio/production-plan; systems→levels+playtest; etc.
- roadmap.md: milestones, gate criteria, epic map, dependency graph (bevy-tetris has this filled).
- Conditional gates → backlog.md debt:gate-* (never silent).

quality vs sdd:
- SDD: Gherkin/test_results.log. Gamedev: Playtest Feedback Loop. Technical crash → Track A bug, not playtest.

grill relation (skill-level, not CBM):
- grill-skill zero-dependency; plans may feed gamedev. bevy-tetris roadmap.source = "grill plan block-line-adventure-mvp2". Conversion is human/@director writing roadmap/systems, not a Companion-to line in spec.md.

sdd-inside-gamedev:
- Track A IS the SDD cycle, relocated under .gamedev/phases/**/systems/. No planner/implementer split — owning agent plans AND executes.
- Running sdd-skill on same root would duplicate spec trees (.sdd-skill/specs vs systems/) and two state.md routers.
