/**
 * @sdd-task: Task #1 - Drop UTC pin in formatIndexedAt + helper oracles
 * @sdd-spec: specs/spec-007-n6p-last-indexed-local/spec.md
 * @sdd-decision: SDD-ADR-034 - indexed_at visible text uses runtime TZ; dateTime/title stay ISO
 * @sdd-why: US-001 / US-006 helper — drop timeZone pin so every caller shows host wall clock
 * @human-debug: If output equals raw ISO → Date parse failed or options dropped; if still UTC while host is not → timeZone leaked back into INDEXED_AT_PARTS
 */

export type IndexedAtLang = "en" | "zh";

const LOCALE_BY_LANG: Record<IndexedAtLang, string> = {
  en: "en-US",
  zh: "zh-CN",
};

const INDEXED_AT_PARTS: Intl.DateTimeFormatOptions = {
  year: "numeric",
  month: "numeric",
  day: "numeric",
  hour: "numeric",
  minute: "numeric",
  timeZoneName: "short",
};

export function formatIndexedAt(iso: string, lang: IndexedAtLang): string {
  const date = new Date(iso);
  if (Number.isNaN(date.getTime())) {
    return iso;
  }
  return new Intl.DateTimeFormat(LOCALE_BY_LANG[lang], INDEXED_AT_PARTS).format(date);
}
