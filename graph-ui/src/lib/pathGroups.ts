/**
 * @sdd-task: Task #3 - Dashboard conflict group + Enter newest + delete older
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-016 - group on list canonical_root; newest matches C catalog
 * @sdd-why: Legacy same-Path clones must fold in UI without a JS realpath
 * @human-debug: If two clones stay as solo rows → canonical_root/root_path key mismatch; if Enter opens the older → pickNewest strcmp drifted from identity.c
 */
import type { Project } from "./types";

export interface PathGroup {
  key: string;
  members: Project[];
  newest: Project;
}

/** Match cbm_identity_cmp_newest: strcmp indexed_at, then strcmp name. */
export function cmpNewest(a: Project, b: Project): number {
  const timeA = a.indexed_at ?? "";
  const timeB = b.indexed_at ?? "";
  if (timeA !== timeB) {
    return timeA < timeB ? -1 : 1;
  }
  const nameA = a.name ?? "";
  const nameB = b.name ?? "";
  if (nameA === nameB) return 0;
  return nameA < nameB ? -1 : 1;
}

export function pickNewest(members: readonly Project[]): Project {
  if (members.length === 0) {
    throw new Error("pickNewest: empty group");
  }
  return members.reduce((best, p) => (cmpNewest(p, best) > 0 ? p : best));
}

export function groupKey(p: Project): string {
  return p.canonical_root || p.root_path;
}

/** First-seen key order so Dashboard list order stays stable. */
export function groupProjects(projects: readonly Project[]): PathGroup[] {
  const buckets = new Map<string, Project[]>();
  const order: string[] = [];
  for (const p of projects) {
    const key = groupKey(p);
    const existing = buckets.get(key);
    if (existing) {
      existing.push(p);
    } else {
      buckets.set(key, [p]);
      order.push(key);
    }
  }
  return order.map((key) => {
    const members = buckets.get(key) ?? [];
    return { key, members, newest: pickNewest(members) };
  });
}
