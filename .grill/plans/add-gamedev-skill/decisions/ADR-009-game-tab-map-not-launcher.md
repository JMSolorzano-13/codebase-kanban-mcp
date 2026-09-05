adr: 009
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: User asked CBM to propose how to use gamedev-skill. ADR-004: CBM does not advance the cycle. Skill entry: `/gamedev-skill continue [@agent]` and `agent @<role>`. CBM cannot dispatch IDE subagents.
decision: Game tab is a map, not a launcher. Chrome shows `state.md` phase + focus and `/gamedev-skill continue`. Each artifact card shows owning agent and a copyable `/gamedev-skill continue @<role>`. Grill inbox copies `/gamedev-skill continue` (no invented specialist; @director converts). Current-phase chrome may mention `/gamedev-skill gate review` — not a CBM button. No invoke, no paste into the IDE, no full command catalog on the card.
why: continue[@agent] is the skill’s resume path; operator confirms done in chat as requested. Catalog (spec/review/bug/task/playtest) belongs in the skill. Fake @role on inbox would imply conversion already happened.
alternatives: [CBM launches the skill, copy agent @role only, dump full command list, no commands just agent name]
irreversible-because: Operators learn “copy continue @role from the card.” A later in-app Run would be a second control surface against ADR-004.
epics: [001, 002]
