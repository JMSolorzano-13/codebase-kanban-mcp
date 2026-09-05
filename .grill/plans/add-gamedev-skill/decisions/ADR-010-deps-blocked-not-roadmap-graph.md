adr: 010
plan: add-gamedev-skill
date: 2026-08-30
status: accepted

context: User asked the Game board to show parallel work and dependencies. Live deps in gamedev are `state.md` agent lines `blocked:"task":"blocked-by"`. Track A spec.md has Inputs. roadmap.md Dependencies is freeform ASCII, not a schema. Graph tab already shows code.
decision: Live deps = `blocked` + `blocked-by` from state.md on that owning agent’s artifact cards, plus a board strip of all current blocked lines. Track A expand also shows spec.md Inputs. Do not parse roadmap.md into a graph. Do not paint agents.md Needs or the skill INPUT→OUTPUT map as edges.
why: blocked-by is the only stable per-project contract (state.md template). Roadmap ASCII would break on format drift and duplicate Graph. agents.md Needs is role-static. Inputs is per-system and already in Track A expand.
alternatives: [roadmap graph layout, Needs as edges, no blocked strip, parse filesystem.md map]
irreversible-because: Operators learn “blocked strip + badge = deps.” A later roadmap graph is a second dep UI. Parsing freeform MD becomes an unofficial schema for @director.
epics: [003]
