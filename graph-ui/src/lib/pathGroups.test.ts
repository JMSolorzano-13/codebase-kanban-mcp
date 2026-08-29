/**
 * @sdd-task: Task #3 - Dashboard conflict group + Enter newest + delete older
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-016 - group on list canonical_root; newest matches C catalog
 * @sdd-why: Lock group key fallback and newest/tie so Dashboard Enter cannot drift from C
 * @human-debug: If tie picks alpha over beta → JS compare is not greater-name; if slash clones split → key used root_path while canonical_root was set
 */
import { describe, expect, it } from "vitest";
import { groupKey, groupProjects, pickNewest } from "./pathGroups";
import type { Project } from "./types";

function p(partial: Partial<Project> & Pick<Project, "name">): Project {
  return {
    root_path: "/tmp/alpha",
    indexed_at: "2026-08-29T10:00:00Z",
    ...partial,
  };
}

describe("pathGroups", () => {
  it("groups by canonical_root when present even if stored root_path differs", () => {
    const groups = groupProjects([
      p({ name: "slash", root_path: "/tmp/alpha/", canonical_root: "/tmp/alpha" }),
      p({ name: "alpha", root_path: "/tmp/alpha", canonical_root: "/tmp/alpha" }),
      p({ name: "beta", root_path: "/tmp/beta", canonical_root: "/tmp/beta" }),
    ]);
    expect(groups).toHaveLength(2);
    expect(groups[0]?.members.map((m) => m.name)).toEqual(["slash", "alpha"]);
    expect(groups[1]?.members.map((m) => m.name)).toEqual(["beta"]);
  });

  it("falls back to root_path when canonical_root is absent", () => {
    expect(groupKey(p({ name: "alpha", root_path: "/tmp/alpha" }))).toBe("/tmp/alpha");
    const groups = groupProjects([
      p({ name: "alpha-old", root_path: "/tmp/alpha", indexed_at: "2026-08-28T10:00:00Z" }),
      p({ name: "alpha", root_path: "/tmp/alpha", indexed_at: "2026-08-29T10:00:00Z" }),
    ]);
    expect(groups).toHaveLength(1);
    expect(groups[0]?.members).toHaveLength(2);
  });

  it("picks newest by indexed_at string, then greater name on tie", () => {
    const older = p({ name: "alpha-old", indexed_at: "2026-08-28T10:00:00Z" });
    const newer = p({ name: "alpha", indexed_at: "2026-08-29T10:00:00Z" });
    expect(pickNewest([older, newer]).name).toBe("alpha");
    expect(pickNewest([newer, older]).name).toBe("alpha");

    const alpha = p({ name: "alpha", indexed_at: "2026-08-29T10:00:00Z" });
    const beta = p({ name: "beta", indexed_at: "2026-08-29T10:00:00Z" });
    expect(pickNewest([alpha, beta]).name).toBe("beta");
    expect(pickNewest([beta, alpha]).name).toBe("beta");
  });
});
