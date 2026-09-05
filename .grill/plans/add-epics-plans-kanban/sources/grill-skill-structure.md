# source: grill-skill filesystem — inspected
date: 2026-08-30
path: /Users/jmsolorzano/SWE/_SKILLs/grill-skill/source/grill-skill
example: /Users/jmsolorzano/SWE/tools/codebase-memory-mcp/.grill

tree:
  .grill/index.md — catalog: slug, title, status draft|closed, entry_type
  .grill/plans/<slug>/plan.md — human prose
  .grill/plans/<slug>/epics/epic-NNN-<name>.md — shorthand: name, summary, status pending|detailed, plan slug
  glossary.md, decisions/ADR-NNN, state.md, sources/

facts:
  many independent plans per project
  epic never deleted on sdd pickup; file stays
  example repo: 2 closed plans, 7 epics, all already specs 001-007
  epic card fields available without inventing: name, summary, parent plan slug/title (index.md or plan.md frontmatter)
