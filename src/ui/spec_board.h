/*
 * spec_board.h — Read-only "spec board" (Kanban) derived from an sdd-skill
 * project's .sdd-skill/ folder.
 *
 * Deliberately zero-write: every field here is computed by reading files
 * sdd-skill's own cycle already produces (active.json, tasks.md, state.md,
 * history/test_results.log, checklist.md). No new file, no new field, no
 * change requested from the skill's agents (see docs/kanban-adapter-contract.md
 * once written). Parsing is best-effort/tolerant throughout: a missing or
 * malformed file degrades a field to its zero value rather than failing the
 * whole read — same posture as the rest of the UI support endpoints
 * (project-health, adr, repo-info).
 */
#ifndef CBM_UI_SPEC_BOARD_H
#define CBM_UI_SPEC_BOARD_H

#include <stdbool.h>

/* Caps stay small on purpose: one board struct is still multi-hundred KB and
 * must never live on the stack (see handle_spec_board / tests). sdd-skill is
 * sequential — dozens of historical specs and a few dozen tasks per spec is
 * already generous. */
#define CBM_SPEC_BOARD_MAX_SPECS 64
#define CBM_SPEC_BOARD_MAX_TASKS 48

typedef struct {
    int number;         /* Task #N */
    char name[256];      /* from "### Task #N — [Name]" heading, "" if unknown */
    bool done;            /* most recent history/test_results.log entry for this
                           * task number is PASS (best-effort — see .c for the
                           * known cross-spec numbering caveat) */
    bool current;         /* matches state.md's active task number */
} cbm_spec_task_t;

typedef struct {
    char id[192];             /* folder name, e.g. spec-002-authentication */
    char title[256];          /* spec.md "# Spec-NNN: [Name]" header, "" if not read */
    char column[16];          /* "todo" | "in_progress" | "done" */
    bool active;                /* true only for active.json:active_spec */
    char current_agent[32];    /* state.md:role, only set when active==true */
    char blocked_note[256];    /* state.md notes, only when it looks like a blocker */
    cbm_spec_task_t tasks[CBM_SPEC_BOARD_MAX_TASKS];
    int task_count;
    int tasks_done;
    double checklist_percent;  /* checklist.md "## Feature Status" TOTAL row, -1 if absent */
} cbm_spec_board_entry_t;

typedef struct {
    bool sdd_skill_present;     /* false when root_path has no .sdd-skill/ at all */
    cbm_spec_board_entry_t specs[CBM_SPEC_BOARD_MAX_SPECS];
    int spec_count;
} cbm_spec_board_t;

/* Reads .sdd-skill/ under root_path and fills *out. Always succeeds (out is
 * zero-valued / sdd_skill_present=false when the folder is absent or
 * unreadable) — this is a best-effort UI support read, not a hard API. */
void cbm_spec_board_read(const char *root_path, cbm_spec_board_t *out);

/* Serialize *b to a JSON object string. Caller must free() the result.
 * Returns NULL only on allocation failure. */
char *cbm_spec_board_to_json(const cbm_spec_board_t *b);

/* Cheap presence check reused by the tab-visibility endpoint — does
 * root_path/.sdd-skill exist as a directory? */
bool cbm_spec_board_sdd_skill_present(const char *root_path);

/* Same check for gamedev-skill's .gamedev/ — stubbed out for the future
 * gamedev tab (Fase 2 of the wider plan); always false until that adapter
 * is written, so the endpoint can already report the shape it will have. */
bool cbm_spec_board_gamedev_skill_present(const char *root_path);

#endif /* CBM_UI_SPEC_BOARD_H */
