#include <pebble.h>

static Window *s_main_window;
static Layer *s_draw_layer, *s_date_layer;
static TextLayer *s_day_label, *s_month_label, *s_battery_layer;
static char s_day_buffer[9], s_month_buffer[12];

typedef struct {
  int hours;
  int minutes;
  int seconds;
} Time;

static Time s_last_time;

static void time_update_proc(Layer *this_layer, GContext *ctx) {
  int posY = 115;
  for (int y=0; y<=s_last_time.hours; y++) {
    int posX = 0;
    int minutes = 60;
    if(y==s_last_time.hours) {
      minutes = s_last_time.minutes;
    }
    for(int x=0; x<minutes; x++) {
      graphics_context_set_fill_color(ctx, GColorFromRGB(170,170,170));
      if(y==11) {
        graphics_context_set_fill_color(ctx, GColorFromRGB(255,69,0));
      }
      if(x==s_last_time.seconds) {
        graphics_context_set_fill_color(ctx, GColorFromRGB(255,69,0));
      }
      graphics_fill_rect(ctx, GRect(posX,posY,2,4), 0, GCornerNone);
      posX += 2;
      if((x+1)%15==0) {
        if((x+1)==30) {
          posX += 2;
        }
        else {
          posX += 1;
        }
      }
    }
    posY -= 5;
  }
}

static void date_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%A", t);
  text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_month_buffer, sizeof(s_month_buffer), "%d %B", t);
  text_layer_set_text(s_month_label, s_month_buffer);  
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.minutes = tick_time->tm_min;
  s_last_time.seconds = tick_time->tm_sec;
  // Redraw
  if(s_draw_layer) {
    layer_mark_dirty(s_draw_layer);
  }

  handle_battery(battery_state_service_peek());
}

static void main_window_load(Window *window) {
  s_draw_layer = layer_create(GRect(10, 24, 124, 119));
  layer_set_update_proc(s_draw_layer, time_update_proc);
  layer_add_child(window_get_root_layer(window), s_draw_layer);

  s_date_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_get_root_layer(window), s_date_layer);

  s_day_label = text_layer_create(GRect(5, 147, 55, 20));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorBlack);
  text_layer_set_text_color(s_day_label, GColorWhite);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_day_label, GTextAlignmentLeft);

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_month_label = text_layer_create(GRect(64, 147, 75, 20));
  text_layer_set_text(s_month_label, s_month_buffer);
  text_layer_set_background_color(s_month_label, GColorBlack);
  text_layer_set_text_color(s_month_label, GColorFromRGB(255,69,0));
  text_layer_set_font(s_month_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_month_label, GTextAlignmentRight);

  layer_add_child(s_date_layer, text_layer_get_layer(s_month_label));

  s_battery_layer = text_layer_create(GRect(5, 0, 100, 14));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorBlack);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_text(s_battery_layer, "100%");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
}

static void main_window_unload(Window *window) {
  layer_destroy(s_draw_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_month_label);
  text_layer_destroy(s_battery_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, GColorBlack);

  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  battery_state_service_subscribe(handle_battery);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}