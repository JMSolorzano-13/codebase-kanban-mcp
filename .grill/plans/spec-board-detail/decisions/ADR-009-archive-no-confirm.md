adr: 009
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: Archive + default-hide can look like delete; user chose no confirm modal.
decision: Archive is immediate (no confirm dialog). Recovery is session toggle "show archived" then Unarchive.
why: User 2026-08-29 accepted recommendation. Light UI (original ask); Unarchive already in scope (ADR-004).
alternatives: [window.confirm, undo toast]
irreversible-because: Adding a confirm later fights the "poca información" bar already set for this board.
epics: [epic-002]
