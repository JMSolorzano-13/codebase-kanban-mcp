adr: 004
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: epics_registry.md is lazy/conditional — many games will not have the file yet. Prior Game hide = Companion-to exact OR roadmap slug+NNN (add-gamedev-skill ADR-006). User said registry is the new visualization when that form exists.
decision: File present (even if no matching rows) → ADR-003 is the only hide rule; prior ADR-006 does not also hide. File absent → prior ADR-006 unchanged. CBM never creates the file.
why: Two hide sources at once would drop unlisted-but-roadmap-mapped leftovers the user asked to still see as “faltan”. Absent file must not empty or freeze Inbox on older games.
alternatives: [always AND both rules, always OR both, ignore ADR-006 forever, CBM writes registry]
irreversible-because: Sole-source vs additive changes which leftovers survive; flipping re-floods or hides converted work.
epics: [002]
