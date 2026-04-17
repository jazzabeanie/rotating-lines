## 1. C Firmware — Settings

- [x] 1.1 Add `#define PERSIST_KEY_EXTEND_SECOND_LINES 8` and `static bool s_extend_second_lines = false;` in `main.c`
- [x] 1.2 Load `PERSIST_KEY_EXTEND_SECOND_LINES` in `load_settings()`
- [x] 1.3 Handle `MESSAGE_KEY_EXTEND_SECOND_LINES` tuple in the `inbox_received_callback`

## 2. C Firmware — Drawing

- [x] 2.1 Revert `draw_rim_lines` back to hardcoded ratios (`r * 7/8` pivot, `r / 4` length) — remove `pivot_dist`/`length` parameters added in previous iteration
- [x] 2.2 In `canvas_update_proc`, compute `r_eff` before drawing:
  - Extend on (markers off + extend enabled): `r_eff = radius * 8 / 7`
  - Fill (markers off OR seconds hidden): `r_eff = radius`
  - Normal: `r_eff = r`
- [x] 2.3 Pass `r_eff` as the `r` argument to `draw_inner_lines`, `draw_middle_lines`, and `draw_rim_lines`; pass original `r` to `draw_time_markers`

## 3. JS Settings Page

- [x] 3.1 Add `EXTEND_SECOND_LINES` message key to `appinfo.json` (or `package.json` as applicable)
- [x] 3.2 Add "Extend lines past screen edge" checkbox row to the settings HTML in `index.js`, with `id="extendlines"`, initially hidden via `style="display:none"`
- [x] 3.3 Add JS logic to show/hide the extend-lines row based on the `showmarkers` checkbox state (toggle on `change` event; also apply on page load)
- [x] 3.4 Persist `extend_second_lines` in `localStorage` save/load alongside existing settings
- [x] 3.5 Send `EXTEND_SECOND_LINES` in the message to the watch only when `show_markers` is false (gate in the send handler in `index.js`)
