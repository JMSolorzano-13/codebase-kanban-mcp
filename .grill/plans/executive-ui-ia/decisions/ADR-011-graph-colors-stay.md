adr: 011
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: ADR-002 left 3D node hues open. User chose chrome-only grayscale.
decision: GraphTab 3D nodes/edges keep existing categorical colors (colorForLabel etc.). Theme change does not recolor the galaxy.
why: User Q1. Grayscale nodes would hide Function/Class/Route distinctions.
alternatives: [desaturate graph, grayscale + shape encoding]
irreversible-because: Splits design system: chrome tokens ≠ graph palette. Agents must not "fix" graph colors to match Dashboard.
epics: []
