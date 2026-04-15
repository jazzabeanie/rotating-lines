#include <pebble.h>

#define PERSIST_KEY_BG_COLOR   1
#define PERSIST_KEY_LINE_COLOR 2

static Window *s_window;
static Layer *s_canvas_layer;
static GColor s_bg_color;
static GColor s_line_color;
static int32_t s_angle;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  uint16_t radius = (bounds.size.w < bounds.size.h ? bounds.size.w : bounds.size.h) / 2;

  graphics_context_set_fill_color(ctx, s_bg_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, s_line_color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, center, radius - 2);

  int32_t r = radius - 2;
  GPoint end = {
    .x = center.x + (sin_lookup(s_angle) * r / TRIG_MAX_RATIO),
    .y = center.y - (cos_lookup(s_angle) * r / TRIG_MAX_RATIO),
  };
  graphics_draw_line(ctx, center, end);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_angle = (s_angle + TRIG_MAX_ANGLE / 2) % TRIG_MAX_ANGLE;
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
}

static void window_unload(Window *window) {
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

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
