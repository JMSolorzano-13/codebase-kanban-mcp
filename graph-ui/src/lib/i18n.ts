/**
 * @sdd-task: Task #4 - Create modal path_exists redirect + notice
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-015 - bare POST is create; path_exists notice is not a reindex
 * @sdd-why: US-001 status notice must name the existing project in en+zh
 * @human-debug: If Graph has no English notice → messages.en.index.pathExistsNotice missing
 */
import { useEffect, useState } from "react";

export type UiLanguage = "en" | "zh";

export const messages = {
  en: {
    tabs: {
      specs: "Specs",
      graph: "Graph",
      adr: "ADR",
      projects: "Projects",
      control: "Control",
    },
    common: {
      cancel: "Cancel",
      refresh: "Refresh",
      loading: "Loading...",
      save: "Save",
      saving: "Saving...",
      delete: "Delete",
      noMatches: "No matches",
      dismiss: "Dismiss",
    },
    graph: {
      selectedLabel: "Graph",
      backToDashboard: "Back to Dashboard",
      search: "Search...",
      clearSelection: "Clear selection",
      folders: "Folders",
    },
    projects: {
      indexedProjects: "Indexed Projects",
      noIndexedProjects: "No indexed projects",
      indexFirstRepository: "Index your first repository",
      enter: "Enter",
      lastIndexed: "Last indexed",
      viewGraph: "View Graph",
      nodes: "nodes",
      edges: "edges",
      deleteTitle: "Delete index",
      deleteNamed: (name: string) => `Delete ${name}`,
      deleteConfirm: (name: string) => `Delete index for "${name}"?`,
      conflict: "Path conflict",
      healthHealthy: "Database healthy",
      healthMissing: "Database missing",
      healthCorrupt: "Database unhealthy",
      healthChecking: "Checking...",
      indexingInProgress: "Indexing in progress",
      indexingFailed: "Indexing failed",
    },
    index: {
      newIndex: "New Index",
      selectRepositoryFolder: "Select Repository Folder",
      instructions: "Navigate to the project root and click \"Index This Folder\".",
      repositoryPath: "Repository path",
      projectName: "Project ID (optional — permanent, cannot be renamed)",
      projectNamePlaceholder: "Derived from folder name if blank",
      projectNameHelp: "Becomes the database name and query prefix. Leave blank to derive it from the path.",
      filterFolders: "Filter folders",
      noSubdirectories: "No subdirectories",
      indexThisFolder: "Index This Folder",
      starting: "Starting...",
      browseRoot: (path: string) => `Browse ${path}`,
      indexDirectory: (name: string) => `Index ${name}`,
      pathExistsNotice: (name: string) => `Already indexed as ${name}`,
    },
    adr: {
      title: "Architecture Decision Record",
      lastUpdated: "Last updated",
      saveSuccess: "ADR saved",
      saveError: "Failed to save ADR",
      unsavedConfirm: "You have unsaved ADR changes. Leave without saving?",
    },
    control: {
      panel: "Control Panel",
      totalCpu: "Total CPU",
      totalRam: "Total RAM",
      processes: "Processes",
      selfRam: "Self RAM",
      activeProcesses: "Active Processes",
      processLogs: "Process Logs",
      noProcesses: "No processes found",
      noLogs: "No logs yet",
      thisProcess: "THIS",
      uptime: "Uptime",
    },
    specBoard: {
      selectProject: "Select a project to see its spec board",
      notSddSkill: "This project doesn't use sdd-skill — nothing to show here",
      columnTodo: "Todo",
      columnInProgress: "In Progress",
      columnDone: "Done",
      pendingCount: (n: number) => `${n} pending`,
      currentAgent: "Working now",
      blocked: "Blocked",
      tasksDone: (done: number, total: number) => `${done}/${total} tasks`,
      checklistLabel: "Checklist",
      noTasksYet: "No tasks planned yet",
      noSpecs: "No specs yet",
    },
  },
  zh: {
    tabs: {
      specs: "规格",
      graph: "图谱",
      adr: "架构决策",
      projects: "项目",
      control: "控制",
    },
    common: {
      cancel: "取消",
      refresh: "刷新",
      loading: "加载中...",
      save: "保存",
      saving: "保存中...",
      delete: "删除",
      noMatches: "无匹配结果",
      dismiss: "关闭",
    },
    graph: {
      selectedLabel: "图谱",
      backToDashboard: "返回仪表盘",
      search: "搜索...",
      clearSelection: "清除选择",
      folders: "目录",
    },
    projects: {
      indexedProjects: "已索引项目",
      noIndexedProjects: "暂无已索引项目",
      indexFirstRepository: "索引第一个仓库",
      enter: "进入",
      lastIndexed: "最近索引",
      viewGraph: "查看图谱",
      nodes: "节点",
      edges: "边",
      deleteTitle: "删除索引",
      deleteNamed: (name: string) => `删除 ${name}`,
      deleteConfirm: (name: string) => `删除 "${name}" 的索引？`,
      conflict: "路径冲突",
      healthHealthy: "数据库正常",
      healthMissing: "数据库缺失",
      healthCorrupt: "数据库异常",
      healthChecking: "检查中...",
      indexingInProgress: "正在索引",
      indexingFailed: "索引失败",
    },
    index: {
      newIndex: "新建索引",
      selectRepositoryFolder: "选择仓库目录",
      instructions: "导航到项目根目录，然后点击“索引此目录”。",
      repositoryPath: "仓库路径",
      projectName: "项目 ID（可选，永久且不可重命名）",
      projectNamePlaceholder: "留空则从路径派生",
      projectNameHelp: "将作为数据库名称与查询前缀；留空则从路径派生。",
      filterFolders: "筛选目录",
      noSubdirectories: "没有子目录",
      indexThisFolder: "索引此目录",
      starting: "启动中...",
      browseRoot: (path: string) => `浏览 ${path}`,
      indexDirectory: (name: string) => `索引 ${name}`,
      pathExistsNotice: (name: string) => `已索引为 ${name}`,
    },
    adr: {
      title: "架构决策记录",
      lastUpdated: "最后更新",
      saveSuccess: "架构决策已保存",
      saveError: "保存架构决策失败",
      unsavedConfirm: "架构决策有未保存的更改。确定不保存并离开？",
    },
    control: {
      panel: "控制面板",
      totalCpu: "总 CPU",
      totalRam: "总内存",
      processes: "进程",
      selfRam: "自身内存",
      activeProcesses: "活动进程",
      processLogs: "进程日志",
      noProcesses: "未找到进程",
      noLogs: "暂无日志",
      thisProcess: "本进程",
      uptime: "运行时间",
    },
    specBoard: {
      selectProject: "选择一个项目以查看其规格看板",
      notSddSkill: "该项目未使用 sdd-skill，此处无内容可显示",
      columnTodo: "待办",
      columnInProgress: "进行中",
      columnDone: "已完成",
      pendingCount: (n: number) => `${n} 个待办`,
      currentAgent: "当前处理",
      blocked: "已阻塞",
      tasksDone: (done: number, total: number) => `${done}/${total} 个任务`,
      checklistLabel: "检查清单",
      noTasksYet: "尚未规划任务",
      noSpecs: "暂无规格",
    },
  },
} as const;

