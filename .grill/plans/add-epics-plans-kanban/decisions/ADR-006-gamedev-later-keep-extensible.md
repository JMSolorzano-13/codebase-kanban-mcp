adr: 006
plan: add-epics-plans-kanban
date: 2026-08-30
status: accepted

context: User excluded gamedev from this plan but said it WILL apply later. One path holding sdd AND gamedev is undecided — must not be decided here.
decision: This plan does not read `.gamedev/`, does not paint gamedev cards, does not hide epics via a gamedev companion. A later plan owns that. Presence and entry kind must stay additive (sdd_skill_present + grill_skill_present; kind spec|epic) so a third skill/kind is not a rewrite. Do not assume a path cannot have both sdd and gamedev.
why: User 2026-08-30: hide gamedev for now, another plan, but be prepared because it will apply. Path dual-skill decision is out of this interview.
alternatives: [implement gamedev now, hardcode sdd-xor-gamedev, boolean is_epic only]
irreversible-because: A closed sdd-vs-gamedev identity here would force the later plan to fight this board's presence/kind shape.
epics: [epic-001, epic-002]
