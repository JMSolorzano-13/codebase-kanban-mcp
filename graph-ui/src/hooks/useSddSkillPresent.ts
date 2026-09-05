/**
 * @sdd-task: Task #1 - Presence predicate sdd OR grill
 * @sdd-spec: specs/spec-009-t4x-specs-tab-grill-presence/spec.md
 * @sdd-decision: SDD-ADR-039 - Keep useSddSkillPresent; present is 200 and sdd OR grill
 * @sdd-why: Strip membership is HTTP 200 and (sdd_skill_present === true OR grill_skill_present === true); missing grill is false
 * @human-debug: If Specs missing on grill-only → bodyHasSkill (line 17) still sdd-only; if Specs shows for neither → a flag was not === true; if interval fires → hook reused useSpecBoard
 */
import { useEffect, useState } from "react";

export interface UseSddSkillPresentResult {
  present: boolean;
}

function bodyHasSkill(data: unknown): boolean {
  if (typeof data !== "object" || data === null) return false;
  const body = data as { sdd_skill_present?: unknown; grill_skill_present?: unknown };
  return body.sdd_skill_present === true || body.grill_skill_present === true;
}

export function useSddSkillPresent(project: string | null): UseSddSkillPresentResult {
  const [present, setPresent] = useState(false);

  useEffect(() => {
    if (!project) {
      setPresent(false);
      return;
    }

    let cancelled = false;
    setPresent(false);

    void (async () => {
      try {
        const res = await fetch(`/api/spec-board?project=${encodeURIComponent(project)}`);
        if (cancelled) return;
        if (res.status !== 200) {
          setPresent(false);
          return;
        }
        const data: unknown = await res.json();
        if (!cancelled) setPresent(bodyHasSkill(data));
      } catch {
        if (!cancelled) setPresent(false);
      }
    })();

    return () => {
      cancelled = true;
    };
  }, [project]);

  return { present };
}
