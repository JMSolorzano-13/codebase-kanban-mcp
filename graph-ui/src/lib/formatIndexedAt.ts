/**
 * @sdd-task: Task #3 - formatIndexedAt
 * @sdd-spec: specs/spec-001-w3q-executive-dashboard/spec.md
 * @sdd-decision: SDD-ADR-003 - indexed_at shown as UTC locale via time[dateTime]
 * @sdd-why: US-002 freshness — visible datetime derived from ISO, not raw ISO only
 * @human-debug: If output equals raw ISO → Date parse failed (invalid input) or locale options dropped
 */

export type IndexedAtLang = "en" | "zh";

const LOCALE_BY_LANG: Record<IndexedAtLang, string> = {
  en: "en-US",
  zh: "zh-CN",
};

const UTC_PARTS: Intl.DateTimeFormatOptions = {
  year: "numeric",
  month: "numeric",
  day: "numeric",
  hour: "numeric",
  minute: "numeric",
  timeZone: "UTC",
  timeZoneName: "short",
};

export function formatIndexedAt(iso: string, lang: IndexedAtLang): string {
  const date = new Date(iso);
  if (Number.isNaN(date.getTime())) {
    return iso;
  }
  return new Intl.DateTimeFormat(LOCALE_BY_LANG[lang], UTC_PARTS).format(date);
}
