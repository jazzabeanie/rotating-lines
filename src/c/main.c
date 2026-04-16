#include <pebble.h>

#define PERSIST_KEY_BG_COLOR 1
#define PERSIST_KEY_LINE_COLOR 2

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer *s_time_layer;
static GColor s_bg_color;
static GColor s_line_color;
static int32_t s_angle;
static char s_time_buf[12];

static GPoint hour_pivot(GPoint center, int32_t r, int hour) {
  int32_t hour_angle = hour * TRIG_MAX_ANGLE / 12;
  return (GPoint){
    .x = center.x + sin_lookup(hour_angle) * (r / 2) / TRIG_MAX_RATIO,
    .y = center.y - cos_lookup(hour_angle) * (r / 2) / TRIG_MAX_RATIO,
  };
}

static void draw_rotating_line(GContext *ctx, GPoint pivot, int32_t length, int32_t angle) {
  int32_t dx = sin_lookup(angle) * length / TRIG_MAX_RATIO;
  int32_t dy = cos_lookup(angle) * length / TRIG_MAX_RATIO;
  GPoint p1 = { .x = pivot.x - dx / 2, .y = pivot.y + dy / 2 };
  GPoint p2 = { .x = pivot.x + dx / 2, .y = pivot.y - dy / 2 };
  graphics_draw_line(ctx, p1, p2);
}

static void draw_clock_lines(GContext *ctx, GPoint center, int32_t r, int32_t angle) {
  for (int hour = 0; hour < 12; hour++) {
    draw_rotating_line(ctx, hour_pivot(center, r, hour), r, angle);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  uint16_t radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2;

  graphics_context_set_fill_color(ctx, s_bg_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, s_line_color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, radius - 2);

  draw_clock_lines(ctx, center, radius - 2, s_angle);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_angle = tick_time->tm_sec * (TRIG_MAX_ANGLE / 2) / 60;
  strftime(s_time_buf, sizeof(s_time_buf), "%H:%M:%S", tick_time);
  text_layer_set_text(s_time_layer, s_time_buf);
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
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

  GRect time_rect = GRect(0, bounds.size.h - 30, bounds.size.w, 30);
  s_time_layer = text_layer_create(time_rect);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, s_line_color);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
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
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
