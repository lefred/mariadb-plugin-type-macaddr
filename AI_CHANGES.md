# AI-Assisted Changes

This file logs code changes made with AI assistance, for transparency.

## 2026-08-05 — Claude Sonnet 5

Fixed two issues found during a code review (coding-standards and security
pass) of `macaddr_functions.cc` and `sql_type_macaddr.cc`.

### 1. Inconsistent handling of raw-binary arguments in `MACADDR*` functions

`value_from_item()` in `macaddr_functions.cc` reimplemented a subset of
`Type_handler_fbt::Fbt::make_from_item()` by hand: a fast path for
already-typed `MACADDR`/`MACADDR8` arguments, and a fallback that called
`val_str()` and always parsed the result as hex text. That fallback never
checked for `charset() == &my_charset_bin`, so a raw binary argument of the
correct byte length (e.g. `MACADDR_TRUNC(x'08002b010203')`) was hex-parsed
instead of copied as-is, unlike `CAST(x'...' AS MACADDR)`, which special-cases
that path — producing an unnecessary "Incorrect macaddr value" warning and
`NULL` where a value was expected.

**Fix:** `value_from_item()` now constructs `Handler::Fbt_null(item)`
directly, the same conversion entry point the type-cast implementation
uses, so `MACADDR_TRUNC()`, `MACADDR8_TRUNC()`, and `MACADDR8_SET7BIT()`
handle native, text, and raw-binary arguments identically to `CAST(... AS
MACADDR[8])`.

### 2. `ascii_to_fbt()` accepted separator placements PostgreSQL doesn't

The parser in `sql_type_macaddr.cc` allowed a `:`/`-`/`.` separator at any
even hex-digit boundary, not just the specific group widths PostgreSQL's
`macaddr`/`macaddr8` input syntax documents (byte-pairs, a 3-byte-OUI +
remainder split, or 4-digit dotted groups). That meant malformed-looking
input such as `'aa:bb:ccddee:ff'` or `'08:002b:01:0203'` was silently
accepted as a valid address.

**Fix:** the parser now records each separator-delimited group's digit
count while scanning, then validates the full set of groups against the
three documented forms (`08:00:2b:01:02:03`, `08002b:010203` /
`08002b-0102030405`, `0800.2b01.0203`) plus the no-separator form
(`08002b010203`). Anything else is now rejected. Verified against a
standalone harness covering every existing `mysql-test/type_macaddr`
input plus the two examples above — all previously-passing cases still
parse identically, and the over-permissive cases are now correctly
rejected.

### 3. Added `mysql-test` coverage for both fixes

Added to `mysql-test/type_macaddr/type_macaddr.test`:

- `CAST('aa:bb:ccddee:ff' AS MACADDR) IS NULL` and
  `CAST('08:002b:01:0203' AS MACADDR) IS NULL` — addresses with a
  separator at a digit boundary PostgreSQL doesn't document; must now be
  rejected (fix 2).
- `MACADDR_TRUNC(x'08002b010203')`,
  `MACADDR8_TRUNC(x'08002bfffe010203')`,
  `MACADDR8_SET7BIT(x'003456778899aabb')` — raw binary arguments of the
  correct byte length, exercising the fixed `value_from_item()` path
  (fix 1).

