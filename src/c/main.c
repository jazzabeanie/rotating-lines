#include <pebble.h>

#define PERSIST_KEY_BG_COLOR 1
#define PERSIST_KEY_LINE_COLOR 2
#define PERSIST_KEY_SMOOTH_THRESHOLD 3
#define PERSIST_KEY_SECONDS_THRESHOLD 4
#define PERSIST_KEY_BATTERY_SHOW_THRESHOLD 5
#define PERSIST_KEY_LINE_ROTATION 6
#define PERSIST_KEY_SHOW_MARKERS 7
#define PERSIST_KEY_EXTEND_SECOND_LINES 8
#define PERSIST_KEY_SHOW_DATE 9

#define SMOOTH_ROTATION_INTERVAL_MS 33

static Window *s_window;
static Layer *s_canvas_layer;
// static TextLayer *s_time_layer;
static GColor s_bg_color;
static GColor s_line_color;
static int32_t s_inner_angle;
static int32_t s_middle_angle;
static int32_t s_outer_angle;
// static char s_time_buf[12];

static AppTimer *s_smooth_timer;
static int s_smooth_threshold = 80;
static int s_seconds_threshold = 60;
static int s_battery_show_threshold = 50;
static bool s_line_rotation = true;
static bool s_show_markers = true;
static bool s_extend_second_lines = false;
static bool s_show_date = true;

static AppTimer *s_startup_timer;
static time_t s_startup_time;
static uint16_t s_startup_ms;
static bool s_startup_started = false;

static bool smooth_enabled(void);

static int32_t startup_progress(void) {
  if (!s_startup_started) return 0;
  time_t now;
  uint16_t ms;
  time_ms(&now, &ms);
  int32_t elapsed = (int32_t)(now - s_startup_time) * 1000 + (int32_t)ms - (int32_t)s_startup_ms;
  if (elapsed >= 1000) return 1000;
  if (elapsed < 0) return 0;
  return elapsed;
}

static int32_t phase_stroke_width(int32_t d, int32_t one_unit) {
  int32_t t = one_unit / 4;
  if (d < one_unit) {
    if (d < t)           return 4;
    else if (d < 2 * t)  return 3;
    else if (d < 3 * t)  return 2;
    else                 return 1;
  } else if (d >= TRIG_MAX_ANGLE - one_unit) {
    int32_t d_out = TRIG_MAX_ANGLE - d;
    if (d_out < t)           return 4;
    else if (d_out < 2 * t)  return 3;
    else if (d_out < 3 * t)  return 2;
    else                     return 1;
  }
  return 1;
}

static GPoint mark_pivot(GPoint center, int32_t dist, int i, int count) {
  int32_t mark_angle = i * TRIG_MAX_ANGLE / count;
  return (GPoint){
    .x = center.x + sin_lookup(mark_angle) * dist / TRIG_MAX_RATIO,
    .y = center.y - cos_lookup(mark_angle) * dist / TRIG_MAX_RATIO,
  };
}

static void draw_rotating_line(GContext *ctx, GPoint pivot, int32_t length, int32_t angle) {
  int32_t dx = sin_lookup(angle) * length / TRIG_MAX_RATIO;
  int32_t dy = cos_lookup(angle) * length / TRIG_MAX_RATIO;
  GPoint p1 = { .x = pivot.x - dx / 2, .y = pivot.y + dy / 2 };
  GPoint p2 = { .x = pivot.x + dx / 2, .y = pivot.y - dy / 2 };
  graphics_draw_line(ctx, p1, p2);
}

