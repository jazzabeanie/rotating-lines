## Context

The watchface draws three concentric rings of rotating lines (inner = hours, middle = minutes, outer = seconds). Time marker digits are rendered just beyond the outer edge of the seconds ring at `r + 4` (where `r = min(w,h)/2 - 14`). When markers are hidden, the zone from `r` to the screen edge is unused. When seconds are suppressed by battery threshold, the outer ring zone is similarly freed.

Current ring geometry (all proportions relative to `r`):
- Hours: pivot `r/4`, length `r/2` → spans `[0, r/2]`
- Minutes: pivot `r*5/8`, length `r/4` → spans `[r/2, 3r/4]` (or `[r/2, r]` when seconds hidden)
- Seconds: pivot `r*7/8`, length `r/4` → spans `[3r/4, r]`

Minute ring outer tip: `3r/4`. The three rings tile perfectly from `0` to `r`.

## Goals / Non-Goals

**Goals:**
- When markers hidden OR seconds hidden: all rings expand uniformly to `r_eff = radius`, filling freed space
- Optional toggle `EXTEND_SECOND_LINES` (available when markers off): scales all rings to `r_eff = radius * 8/7`, putting the seconds ring's pivot at the screen edge and extending all rings proportionally past it
- Preserved ring-to-ring ratios in all modes (no overlap)
- New setting persisted and synced like existing settings
- Conditional UI: `EXTEND_SECOND_LINES` control only visible when `SHOW_MARKERS` is off

**Non-Goals:**
- Animating the length transition when modes change
- Changing the radial position of time marker digits
- Changing ring geometry when both markers are on AND seconds are visible

## Decisions

### D1: Single `r_eff` value passed to all draw functions

All three `draw_*_lines` functions accept `r` as their geometry reference. Instead of adding mode parameters inside each function, the caller computes `r_eff` once and passes it as the `r` argument to all three.

**Why**: The ring geometry ratios are hardcoded relative to `r` inside each function. Passing `r_eff` as `r` requires zero changes inside the drawing functions — the scaling falls out automatically. Adding per-function mode flags would embed policy inside drawing functions and require more code paths.

**Alternative**: Parameterize each function with explicit pivot/length — rejected because it duplicates the ratio arithmetic and breaks when adding new rings.

### D2: `r_eff` computation rules (evaluated in priority order)

1. `EXTEND_SECOND_LINES` on (implies markers off): `r_eff = radius * 8 / 7`
2. Markers off OR seconds hidden: `r_eff = radius`
3. Otherwise: `r_eff = r`

`radius * 8 / 7` is derived from: seconds pivot = `r_eff * 7/8 = radius` → `r_eff = radius * 8/7`. This preserves the original single-ring intent of "pivot at screen edge" while scaling all rings.

**Why combine markers-off and seconds-hidden into the same fill level**: Both conditions free the same radial margin. Treating them identically keeps the computation simple and the visual consistent.

### D3: Markers always drawn at original `r`, not `r_eff`

`draw_time_markers` continues to receive the original `r` value (not `r_eff`). Markers are only drawn when `SHOW_MARKERS` is true, which is also when `r_eff = r`, so this is always consistent.

**Why**: When markers are on, `r_eff = r` by definition (rule 3 or seconds-visible rule 3). Passing original `r` is redundant but makes the contract explicit.

### D4: Revert `draw_rim_lines` parameterization

The initial implementation parameterized `draw_rim_lines` with explicit `pivot_dist` and `length`. With the uniform `r_eff` approach, this is unnecessary — the function should be reverted to its original hardcoded ratios and simply receive `r_eff` as `r`.

**Why**: Fewer moving parts. The uniform scaling design makes per-function parameterization a complexity with no benefit.

## Risks / Trade-offs

- **`radius * 8/7` integer truncation** → At `radius = 72` (square Pebble): `r_eff = 82`, outer tip at 82px (10px past edge). At `radius = 90` (round): `r_eff = 102` (12px past edge). Sub-pixel at these sizes; acceptable.
- **Seconds-hidden + markers-on: minute lines reach screen edge** → Extended minute outer tip = `r_eff = radius`. Marker digits sit at `r + 4 = radius - 10`, so lines are drawn 10px behind/under markers. Markers are drawn last so they appear on top. Visual is fine.
- **New persist key must not conflict** → Use `PERSIST_KEY_EXTEND_SECOND_LINES = 8` (next available).

## Migration Plan

No migration needed. New persist key defaults to `false` (0) on first load. JS settings page degrades gracefully: old saved settings without `extend_second_lines` leave the default.
