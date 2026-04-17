## ADDED Requirements

### Requirement: All rings expand when markers hidden
When `SHOW_MARKERS` is off and seconds are visible, all three rings (hours, minutes, seconds) SHALL expand uniformly by using an effective radius `r_eff = radius` instead of the normal `r = radius - 14`. Each ring's internal geometry ratios are preserved; only the overall scale increases. This fills the space previously occupied by time marker digits without any ring growing disproportionately relative to the others.

#### Scenario: Markers off — all rings visually longer
- **WHEN** `SHOW_MARKERS` is false and seconds are displayed
- **THEN** all three rings (hours, minutes, seconds) are proportionally longer, with the outer edge of the seconds ring reaching the short-dimension screen edge

#### Scenario: Markers on — all rings normal length
- **WHEN** `SHOW_MARKERS` is true
- **THEN** all rings use normal geometry (`r_eff = r`), leaving radial margin for digit labels

#### Scenario: Seconds hidden — fill mode not relevant for seconds ring
- **WHEN** seconds are suppressed by battery threshold
- **THEN** the seconds ring is not drawn; hours and minutes expansion follows the seconds-hidden rule

### Requirement: Hours and minutes expand when seconds hidden
When seconds are suppressed by the battery threshold, hours and minutes SHALL expand to fill the freed space by using `r_eff = radius`. The minutes ring SHALL use its extended geometry (spanning `[r_eff/2, r_eff]`) and the hours ring SHALL span `[0, r_eff/2]`, so together they cover the full display radius.

#### Scenario: Seconds hidden — hours and minutes fill freed space
- **WHEN** battery is below the seconds threshold (seconds not drawn)
- **THEN** hours and minutes rings are both longer than when seconds are visible, filling from center to screen edge

#### Scenario: Seconds hidden with markers on — lines longer, markers unchanged
- **WHEN** seconds are hidden AND `SHOW_MARKERS` is true
- **THEN** hours and minutes expand to `r_eff = radius` but time marker digits remain at their normal radial position (`r + 4`)

#### Scenario: Seconds visible — no expansion from this rule
- **WHEN** seconds are being drawn
- **THEN** this rule does not apply; expansion (if any) is governed only by the markers-hidden rule