static void draw_inner_lines(GContext *ctx, GPoint center, int32_t r, int32_t inner_angle, int highlight) {
  int32_t one_hour = TRIG_MAX_ANGLE / 12;
  int32_t prog = startup_progress();
  int32_t eased = prog;
  if (prog < 1000) {
    int64_t inv = 1000 - prog;
    eased = (int32_t)(1000 - inv * inv * inv / 1000000);
  }
  for (int i = 0; i < 12; i++) {
    int32_t mark_angle = i * TRIG_MAX_ANGLE / 12;
    int32_t line_angle;
    if (s_line_rotation) {
      int32_t d = ((mark_angle - inner_angle * 2) % TRIG_MAX_ANGLE + TRIG_MAX_ANGLE) % TRIG_MAX_ANGLE;
      int32_t rot;
      if (d < one_hour) {
        rot = (TRIG_MAX_ANGLE / 4) * d / one_hour;
      } else {
        rot = (TRIG_MAX_ANGLE / 4) * (TRIG_MAX_ANGLE - d) / (TRIG_MAX_ANGLE * 11 / 12);
      }
      line_angle = mark_angle + rot;
    } else {
      line_angle = inner_angle + i * TRIG_MAX_ANGLE / 24;
    }
    if (eased < 1000) {
      int32_t diff = line_angle - mark_angle;
      while (diff > TRIG_MAX_ANGLE / 2) diff -= TRIG_MAX_ANGLE;
      while (diff < -TRIG_MAX_ANGLE / 2) diff += TRIG_MAX_ANGLE;
      line_angle = mark_angle + diff * eased / 1000;
    }
    graphics_context_set_stroke_width(ctx, i == highlight ? 2 : 1);
    draw_rotating_line(ctx, mark_pivot(center, r / 4, i, 12), r / 2, line_angle);
  }
}

static void draw_middle_lines(GContext *ctx, GPoint center, int32_t r, int32_t middle_angle, int highlight, bool extended) {
  int32_t pivot_dist = extended ? r * 3 / 4 : r * 5 / 8;
  int32_t length = extended ? r / 2 : r / 4;
  int32_t one_min = TRIG_MAX_ANGLE / 60;
  int32_t prog = startup_progress();
  int32_t eased = prog;
  if (prog < 1000) {
    int64_t inv = 1000 - prog;
    eased = (int32_t)(1000 - inv * inv * inv / 1000000);
  }
  for (int i = 0; i < 60; i++) {
    int32_t mark_angle = i * TRIG_MAX_ANGLE / 60;
    int32_t line_angle;
    if (s_line_rotation) {
      int32_t d = ((mark_angle - middle_angle * 2) % TRIG_MAX_ANGLE + TRIG_MAX_ANGLE) % TRIG_MAX_ANGLE;
      int32_t rot;
      if (d < one_min) {
        rot = (TRIG_MAX_ANGLE / 4) * d / one_min;
      } else {
        rot = (TRIG_MAX_ANGLE / 4) * (TRIG_MAX_ANGLE - d) / (TRIG_MAX_ANGLE * 59 / 60);
      }
      line_angle = mark_angle + rot;
    } else {
      line_angle = middle_angle + i * TRIG_MAX_ANGLE / 120;
    }
    if (eased < 1000) {
      int32_t diff = line_angle - mark_angle;
      while (diff > TRIG_MAX_ANGLE / 2) diff -= TRIG_MAX_ANGLE;
      while (diff < -TRIG_MAX_ANGLE / 2) diff += TRIG_MAX_ANGLE;
      line_angle = mark_angle + diff * eased / 1000;
    }
    graphics_context_set_stroke_width(ctx, i == highlight ? 2 : 1);
    draw_rotating_line(ctx, mark_pivot(center, pivot_dist, i, 60), length, line_angle);
  }
}

static void draw_rim_lines(GContext *ctx, GPoint center, int32_t r, int32_t outer_angle, int highlight) {
  int32_t one_sec = TRIG_MAX_ANGLE / 60;
  int32_t prog = startup_progress();
  int32_t eased = prog;
  if (prog < 1000) {
    int64_t inv = 1000 - prog;
    eased = (int32_t)(1000 - inv * inv * inv / 1000000);
  }
  for (int i = 0; i < 60; i++) {
    int32_t mark_angle = i * TRIG_MAX_ANGLE / 60;
    int32_t line_angle;
    if (s_line_rotation) {
      int32_t d = ((mark_angle - outer_angle * 2) % TRIG_MAX_ANGLE + TRIG_MAX_ANGLE) % TRIG_MAX_ANGLE;
      int32_t rot;
      if (d < one_sec) {
        rot = (TRIG_MAX_ANGLE / 4) * d / one_sec;
      } else {
        rot = (TRIG_MAX_ANGLE / 4) * (TRIG_MAX_ANGLE - d) / (TRIG_MAX_ANGLE * 59 / 60);
      }
      line_angle = mark_angle + rot;
      // TODO: step timing needs tuning to make the gradual build/fade more perceptible
      // graphics_context_set_stroke_width(ctx, smooth_enabled() ? phase_stroke_width(d, one_sec) : (i == highlight ? 2 : 1));
      graphics_context_set_stroke_width(ctx, 1);
    } else {
      line_angle = outer_angle + i * TRIG_MAX_ANGLE / 120;
      graphics_context_set_stroke_width(ctx, i == highlight ? 2 : 1);
    }
    if (eased < 1000) {
      int32_t diff = line_angle - mark_angle;
      while (diff > TRIG_MAX_ANGLE / 2) diff -= TRIG_MAX_ANGLE;
      while (diff < -TRIG_MAX_ANGLE / 2) diff += TRIG_MAX_ANGLE;
      line_angle = mark_angle + diff * eased / 1000;
    }
    draw_rotating_line(ctx, mark_pivot(center, r * 7 / 8, i, 60), r / 4, line_angle);
  }
}

