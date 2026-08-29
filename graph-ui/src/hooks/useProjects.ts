/**
 * @sdd-task: Task #1 - useProjects list-only
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-006 - useProjects is list_projects only
 * @sdd-why: Dashboard list paint must not call get_graph_schema (TD-001)
 * @human-debug: If list stays empty after a live daemon → fetchProjects catch (RPC/HTTP) or result.projects missing
 */
import { useCallback, useEffect, useState } from "react";
import { callTool } from "../api/rpc";
import type { Project } from "../lib/types";

interface UseProjectsResult {
  projects: Project[];
  loading: boolean;
  error: string | null;
  refresh: () => void;
}

export function useProjects(): UseProjectsResult {
  const [projects, setProjects] = useState<Project[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchProjects = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const result = await callTool<{ projects: Project[] }>("list_projects");
      setProjects(result.projects ?? []);
    } catch (e) {
      setError(e instanceof Error ? e.message : "Failed to fetch projects");
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    void fetchProjects();
  }, [fetchProjects]);

  return { projects, loading, error, refresh: fetchProjects };
}
