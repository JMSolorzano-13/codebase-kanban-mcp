adr: 002
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: Current theme is teal-green on near-black (globals.css primary #1DA27E, bg #0a161a). User wants executive look.
decision: Chrome only (Dashboard, workspace chrome, tabs, buttons, links, Control, headers, modals). Dark grayscale with contrast between surfaces. Semantic red/amber kept for error/health. 3D graph node/edge label colors stay categorical (ADR-011).
why: User Q1: solo chrome. Green/teal chrome goes; graph remains readable.
alternatives: [full grayscale including 3D, teal accent on gray]
irreversible-because: Token rewrite across graph-ui chrome. 3D palette explicitly out.
epics: []
