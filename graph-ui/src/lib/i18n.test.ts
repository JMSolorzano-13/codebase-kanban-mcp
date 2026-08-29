/**
 * @sdd-task: Task #3 - Dashboard conflict group + Enter newest + delete older
 * @sdd-spec: specs/spec-003-h7q-path-project-identity/spec.md
 * @sdd-decision: SDD-ADR-016 - conflict copy on Dashboard groups
 * @sdd-why: Lock projects.conflict and deleteNamed in en+zh for conflict Delete controls
 * @human-debug: If conflict region has no English name → messages.en.projects.conflict drifted
 */
import { describe, expect, it } from "vitest";
import { detectLanguage, messages } from "./i18n";

describe("i18n", () => {
  it("detects Chinese from Accept-Language and falls back to English", () => {
    expect(detectLanguage("zh-CN,zh;q=0.9,en;q=0.8")).toBe("zh");
    expect(detectLanguage("de-DE,de;q=0.9")).toBe("en");
  });

  it("ranks Accept-Language by q rather than by presence", () => {
    // A bilingual visitor who prefers English but lists Chinese as a fallback.
    expect(detectLanguage("en-US,en;q=0.9,zh;q=0.5")).toBe("en");
    expect(detectLanguage("en,zh;q=0.1")).toBe("en");
    // Chinese still wins when it is actually preferred.
    expect(detectLanguage("zh;q=0.9,en;q=0.5")).toBe("zh");
  });

  it("treats q=0 as unacceptable", () => {
    expect(detectLanguage("zh;q=0, en")).toBe("en");
    expect(detectLanguage("zh;q=0")).toBe("en");
  });

  it("matches the language subtag, not a substring", () => {
    expect(detectLanguage("en-GB")).toBe("en");
    expect(detectLanguage("zh-Hant-TW")).toBe("zh");
  });

  it("honours the override and handles empty input", () => {
    expect(detectLanguage("zh-CN", "en")).toBe("en");
    expect(detectLanguage("en-US", "zh")).toBe("zh");
    expect(detectLanguage(null)).toBe("en");
    expect(detectLanguage("")).toBe("en");
  });

  it("keeps UI chrome messages in the catalog", () => {
    expect(messages.zh.tabs.projects).toBe("项目");
    expect(messages.zh.index.newIndex).toBe("新建索引");
    expect(messages.en.index.repositoryPath).toBe("Repository path");
    expect(messages.en.projects.enter).toBe("Enter");
    expect(messages.en.projects.lastIndexed).toBe("Last indexed");
    expect(messages.zh.projects.enter).toBe("进入");
    expect(messages.zh.projects.lastIndexed).toBe("最近索引");
    expect(messages.en.control.panel).toBe("Control Panel");
    expect(messages.en.projects.noIndexedProjects).toBe("No indexed projects");
    expect(messages.en.projects.indexFirstRepository).toBe("Index your first repository");
    expect(messages.en.graph.backToDashboard).toBe("Back to Dashboard");
    expect(messages.zh.graph.backToDashboard).toBe("返回仪表盘");
    expect(messages.en.tabs.adr).toBe("ADR");
    expect(messages.zh.tabs.adr).toBe("架构决策");
    expect(messages.en.adr.saveSuccess).toBe("ADR saved");
    expect(messages.zh.adr.saveSuccess).toBe("架构决策已保存");
    expect(messages.en.adr.saveError).toBe("Failed to save ADR");
    expect(messages.zh.adr.saveError).toBe("保存架构决策失败");
    expect(messages.en.adr.unsavedConfirm).toBe("You have unsaved ADR changes. Leave without saving?");
    expect(messages.zh.adr.unsavedConfirm).toBe("架构决策有未保存的更改。确定不保存并离开？");
    expect(messages.en.projects.conflict).toBe("Path conflict");
    expect(messages.zh.projects.conflict).toBe("路径冲突");
    expect(messages.en.projects.deleteNamed("a1")).toBe("Delete a1");
    expect(messages.zh.projects.deleteNamed("a1")).toBe("删除 a1");
  });
});
