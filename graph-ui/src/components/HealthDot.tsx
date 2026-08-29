/**
 * @sdd-task: Task #4 - Dashboard page: list + Control + create-index
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-005 - Chrome grayscale; lock colorForLabel and EdgeLines hex
 * @sdd-why: Extract health chrome so Dashboard can drop schema dumps; semantic hex stays
 * @human-debug: If tooltip/dot is teal-black → leftover hardcoded panel hex; dots must stay #34d399/#fbbf24/#f87171
 */
import { useEffect, useState } from "react";
import { useUiMessages } from "../lib/i18n";

interface HealthPayload {
  status?: "loading" | "healthy" | "corrupt" | "missing";
  nodes?: number;
  edges?: number;
  size_bytes?: number;
  reason?: string;
}

export function HealthDot({ name }: { name: string }) {
  const t = useUiMessages();
  const [status, setStatus] = useState<"loading" | "healthy" | "corrupt" | "missing">("loading");
  const [info, setInfo] = useState("");

  useEffect(() => {
    let cancelled = false;
    const load = async () => {
      try {
        const res = await fetch(`/api/project-health?name=${encodeURIComponent(name)}`);
        const d = (await res.json()) as HealthPayload;
        if (cancelled) return;
        setStatus(d.status ?? "corrupt");
        if (d.nodes !== undefined) {
          const sizeMB = ((d.size_bytes ?? 0) / 1024 / 1024).toFixed(1);
          setInfo(`${d.nodes.toLocaleString()} nodes, ${d.edges?.toLocaleString() ?? "0"} edges, ${sizeMB} MB`);
        } else if (d.reason) {
          setInfo(d.reason);
        }
      } catch {
        if (!cancelled) setStatus("corrupt");
      }
    };
    void load();
    return () => {
      cancelled = true;
    };
  }, [name]);

  const dotColor =
    status === "healthy" ? "#34d399" :
    status === "missing" ? "#fbbf24" :
    status === "corrupt" ? "#f87171" : "#555";

  const label =
    status === "healthy" ? t.projects.healthHealthy :
    status === "missing" ? t.projects.healthMissing :
    status === "corrupt" ? t.projects.healthCorrupt : t.projects.healthChecking;

  return (
    <div className="group relative inline-flex items-center">
      <span
        className="absolute w-3 h-3 rounded-full animate-pulse opacity-40 blur-[3px]"
        style={{ backgroundColor: dotColor }}
      />
      <span
        className="relative w-[8px] h-[8px] rounded-full"
        style={{ backgroundColor: dotColor, boxShadow: `0 0 6px ${dotColor}80` }}
      />
      <div className="absolute bottom-full left-1/2 -translate-x-1/2 mb-3 hidden group-hover:block z-20 pointer-events-none">
        <div className="bg-card border border-border/50 rounded-lg px-3 py-2 text-[11px] whitespace-nowrap shadow-xl">
          <p className="font-medium" style={{ color: dotColor }}>{label}</p>
          {info && <p className="text-foreground/35 text-[10px] mt-0.5">{info}</p>}
        </div>
      </div>
    </div>
  );
}
