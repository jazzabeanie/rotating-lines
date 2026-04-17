## Why

When time markers (digit labels) are hidden, the outer screen area they occupied becomes wasted whitespace. Two new behaviors can reclaim that space: automatically extending second-tick lines to fill it, and optionally extending them all the way to a dramatic "bleed off the screen" effect on square devices.

## What Changes

- When `SHOW_MARKERS` is off, second-tick lines (outer ring) automatically lengthen to reach the radius formerly occupied by the time marker digits.
- A new settings toggle `EXTEND_SECOND_LINES` appears **only when** `SHOW_MARKERS` is off; when enabled, second-tick lines extend further so their pivot sits just inside the screen's short-dimension edge and their inner endpoint aligns with the outer edge of the minute lines.
- The `EXTEND_SECOND_LINES` setting is persisted and synced via the JS settings page, mirroring the existing `SHOW_MARKERS` pattern.

## Capabilities

### New Capabilities

- `second-line-fill`: When markers hidden, second-tick lines extend outward to fill the space previously occupied by time marker digits.
- `second-line-overextend`: Optional secondary toggle (visible only when markers are off) that extends second-tick lines past the screen edge; pivot centered at `radius` (short-dimension edge), inner tip aligned with minute-line outer edge.

### Modified Capabilities

*(none — no existing spec-level behavior changes)*

## Impact

- `src/c/main.c`: `draw_rim_lines()` gains length/pivot parameters driven by the two new modes; `layer_update_proc` passes the appropriate values; settings load/receive handler gains `PERSIST_KEY_EXTEND_SECOND_LINES`.
- `src/pkjs/index.js`: settings page gains conditional `EXTEND_SECOND_LINES` checkbox (shown/hidden based on `show_markers` state); save/load/send logic updated.
- `package.json` / `appinfo.json`: new `MESSAGE_KEY_EXTEND_SECOND_LINES` entry.
