adr: 002
plan: spec-board-detail
date: 2026-08-29
status: accepted

context: User chose the click-detail pattern after seeing what today's active-spec expand already does.
decision: Reuse SpecCard in-card expand (same card grows; no floating overlay/popover/modal). Extend it to todo / in_progress / done, not only active.
why: User 2026-08-29: "Reusamos ese expand". Matches "no interfaz complicada". Pattern already in SpecBoardTab.
alternatives: [floating popover over the card, side panel, separate spec page]
irreversible-because: Locks the interaction model operators will learn; a later overlay would be a second pattern on the same board.
epics: [epic-001]
