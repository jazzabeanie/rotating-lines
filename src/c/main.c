#include <pebble.h>

#define PERSIST_KEY_BG_COLOR 1
#define PERSIST_KEY_LINE_COLOR 2

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

static bool s_smooth_rotation = true;
static AppTimer *s_smooth_timer;

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

static void draw_inner_lines(GContext *ctx, GPoint center, int32_t r, int32_t angle) {
  for (int i = 0; i < 12; i++) {
    int32_t offset = i * TRIG_MAX_ANGLE / 24;
    draw_rotating_line(ctx, mark_pivot(center, r / 4, i, 12), r / 2, angle + offset);
  }
}

static void draw_middle_lines(GContext *ctx, GPoint center, int32_t r, int32_t angle) {
  for (int i = 0; i < 60; i++) {
    int32_t offset = i * TRIG_MAX_ANGLE / 120;
    draw_rotating_line(ctx, mark_pivot(center, r * 5 / 8, i, 60), r / 4, angle + offset);
  }
}

static void draw_rim_lines(GContext *ctx, GPoint center, int32_t r, int32_t angle) {
  for (int i = 0; i < 60; i++) {
    int32_t offset = i * TRIG_MAX_ANGLE / 120;
    draw_rotating_line(ctx, mark_pivot(center, r * 7 / 8, i, 60), r / 4, angle + offset);
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

static void draw_time_markers(GContext *ctx, GPoint center, int32_t r_circle) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int hour = t->tm_hour % 12;
  if (hour == 0) hour = 12;
  int minute = t->tm_min;
  int second = t->tm_sec;

  int32_t dist = r_circle + 10;
  char buf[4];

  graphics_context_set_text_color(ctx, s_line_color);

  int hour_pos = (hour % 12) * 5;
  int32_t h_angle = hour_pos * TRIG_MAX_ANGLE / 60;
  snprintf(buf, sizeof(buf), "%d", hour);
  draw_marker_text(ctx, center, dist, h_angle, buf, FONT_KEY_GOTHIC_14_BOLD);

  bool show_minute = wrap_diff(minute, hour_pos, 60) >= 3;
  if (show_minute) {
    int32_t m_angle = minute * TRIG_MAX_ANGLE / 60;
    snprintf(buf, sizeof(buf), "%d", minute);
    draw_marker_text(ctx, center, dist, m_angle, buf, FONT_KEY_GOTHIC_14_BOLD);
  }

  bool clear_of_hour = wrap_diff(second, hour_pos, 60) >= 3;
  bool clear_of_minute = !show_minute || wrap_diff(second, minute, 60) >= 3;
  if (clear_of_hour && clear_of_minute) {
    int32_t s_mark_angle = second * TRIG_MAX_ANGLE / 60;
    snprintf(buf, sizeof(buf), "%d", second);
    draw_marker_text(ctx, center, dist, s_mark_angle, buf, FONT_KEY_GOTHIC_14);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  uint16_t radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2;
  int32_t r = radius - 14;

  graphics_context_set_fill_color(ctx, s_bg_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, s_line_color);
  graphics_context_set_stroke_width(ctx, 2);
  // graphics_draw_circle(ctx, center, r);

  draw_inner_lines(ctx, center, r, s_inner_angle);
  draw_middle_lines(ctx, center, r, s_middle_angle);
  draw_rim_lines(ctx, center, r, s_outer_angle);
  draw_time_markers(ctx, center, r);
}

static void update_angles(void) {
  time_t now;
  uint16_t ms;
  time_ms(&now, &ms);
  struct tm *t = localtime(&now);

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

static void start_smooth_rotation(void) {
  s_smooth_rotation = true;
  if (!s_smooth_timer) {
    s_smooth_timer = app_timer_register(SMOOTH_ROTATION_INTERVAL_MS, smooth_rotation_tick, NULL);
  }
}

static void smooth_rotation_tick(void *data) {
  s_smooth_timer = NULL;
  if (!s_smooth_rotation) return;
  update_angles();
  s_smooth_timer = app_timer_register(SMOOTH_ROTATION_INTERVAL_MS, smooth_rotation_tick, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_angles();
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
  app_message_open(64, 64);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  if (s_smooth_rotation) {
    start_smooth_rotation();
  }
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
