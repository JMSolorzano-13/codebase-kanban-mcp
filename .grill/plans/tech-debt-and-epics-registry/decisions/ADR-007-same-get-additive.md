adr: 007
plan: tech-debt-and-epics-registry
date: 2026-09-01
status: accepted

context: GET /api/spec-board and GET /api/game-board already feed the two boards. Prior add-epics ADR-008 kept one GET additive. New routes would split chrome from the board payload the UI already fetches.
decision: Same two GETs. Additive fields for open-debt lists and any registry-derived hide already applied server-side (Inbox cards omitted, not a client filter). No new HTTP resource for debt or registry.
why: One fetch per board; hide stays a server read of the filesystem (same as conversion today).
alternatives: [GET /api/tech-debt, client reads registry, MCP-only]
irreversible-because: Extra routes or client-side hide would fork the board contract the UI and Gherkin already assume.
epics: [001, 002, 003]
