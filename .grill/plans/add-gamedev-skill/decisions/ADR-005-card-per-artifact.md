adr: 005
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: ADR-003 needs a card unit. User wants pending/in-progress/done in parallel and to know which agent to load in gamedev-skill (ADR-004). state.md lists only non-idle agents; done/idle drop off.
decision: One card per owned filesystem artifact that exists, plus Game inbox grill epics. Pre-prod → phase docs (gdd, style-guide, tech-architecture, audio-direction, production-plan, narrative-bible if present). Production → each `systems/SYS-*` folder, each `levels/LVL-*` folder, each art/audio/ui/animation asset folder, playtest-log. Post-prod → optimization / platform-integration / release-plan / marketing-plan / postmortem if present. Owning agent shown (agents.md map). No placeholder cards for missing files. Not one card per agent. Not one card per state.md line.
why: agents.md already maps owner→file; _core.agent stamps `status:` on that doc; Track A stamps the spec folder. Agent-line cards would lose pending and done. One card per agent would hide many SYS-* under @gameplay-engineer. Missing-file placeholders contradict filesystem-as-memory.
alternatives: [one card per agent, one card per state.md line, missing-file placeholders, roadmap-epic as the only card]
irreversible-because: Operators learn “card = file/folder on disk.” Switching to agent-lines later would make done work vanish from the board.
epics: [002]
