# glossary — spec-board-detail

- Spec board: existing 3-col Kanban in workspace Specs tab; read of .sdd-skill/ via GET /api/spec-board. (source: inspection SpecBoardTab + spec_board.h)
- Spec card: one SpecBoardEntry in a column. Click-detail is the existing in-card expand, extended to all columns (ADR-002). Several cards may stay open (ADR-008). Today only active && has tasks can expand. (source: SpecBoardTab + ADR-002 + ADR-008)
- Spec objective blurb: first 1-2 sentences of spec.md ## Executive Summary (not KPI). Omit if missing/empty; do not invent. Card still expands. (source: ADR-003 + ADR-006)
- Archive (spec): CBM-owned flag on a Done spec. Hidden by default; show/hide is session-only; still sits in Done; Unarchive from the same expand. No confirm dialog (ADR-009). No skill file change. (source: ADR-001 + ADR-004 + ADR-007 + ADR-009)
- Last indexed: Project.indexed_at ISO via formatIndexedAt. Display = browser local TZ on Dashboard, workspace header, and AdrTab stamp. Storage stays Z. (source: ADR-005)
- Zero-write-to-skills: this plan may persist archive in CBM and may read more skill files; it never writes skill trees. spec_board.c stays a skill-file reader. (source: ADR-001 + spec_board.h)
