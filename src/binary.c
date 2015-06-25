#include "pebble.h"

static Window *window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer, *s_battery_layer;
static TextLayer *s_day_label, *s_num_label;

static char s_num_buffer[4], s_day_buffer[6];

static void bg_update_proc(Layer *layer, GContext *ctx) {
  printf("bg update");
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void handle_battery(BatteryChargeState charge_state, GContext *ctx) {
  GRect bounds = layer_get_bounds(s_battery_layer);
  int width = bounds.size.w;
  int height = bounds.size.h;
  
  int cp = charge_state.charge_percent;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  graphics_draw_rect(ctx, GRect(0, height - 4, width, 4));

  graphics_fill_rect(ctx, GRect(width*(100 - cp)/100, height - 4, width, 4), 0, GCornerNone);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  printf("handle update proc");
  GRect bounds = layer_get_bounds(layer);
  int width = bounds.size.w;
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  int n, c, k;
  
  n = t->tm_sec;
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);

  //seconds
  for (c = 5; c >= 0; c--)
  {
    k = n >> c;
    
    GPoint point = GPoint(width - ((c+1) * width/6) + width/12, width/6*4);
    
    if (k & 1)
      graphics_fill_circle(ctx, point, width/13);  
    else
      graphics_draw_circle(ctx, point, width/13);  
  }
  //minutes
  n = t->tm_min;
  for (c = 5; c >= 0; c--)
  {
    k = n >> c;
    
    GPoint point = GPoint(width - ((c+1) * width/6) + width/12, width/6*2.5);
    
    if (k & 1)
      graphics_fill_circle(ctx, point, width/13);  
    else
      graphics_draw_circle(ctx, point, width/13);  
  }
  //hours
  n = t->tm_hour;
  for (c = 4; c >= 0; c--)
  {
    k = n >> c;
    GPoint point = GPoint(width - ((c+1) * width/6) + width/12, width/6*1);
    
    if (k & 1)
      graphics_fill_circle(ctx, point, width/13);  
    else
      graphics_draw_circle(ctx, point, width/13);  
  }

  handle_battery(battery_state_service_peek(), ctx);
}

static void date_update_proc(Layer *layer, GContext *ctx) {
  printf("date update proc");
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
  text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
  text_layer_set_text(s_num_label, s_num_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  printf("window Load");
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);
  printf("background");
  
  
  s_date_layer = layer_create(bounds);
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);
  printf("date");
  s_day_label = text_layer_create(GRect(46, 130, 27, 20));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorBlack);
  text_layer_set_text_color(s_day_label, GColorWhite);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  printf("daylabel");
  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_num_label = text_layer_create(GRect(73, 130, 18, 20));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorBlack);
  text_layer_set_text_color(s_num_label, GColorWhite);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  printf("num label");
  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));
    printf("set proc0");
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
    printf("set proc1");
  layer_add_child(window_layer, s_hands_layer);
    printf("set proc2");
  
  s_battery_layer = layer_create(bounds);
  layer_add_child(window_layer, s_battery_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);

  layer_destroy(s_hands_layer);
}

static void init() {
  printf("init");
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  s_day_buffer[0] = '\0';
  s_num_buffer[0] = '\0';

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