export type UiMessages = (typeof messages)[UiLanguage];

export function detectLanguage(acceptLanguage?: string | null, override?: string | null): UiLanguage {
  if (override === "zh" || override === "en") return override;
  if (!acceptLanguage) return "en";

  // Ranked by q, not by whether "zh" appears anywhere. A substring test served
  // Chinese for "en-US,en;q=0.9,zh;q=0.5", where English is clearly preferred,
  // and for "zh;q=0, en", where q=0 means Chinese is unacceptable.
  const best = acceptLanguage
    .split(",")
    .map((part) => {
      const [tag, ...params] = part.trim().split(";");
      const q = params.map((p) => /^\s*q\s*=\s*([\d.]+)\s*$/i.exec(p)).find(Boolean);
      return { tag: tag.trim().toLowerCase(), q: q ? Number(q[1]) : 1 };
    })
    .filter(({ tag, q }) => tag && Number.isFinite(q) && q > 0)
    .sort((a, b) => b.q - a.q)
    .find(({ tag }) => tag.split("-")[0] === "zh" || tag.split("-")[0] === "en");

  return best?.tag.startsWith("zh") ? "zh" : "en";
}

let cachedLanguage: UiLanguage = "en";
let languageLoaded = false;
let languageRequest: Promise<UiLanguage> | null = null;
const languageListeners = new Set<(lang: UiLanguage) => void>();

function loadUiLanguage(): Promise<UiLanguage> {
  if (languageLoaded) return Promise.resolve(cachedLanguage);
  if (languageRequest) return languageRequest;

  languageRequest = fetch("/api/ui-config")
    .then((r) => r.json())
    .then((data) => detectLanguage(null, data?.lang))
    .catch(() => detectLanguage(navigator.language))
    .then((lang) => {
      cachedLanguage = lang;
      languageLoaded = true;
      for (const listener of languageListeners) listener(lang);
      return lang;
    })
    .finally(() => {
      languageRequest = null;
    });

  return languageRequest;
}

export function useUiLanguage(): UiLanguage {
  const [lang, setLang] = useState<UiLanguage>(cachedLanguage);

  useEffect(() => {
    let cancelled = false;
    languageListeners.add(setLang);
    void loadUiLanguage().then((nextLang) => {
      if (!cancelled) setLang(nextLang);
    });
    return () => {
      cancelled = true;
      languageListeners.delete(setLang);
    };
  }, []);

  return lang;
}

export function useUiMessages(): UiMessages {
  const lang = useUiLanguage();
  return messages[lang];
}
