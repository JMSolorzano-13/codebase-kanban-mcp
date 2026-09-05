/**
 * @sdd-task: Task #1 - XOR trio select + gamedev relatives
 * @sdd-spec: specs/spec-013-r9w-adr-fill-gamedev-trio/spec.md
 * @sdd-decision: SDD-ADR-058 XOR; SDD-ADR-059 local is-dir; SDD-ADR-060 NULL=neither dir; SDD-ADR-061 no ALWAYS_SKIP
 * @sdd-why: XOR trio: .gamedev/ dir wins; else .sdd-skill/; neither dir is NULL (blob unchanged)
 * @human-debug: NULL means neither .gamedev/ nor .sdd-skill/ is a directory (file-at-path .gamedev
 * is not present). Empty .gamedev/ dir still marks. OOM is also NULL. Unmarked text missing
 * after fill means the body already had MANUAL markers so only that span was kept
 */
#ifndef CBM_ADR_FILL_H
#define CBM_ADR_FILL_H

#include "foundation/constants.h"

#define CBM_ADR_GENERATED_START "<!-- CBM-GENERATED-START -->"
#define CBM_ADR_GENERATED_END "<!-- CBM-GENERATED-END -->"
#define CBM_ADR_MANUAL_START "<!-- CBM-MANUAL-START -->"
#define CBM_ADR_MANUAL_END "<!-- CBM-MANUAL-END -->"

/* 1536 — first window after a 64KiB read (SDD-ADR-020). */
enum { CBM_ADR_EXTRACT_MAX = CBM_SZ_1K + CBM_SZ_512 };

/*
 * Splice a generated trio extract in front of the manual region.
 * No store and no HTTP. Caller frees a non-NULL result.
 *
 * Neither .gamedev/ nor .sdd-skill/ is a directory: returns NULL (existing is unchanged).
 * .gamedev/ is a directory: gamedev trio only (never opens .sdd-skill/ trio paths).
 * Else .sdd-skill/ is a directory: existing sdd trio.
 * Empty skill dir: marked document with no extracts. existing may be NULL (empty ADR).
 * Unmarked existing becomes the manual region. NULL on allocation failure.
 */
char *cbm_adr_fill_document(const char *root_path, const char *existing);

#endif /* CBM_ADR_FILL_H */
