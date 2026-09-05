/**
 * @sdd-task: Task #1 - spec_board debt parse + additive JSON
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-065 always-emit debt[{id,title}] cap 16; SDD-ADR-066 parse in spec_board.c heading Status wins
 * @sdd-why: Same GET; open TD-NNN only; conversion matcher untouched; own debt cap independent of specs/epics
 * @human-debug: If open TD rows missing → heading Status: / Debt Summary fallback missed or token is exact resolved
 *
 * spec_board.h — Read-only "spec board" (Kanban) derived from .sdd-skill/
 * and, additively, unconverted grill epics under .grill/ plus open TECH_DEBT.md.
 *
 * Deliberately zero-write: fopen "rb" only. Never writes .grill/ or .sdd-skill/.
 * Grill fill runs even when sdd is absent. Debt fill is independent of sdd too.
 * Conversion scratch (companion_grill, source.grill_epic) is not JSON.
 * Heap-only for cbm_spec_board_t.
 */
#ifndef CBM_UI_SPEC_BOARD_H
#define CBM_UI_SPEC_BOARD_H

#include <stdbool.h>
#include <stddef.h>

/* Caps stay small on purpose: one board struct is still multi-hundred KB and
 * must never live on the stack (see handle_spec_board / tests). sdd-skill is
 * sequential — dozens of historical specs and a few dozen tasks per spec is
 * already generous. */
#define CBM_SPEC_BOARD_MAX_SPECS 64
#define CBM_SPEC_BOARD_MAX_EPICS 64
#define CBM_SPEC_BOARD_MAX_TASKS 48
#define CBM_SPEC_BOARD_MAX_DEBT 16
#define CBM_SPEC_BOARD_BLURB_MAX 512

typedef struct {
    int number;         /* Task #N */
    char name[256];      /* from "### Task #N — [Name]" heading, "" if unknown */
    bool done;            /* last matching history/test_results.log line:
                           * active = bare Task #N (PASS sets, else unset);
                           * non-active = line must contain spec id + Task #N */
    bool current;         /* matches state.md's active task number */
} cbm_spec_task_t;

typedef struct {
    char id[192];             /* folder name, e.g. spec-002-authentication */
    char title[256];          /* spec.md "# Spec-NNN: [Name]" header, "" if not read */
    char blurb[CBM_SPEC_BOARD_BLURB_MAX]; /* 1–2 ES sentences; "" if not extracted */
    char column[16];          /* "todo" | "in_progress" | "done" */
    bool archived;              /* HTTP merge only; cbm_spec_board_read leaves 0 */
    bool active;                /* true only for active.json:active_spec */
    char current_agent[32];    /* state.md:role, only set when active==true */
    char blocked_note[256];    /* state.md notes, only when it looks like a blocker */
    cbm_spec_task_t tasks[CBM_SPEC_BOARD_MAX_TASKS];
    int task_count;
    int tasks_done;
    double checklist_percent;  /* checklist.md "## Feature Status" TOTAL row, -1 if absent */
    char companion_grill[256]; /* first Companion-to .grill/plans/…md; not JSON */
} cbm_spec_board_entry_t;

/* kind is conceptually "epic" (always emitted as that string; not stored). */
typedef struct {
    char id[256];              /* .grill/plans/<slug>/epics/epic-NNN-<name>.md */
    char title[256];           /* epic.md name: */
    char summary[512];         /* epic.md summary: (not spec blurb extract) */
    char plan_title[256];      /* index.md title else plan.md title else slug */
    char column[16];           /* always "todo" */
} cbm_spec_board_epic_t;

typedef struct {
    char id[32];               /* TD- + digits from ## TD-NNN */
    char title[256];           /* trimmed remainder after ## TD-NNN: ; not Title: */
} cbm_spec_board_debt_t;

typedef struct {
    bool sdd_skill_present;     /* false when root_path has no .sdd-skill/ at all */
    bool grill_skill_present;   /* true iff root_path/.grill exists as a directory */
    cbm_spec_board_entry_t specs[CBM_SPEC_BOARD_MAX_SPECS];
    int spec_count;
    cbm_spec_board_epic_t epics[CBM_SPEC_BOARD_MAX_EPICS];
    int epic_count;
    cbm_spec_board_debt_t debt[CBM_SPEC_BOARD_MAX_DEBT];
    int debt_count;             /* open items only; cap 16; resolved does not consume */
} cbm_spec_board_t;

/* Reads .sdd-skill/, .grill/, and TECH_DEBT.md under root_path and fills *out.
 * Always succeeds (best-effort). Grill and debt fill are independent of sdd. */
void cbm_spec_board_read(const char *root_path, cbm_spec_board_t *out);

/* Serialize *b to a JSON object string. Caller must free() the result.
 * Returns NULL only on allocation failure. Additive keys: grill_skill_present,
 * epics[], and always-present debt[]. Spec objects have no "kind".
 * Never emits has_more. */
char *cbm_spec_board_to_json(const cbm_spec_board_t *b);

/* Best-effort ## Executive Summary → 1–2 sentences into out (SDD-ADR-025).
 * NULL or empty spec_md → out[0] = 0. Does not open files. */
void cbm_spec_board_extract_blurb(const char *spec_md, char *out, size_t outsz);

/* Open TECH_DEBT.md items from an in-memory buffer (SDD-ADR-066).
 * NULL or empty md → *count = 0. Does not open files. */
void cbm_spec_board_parse_tech_debt(const char *md, cbm_spec_board_debt_t *out, int *count);

/* Cheap presence check reused by the tab-visibility endpoint — does
 * root_path/.sdd-skill exist as a directory? */
bool cbm_spec_board_sdd_skill_present(const char *root_path);

/* true iff root_path/.grill exists as a directory. File-at-path or missing → false. */
bool cbm_spec_board_grill_skill_present(const char *root_path);

/* Same check for gamedev-skill's .gamedev/ — wired only to /api/skill-presence.
 * spec-board read/to_json must not call this or emit gamedev_skill_present. */
bool cbm_spec_board_gamedev_skill_present(const char *root_path);

#endif /* CBM_UI_SPEC_BOARD_H */
