plan: tech-debt-and-epics-registry
date: 2026-09-01
skill: /Users/jmsolorzano/SWE/_SKILLs/sdd-skill/source/sdd-skill
inspected: SKILL.md, README.md, references/filesystem.md, references/templates/TECH_DEBT.md, prompt-planner.agent, prompt-architect.agent

layout:
- path: .sdd-skill/baseline/TECH_DEBT.md
- owner-create: @architect on init --adopt (codebase scan)
- owner-ongoing: @planner (user NL "Add technical debt: …", or new debt at spec close)
- not-a-queue: one item → one spec; @planner offers open items when starting features

item-shape:
- heading: ## TD-NNN: [Title]
- fields: ID / Category / Severity / Status / Identified / Origin / Spec / Resolved In
- status: identified → deferred → in_progress → resolved
- severity: critical|high|medium|low
- category: architecture|security|performance|testing|code-quality|dependencies|ux
- origin: adopt|spec-NNN|user report
- body: Description, Why Deferred, Acceptance Criteria (future spec)
- summary-table: | ID | Title | Category | Severity | Status |
- notes: project-wide patterns

this-repo-example:
- CBM .sdd-skill/baseline/TECH_DEBT.md exists; TD-001..004 all Status=resolved
- CBM has zero parser/UI for TECH_DEBT.md (search_code: 0 hits)
