adr: 001
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: Both skills already write a debt register. User wants it on Specs/Game. Mixing TD/debt:* into Todo/Inbox would collide with grill-epic cards (kind E) and conversion hide rules. A new Kanban column would redesign both boards (prior plans locked 3-col Specs and 4-col Game).
decision: Open debt is a chrome list on the matching board (Specs ← TECH_DEBT.md, Game ← backlog.md debt:*). Not a column. Not a card in Todo/Inbox. Not kind E. Not expandable/archivable. Missing file or zero open items → omit the list (no empty header).
why: Operator sees debt without changing column semantics or conversion. Same “chrome not axis” pattern as last-indexed / Show Dones.
alternatives: [4th column Debt, mix debt cards into Todo/Inbox, dedicated Debt tab]
irreversible-because: Column vs chrome is a board contract; flipping later redoes IA and any GET shape that assumed lists not cards.
epics: [001, 003]
