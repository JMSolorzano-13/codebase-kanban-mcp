/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-016 - list canonical_root is the skip/group key
 * @sdd-why: Lock findNewestForPath so create skip matches Dashboard newest
 * @human-debug: If skip misses trailing slash → foldPathKey; if older name wins → pickNewest
 */
import { describe, expect, it } from "vitest";
import { findNewestForPath, groupKey, groupProjects, pickNewest } from "./pathGroups";
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

  it("finds newest owner for a listed Path including trailing slash", () => {
    const older = p({ name: "alpha-old", indexed_at: "2026-08-28T10:00:00Z", canonical_root: "/tmp/alpha" });
    const newer = p({ name: "alpha", indexed_at: "2026-08-29T10:00:00Z", canonical_root: "/tmp/alpha" });
    expect(findNewestForPath([older, newer], "/tmp/alpha/")?.name).toBe("alpha");
    expect(findNewestForPath([newer], "/tmp/other")).toBeUndefined();
  });
});
