# Open Questions and Coverage Review

Holding area for findings that are **not yet verified enough** for a topic page. Nothing here is authoritative; do not act on an entry without checking the code. Promote verified entries into the relevant page and delete them here; delete entries that turn out to be wrong.

## Open findings

Add one bullet per finding: `- <area/file>: <observation> — <what would verify it> (<YYYY-MM-DD>)`.

- None recorded.

## Coverage review

Pages that are small relative to the area they cover. They are review candidates, not known to be incomplete. When a session works in one of these areas, compare the page against the code and extend it with verified guidance; remove the row once reviewed.

| Page | Size | Area to compare against |
|---|---|---|
| [Regular expressions](domain/regular-expressions.md) | ~1 KB | PCRE2 integration and search/replace paths |
| [Localization](domain/localization.md) | ~1.4 KB | Locale resources under `language\` and string-adding workflow |
| [Configuration and INI](domain/configuration-and-ini.md) | ~1.9 KB | `Config.cpp` settings load/save and `Settings` / `Settings2` structs |
| [Lexer development](domain/lexer-development.md) | ~2.9 KB | Lexer registration, styles, and themes |
