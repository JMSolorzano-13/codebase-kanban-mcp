adr: 003
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: User confirmed expand body after recommendation on objective source + per-column task list.
decision: Expand always includes 1-2 sentence blurb from spec.md ## Executive Summary (not KPI). Todo: pending tasks only (empty → existing no-tasks copy). In progress: all tasks + current/done/pending status + blurb. Done: full task list + Archive button.
why: User 2026-08-29 accepted recommendation. Executive Summary is the spec objective; KPI is a metric. Matches original column rules.
alternatives: [KPI line as blurb, full Executive Summary, todo shows all tasks, Done archive-only with no task list]
irreversible-because: Parser must read spec.md body for every column (today only active H1); changing the blurb source later is a second parse contract.
epics: [epic-001]