static void draw_marker_text(GContext *ctx, GPoint center, int32_t dist, int32_t mark_angle, const char *text, const char *font_key) {
  GPoint pos = {
    .x = center.x + sin_lookup(mark_angle) * dist / TRIG_MAX_RATIO,
    .y = center.y - cos_lookup(mark_angle) * dist / TRIG_MAX_RATIO,
  };
  GRect rect = GRect(pos.x - 14, pos.y - 12, 28, 20);
  graphics_draw_text(ctx, text, fonts_get_system_font(font_key),
                     rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static int wrap_diff(int a, int b, int modulus) {
  int d = a - b;
  if (d < 0) d = -d;
  if (d > modulus / 2) d = modulus - d;
  return d;
}

static void draw_time_markers(GContext *ctx, GPoint center, int32_t r_circle, bool show_seconds) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int hour = t->tm_hour % 12;
  if (hour == 0) hour = 12;
  int minute = t->tm_min;
  int second = t->tm_sec;

  int32_t dist = r_circle + 4;
  char buf[4];

  graphics_context_set_text_color(ctx, s_line_color);

  int hour_pos = (hour % 12) * 5;
  int32_t h_angle = hour_pos * TRIG_MAX_ANGLE / 60;
  snprintf(buf, sizeof(buf), "%d", hour);
  draw_marker_text(ctx, center, dist, h_angle, buf, FONT_KEY_GOTHIC_18_BOLD);

  bool show_minute = wrap_diff(minute, hour_pos, 60) >= 3;
  if (show_minute) {
    int32_t m_angle = minute * TRIG_MAX_ANGLE / 60;
    snprintf(buf, sizeof(buf), "%02d", minute);
    draw_marker_text(ctx, center, dist, m_angle, buf, FONT_KEY_GOTHIC_14_BOLD);
  }

  if (show_seconds) {
    bool clear_of_hour = wrap_diff(second, hour_pos, 60) >= 3;
    bool clear_of_minute = !show_minute || wrap_diff(second, minute, 60) >= 3;
    if (clear_of_hour && clear_of_minute) {
      int32_t s_mark_angle = second * TRIG_MAX_ANGLE / 60;
      snprintf(buf, sizeof(buf), "%02d", second);
      draw_marker_text(ctx, center, dist, s_mark_angle, buf, FONT_KEY_GOTHIC_14);
    }
  }
}

static void draw_battery(GContext *ctx, GRect bounds) {
  BatteryChargeState state = battery_state_service_peek();
  if (state.charge_percent > s_battery_show_threshold) return;

  int x = 4;
  int y = bounds.size.h - 18;

  // Battery body
  graphics_context_set_stroke_color(ctx, s_line_color);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(x, y + 2, 16, 10));
  // Battery nub
  graphics_context_set_fill_color(ctx, s_line_color);
  graphics_fill_rect(ctx, GRect(x + 16, y + 4, 2, 6), 0, GCornerNone);
  // Fill level
  int fill_w = state.charge_percent * 14 / 100;
  if (fill_w > 0) {
    graphics_fill_rect(ctx, GRect(x + 1, y + 3, fill_w, 8), 0, GCornerNone);
  }

  // Percentage text
  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", state.charge_percent);
  graphics_context_set_text_color(ctx, s_line_color);
  GRect text_rect = GRect(x + 22, y - 2, 36, 16);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     text_rect, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void draw_date(GContext *ctx, GRect bounds) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char buf[8];
  strftime(buf, sizeof(buf), "%b %d", t);
  graphics_context_set_text_color(ctx, s_line_color);
  GRect rect = GRect(bounds.size.w - 50, bounds.size.h - 20, 48, 20);
  graphics_draw_text(ctx, buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     rect, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  if (!s_startup_started) {
    s_startup_started = true;
    time_ms(&s_startup_time, &s_startup_ms);
  }

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  uint16_t radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2;
  int32_t r = radius - 14;

  graphics_context_set_fill_color(ctx, s_bg_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, s_line_color);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int hour_idx = t->tm_hour % 12;
  int minute_idx = t->tm_min;
  int second_idx = t->tm_sec;

  BatteryChargeState bat = battery_state_service_peek();
  bool show_seconds = s_seconds_threshold == 0 || bat.charge_percent >= s_seconds_threshold;

  int32_t r_eff;
  if (!s_show_markers && s_extend_second_lines) {
    r_eff = radius * 8 / 7;
  } else if (!s_show_markers || !show_seconds) {
    r_eff = radius;
  } else {
    r_eff = r;
  }

  draw_inner_lines(ctx, center, r_eff, s_inner_angle, hour_idx);
  draw_middle_lines(ctx, center, r_eff, s_middle_angle, minute_idx, !show_seconds);
  if (show_seconds) {
    draw_rim_lines(ctx, center, r_eff, s_outer_angle, second_idx);
  }
  if (s_show_markers) draw_time_markers(ctx, center, r, show_seconds);
  draw_battery(ctx, bounds);
  if (s_show_date) draw_date(ctx, bounds);
}

static void update_angles(void) {
  time_t now;
  uint16_t ms;
  time_ms(&now, &ms);
  struct tm *t = localtime(&now);

  if (!smooth_enabled()) ms = 0;

  int64_t sec_ms = (int64_t)t->tm_sec * 1000 + ms;
  s_outer_angle = (int32_t)(sec_ms * (TRIG_MAX_ANGLE / 2) / 60000);

  int64_t hour_ms = ((int64_t)t->tm_min * 60 + t->tm_sec) * 1000 + ms;
  s_middle_angle = (int32_t)(hour_ms * (TRIG_MAX_ANGLE / 2) / 3600000);

  int64_t day_ms = (((int64_t)t->tm_hour * 60 + t->tm_min) * 60 + t->tm_sec) * 1000 + ms;
  s_inner_angle = (int32_t)(day_ms * TRIG_MAX_ANGLE / 86400000);

  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void smooth_rotation_tick(void *data);

static bool smooth_enabled(void) {
  BatteryChargeState state = battery_state_service_peek();
  return state.charge_percent >= s_smooth_threshold;
}

static void ensure_smooth_timer(void) {
  if (smooth_enabled() && !s_smooth_timer) {
    s_smooth_timer = app_timer_register(SMOOTH_ROTATION_INTERVAL_MS, smooth_rotation_tick, NULL);
  }
}

static void smooth_rotation_tick(void *data) {
  s_smooth_timer = NULL;
  update_angles();
  if (smooth_enabled()) {
    s_smooth_timer = app_timer_register(SMOOTH_ROTATION_INTERVAL_MS, smooth_rotation_tick, NULL);
  }
}

static void startup_tick(void *data) {
  s_startup_timer = NULL;
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
  if (startup_progress() < 1000) {
    s_startup_timer = app_timer_register(SMOOTH_ROTATION_INTERVAL_MS, startup_tick, NULL);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_angles();
  ensure_smooth_timer();
}

static void load_settings(void) {
  s_bg_color = GColorBlack;
  s_line_color = GColorWhite;
  if (persist_exists(PERSIST_KEY_BG_COLOR)) {
    s_bg_color = GColorFromHEX(persist_read_int(PERSIST_KEY_BG_COLOR));
  }
  if (persist_exists(PERSIST_KEY_LINE_COLOR)) {
    s_line_color = GColorFromHEX(persist_read_int(PERSIST_KEY_LINE_COLOR));
  }
  if (persist_exists(PERSIST_KEY_SMOOTH_THRESHOLD)) {
    s_smooth_threshold = persist_read_int(PERSIST_KEY_SMOOTH_THRESHOLD);
  }
  if (persist_exists(PERSIST_KEY_SECONDS_THRESHOLD)) {
    s_seconds_threshold = persist_read_int(PERSIST_KEY_SECONDS_THRESHOLD);
  }
  if (persist_exists(PERSIST_KEY_BATTERY_SHOW_THRESHOLD)) {
    s_battery_show_threshold = persist_read_int(PERSIST_KEY_BATTERY_SHOW_THRESHOLD);
  }
  if (persist_exists(PERSIST_KEY_LINE_ROTATION)) {
    s_line_rotation = persist_read_int(PERSIST_KEY_LINE_ROTATION) != 0;
  }
  if (persist_exists(PERSIST_KEY_SHOW_MARKERS)) {
    s_show_markers = persist_read_int(PERSIST_KEY_SHOW_MARKERS) != 0;
  }
  if (persist_exists(PERSIST_KEY_EXTEND_SECOND_LINES)) {
    s_extend_second_lines = persist_read_int(PERSIST_KEY_EXTEND_SECOND_LINES) != 0;
  }
  if (persist_exists(PERSIST_KEY_SHOW_DATE)) {
    s_show_date = persist_read_int(PERSIST_KEY_SHOW_DATE) != 0;
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *bg = dict_find(iter, MESSAGE_KEY_BG_COLOR);
  Tuple *line = dict_find(iter, MESSAGE_KEY_LINE_COLOR);
  if (bg) {
    int hex = bg->value->int32;
    s_bg_color = GColorFromHEX(hex);
    persist_write_int(PERSIST_KEY_BG_COLOR, hex);
    window_set_background_color(s_window, s_bg_color);
  }
  if (line) {
    int hex = line->value->int32;
    s_line_color = GColorFromHEX(hex);
    persist_write_int(PERSIST_KEY_LINE_COLOR, hex);
  }
  Tuple *thresh = dict_find(iter, MESSAGE_KEY_SMOOTH_THRESHOLD);
  if (thresh) {
    s_smooth_threshold = thresh->value->int32;
    persist_write_int(PERSIST_KEY_SMOOTH_THRESHOLD, s_smooth_threshold);
    ensure_smooth_timer();
  }
  Tuple *sec_thresh = dict_find(iter, MESSAGE_KEY_SECONDS_THRESHOLD);
  if (sec_thresh) {
    s_seconds_threshold = sec_thresh->value->int32;
    persist_write_int(PERSIST_KEY_SECONDS_THRESHOLD, s_seconds_threshold);
  }
  Tuple *bat_thresh = dict_find(iter, MESSAGE_KEY_BATTERY_SHOW_THRESHOLD);
  if (bat_thresh) {
    s_battery_show_threshold = bat_thresh->value->int32;
    persist_write_int(PERSIST_KEY_BATTERY_SHOW_THRESHOLD, s_battery_show_threshold);
  }
  Tuple *rot = dict_find(iter, MESSAGE_KEY_LINE_ROTATION);
  if (rot) {
    s_line_rotation = rot->value->int32 != 0;
    persist_write_int(PERSIST_KEY_LINE_ROTATION, rot->value->int32);
  }
  Tuple *markers = dict_find(iter, MESSAGE_KEY_SHOW_MARKERS);
  if (markers) {
    s_show_markers = markers->value->int32 != 0;
    persist_write_int(PERSIST_KEY_SHOW_MARKERS, markers->value->int32);
  }
  Tuple *extend = dict_find(iter, MESSAGE_KEY_EXTEND_SECOND_LINES);
  if (extend) {
    s_extend_second_lines = extend->value->int32 != 0;
    persist_write_int(PERSIST_KEY_EXTEND_SECOND_LINES, extend->value->int32);
  }
  Tuple *show_date = dict_find(iter, MESSAGE_KEY_SHOW_DATE);
  if (show_date) {
    s_show_date = show_date->value->int32 != 0;
    persist_write_int(PERSIST_KEY_SHOW_DATE, show_date->value->int32);
  }
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  // GRect time_rect = GRect(0, bounds.size.h - 30, bounds.size.w, 30);
  // s_time_layer = text_layer_create(time_rect);
  // text_layer_set_background_color(s_time_layer, GColorClear);
  // text_layer_set_text_color(s_time_layer, s_line_color);
  // text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  // text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void window_unload(Window *window) {
  // text_layer_destroy(s_time_layer);
  layer_destroy(s_canvas_layer);
}

static void init(void) {
  load_settings();

  s_window = window_create();
  window_set_background_color(s_window, s_bg_color);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 64);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  update_angles();
  ensure_smooth_timer();
  s_startup_timer = app_timer_register(SMOOTH_ROTATION_INTERVAL_MS, startup_tick, NULL);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
