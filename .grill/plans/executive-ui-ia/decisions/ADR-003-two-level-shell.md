adr: 003
plan: executive-ui-ia
date: 2026-08-29
status: accepted

context: Current IA is four global tabs (Specs default, Graph, Projects, Control). User wants executive entry.
decision: Two-level shell. Level 1 Dashboard = project list + Control on the same screen. Level 2 = enter one project, then scalable tabs.
why: User confirmed Q1=yes and Q2=same screen. Matches "proyectos y control" as home, Graph/Specs/ADR as project interior.
alternatives: [keep global tabs, Dashboard projects only + Control as account tab]
irreversible-because: Replaces App.tsx route model (TabId stats/control/specs/graph as siblings) and default-home (today specs).
epics: []
