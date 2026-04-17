## ADDED Requirements

### Requirement: Extend-lines setting available when markers hidden
A new persistent setting `EXTEND_SECOND_LINES` (boolean, default false) SHALL exist. It SHALL only be configurable when `SHOW_MARKERS` is off. The setting SHALL be persisted via `PERSIST_KEY_EXTEND_SECOND_LINES = 8` and synced from the JS settings page via `MESSAGE_KEY_EXTEND_SECOND_LINES`.

#### Scenario: Setting hidden when markers on
- **WHEN** `SHOW_MARKERS` is true on the settings page
- **THEN** the "Extend lines" control SHALL NOT be visible

#### Scenario: Setting visible when markers off
- **WHEN** `SHOW_MARKERS` is false on the settings page
- **THEN** the "Extend lines" checkbox SHALL be visible and interactive

#### Scenario: Setting persisted across reloads
- **WHEN** user enables `EXTEND_SECOND_LINES` and saves
- **THEN** the setting is retained on the next settings page open

### Requirement: All rings overextend past screen when setting enabled
When both `SHOW_MARKERS` is false and `EXTEND_SECOND_LINES` is true, ALL three rings (hours, minutes, seconds) SHALL use an effective radius `r_eff = radius * 8 / 7`. Internal geometry ratios are preserved, so rings continue to tile without overlap. The outer tip of the seconds ring extends past the short-dimension screen edge; hours and minutes also grow proportionally.

This `r_eff` is derived from the condition that the seconds ring's pivot (`r_eff * 7/8`) lands exactly at `radius` (the screen edge), matching the original single-ring overextend intent while scaling all rings uniformly.

Geometry (where `r = min(w,h)/2 - 14`, `radius = min(w,h)/2`):
- `r_eff = radius * 8 / 7` (integer arithmetic)

#### Scenario: All rings extend past screen edge
- **WHEN** `SHOW_MARKERS` is false and `EXTEND_SECOND_LINES` is true and seconds are visible
- **THEN** all three rings are scaled to `r_eff = radius * 8 / 7`; the seconds ring's outer tip extends past the screen edge and hours/minutes are proportionally longer

#### Scenario: Overextend disabled falls back to fill mode
- **WHEN** `SHOW_MARKERS` is false and `EXTEND_SECOND_LINES` is false
- **THEN** all rings use fill-mode geometry (`r_eff = radius`)

#### Scenario: Markers re-enabled clears overextend visual
- **WHEN** `SHOW_MARKERS` is toggled back to true
- **THEN** all rings revert to normal geometry (`r_eff = r`) regardless of `EXTEND_SECOND_LINES` value

#### Scenario: Seconds hidden while overextend enabled
- **WHEN** `SHOW_MARKERS` is false and `EXTEND_SECOND_LINES` is true but seconds are suppressed
- **THEN** hours and minutes still use `r_eff = radius * 8 / 7` (overextend takes precedence over fill)
