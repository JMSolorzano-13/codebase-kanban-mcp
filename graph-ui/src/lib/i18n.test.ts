/**
 * @sdd-task: Task #3 - SpecBoardTab strip + EpicCard wrap + i18n
 * @sdd-spec: specs/spec-015-s5k-specs-debt-and-path/spec.md
 * @sdd-decision: SDD-ADR-067 - Specs-only debt strip; aria-label; dead text
 * @sdd-why: lock specBoard.openTechDebt en "Open tech debt"; zh filled
 * @human-debug: If region name fails → openTechDebt drifted from "Open tech debt"
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
    expect(messages.en.adr.generatedAt).toBe("Generated at");
    expect(messages.zh.adr.generatedAt).toBe("生成于");
    expect(messages.en.adr.replaceWarning).toBe(
      "Edits inside the generated region are replaced on the next user-triggered index.",
    );
    expect(messages.zh.adr.replaceWarning).toBe("生成区域内的编辑会在下一次用户触发的索引时被替换。");
    expect(messages.en.projects.conflict).toBe("Path conflict");
    expect(messages.zh.projects.conflict).toBe("路径冲突");
    expect(messages.en.projects.deleteNamed("a1")).toBe("Delete a1");
    expect(messages.zh.projects.deleteNamed("a1")).toBe("删除 a1");
    expect(messages.en.index.pathExistsNotice("alpha")).toBe("Already indexed as alpha");
    expect(messages.zh.index.pathExistsNotice("alpha")).toBe("已索引为 alpha");
    expect(messages.en.projects.reindex).toBe("Reindex");
    expect(messages.zh.projects.reindex).toBe("重新索引");
    expect(messages.en.projects.reindexError).toBe("Reindex failed");
    expect(messages.zh.projects.reindexError).toBe("重新索引失败");
    expect(messages.en.index.nameExists("foo")).toBe("Name already used by foo");
    expect(messages.zh.index.nameExists("foo")).toBe("名称已被 foo 占用");
    expect(messages.en.specBoard.archive).toBe("Archive");
    expect(messages.zh.specBoard.archive).toBe("归档");
    expect(messages.en.specBoard.unarchive).toBe("Unarchive");
    expect(messages.zh.specBoard.unarchive).toBe("取消归档");
    expect(messages.en.specBoard.showArchived).toBe("Show archived");
    expect(messages.zh.specBoard.showArchived).toBe("显示已归档");
    expect(messages.en.tabs.game).toBe("Game");
    expect(messages.zh.tabs.game).toBe("游戏");
    expect(messages.en.gameBoard.stateMdMissing).toBe("state.md missing");
    expect(messages.zh.gameBoard.stateMdMissing).toBe("缺少 state.md");
    expect(messages.en.gameBoard.phasePreproduction).toBe("Pre-production");
    expect(messages.en.gameBoard.phaseProduction).toBe("Production");
    expect(messages.en.gameBoard.phasePostproduction).toBe("Post-production & Launch");
    expect(messages.zh.gameBoard.phasePreproduction).toBe("前期制作");
    expect(messages.zh.gameBoard.phaseProduction).toBe("制作");
    expect(messages.zh.gameBoard.phasePostproduction).toBe("后期制作与发行");
    expect(messages.en.gameBoard.columnInbox).toBe("Inbox");
    expect(messages.zh.gameBoard.columnInbox).toBe("收件箱");
    expect(messages.en.gameBoard.workStatePending).toBe("Pending");
    expect(messages.en.gameBoard.workStateInProgress).toBe("In progress");
    expect(messages.en.gameBoard.workStateDone).toBe("Done");
    expect(messages.en.gameBoard.workStateBlocked).toBe("Blocked");
    expect(messages.zh.gameBoard.workStatePending).toBe("待处理");
    expect(messages.zh.gameBoard.workStateInProgress).toBe("进行中");
    expect(messages.zh.gameBoard.workStateDone).toBe("已完成");
    expect(messages.zh.gameBoard.workStateBlocked).toBe("已阻塞");
    expect(messages.en.gameBoard.inputs).toBe("Inputs");
    expect(messages.zh.gameBoard.inputs).toBe("输入");
    expect(messages.en.gameBoard.blockedStrip).toBe("Blocked");
    expect(messages.zh.gameBoard.blockedStrip).toBe("已阻塞");
    expect(messages.en.gameBoard.showDones).toBe("Show Dones");
    expect(messages.en.gameBoard.trackA).toBe("Track A");
    expect(messages.en.gameBoard.trackB).toBe("Track B");
    expect(messages.en.gameBoard.trackAll).toBe("All");
    expect(messages.zh.gameBoard.showDones).toBe("显示已完成");
    expect(messages.zh.gameBoard.trackA).toBe("轨道 A");
    expect(messages.zh.gameBoard.trackB).toBe("轨道 B");
    expect(messages.zh.gameBoard.trackAll).toBe("全部");
    expect(messages.en.specBoard.showArchived).toBe("Show archived");
    expect(messages.zh.specBoard.showArchived).toBe("显示已归档");
    expect("showArchived" in messages.en.gameBoard).toBe(false);
    expect("showArchived" in messages.zh.gameBoard).toBe(false);
    expect(messages.en.specBoard.noTasksYet).toBe("No tasks planned yet");
    expect(messages.en.specBoard.openTechDebt).toBe("Open tech debt");
    expect(messages.zh.specBoard.openTechDebt).toBe("未解决的技术债");
  });
});
