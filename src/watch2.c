#include <pebble.h>

#define KEY_SECONDS 0x0

static Window *s_main_window;
static Layer *s_draw_layer, *s_date_layer, *s_bluetooth_layer, *s_12hour_layer;
static TextLayer *s_day_label, *s_month_label, *s_battery_layer, *s_12hour_label;
static char s_day_buffer[10], s_month_buffer[12];

static GColor primary_colour;
static GColor highlight_colour;
static GColor midday_highlight_colour;
static GColor text_primary_colour;
static GColor text_secondary_colour;

typedef struct {
  int hours;
  int minutes;
  int seconds;
} Time;

static Time s_last_time;
bool bluetooth = true;
bool second_hand = true;

//draw call for 12 hour time
static void time_update_proc(Layer *this_layer, GContext *ctx) {
  int posY = 111;
  int start = 0;
  int end = 12;

  if(s_last_time.hours >= 12) {
    start = 12;
    end = 24;
  }

  for (int y=start; y<=end; y++) {
    int posX = 0;
    for(int x=0; x<60; x++) {
      if((x<=1) && (y%2!=0)) {
        graphics_context_set_fill_color(ctx, highlight_colour);
      }
      else {
        graphics_context_set_fill_color(ctx, primary_colour);
      }

      //midday
      if(y==(end-1)) {
        graphics_context_set_fill_color(ctx, midday_highlight_colour);
      }
      
      if(y<s_last_time.hours) {
        graphics_fill_rect(ctx, GRect(posX,posY,2,8), 0, GCornerNone);
      }
      else if(y==s_last_time.hours && x<s_last_time.minutes) {
        graphics_fill_rect(ctx, GRect(posX,posY,2,8), 0, GCornerNone);
      }
      //second hand
      if(second_hand == true) {
        if(x==s_last_time.seconds && y<end) {
          graphics_context_set_fill_color(ctx, highlight_colour);
          graphics_fill_rect(ctx, GRect(posX,posY,2,8), 0, GCornerNone);
        }
      }
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
    posY -= 9;
  }
}

//draw call for 24 hour time
static void time_update_proc_24(Layer *this_layer, GContext *ctx) {
  int posY = 115;
  for (int y=0; y<=23; y++) {
    int posX = 0;
    for(int x=0; x<60; x++) {
      if((x<=1) && (y%2!=0)) {
        graphics_context_set_fill_color(ctx, highlight_colour);
      }
      else {
        graphics_context_set_fill_color(ctx, primary_colour);
      }

      //midday
      if(y==11) {
        graphics_context_set_fill_color(ctx, midday_highlight_colour);
      }
      
      if(y<s_last_time.hours) {
        graphics_fill_rect(ctx, GRect(posX,posY,2,4), 0, GCornerNone);
      }
      else if(y==s_last_time.hours && x<s_last_time.minutes) {
        graphics_fill_rect(ctx, GRect(posX,posY,2,4), 0, GCornerNone);
      }
      //second hand
      if(second_hand == true) {
        if(x==s_last_time.seconds) {
          graphics_context_set_fill_color(ctx, highlight_colour);
          graphics_fill_rect(ctx, GRect(posX,posY,2,4), 0, GCornerNone);
        }
      }
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

static void twelve_hour_update_proc(Layer *layer, GContext *ctx) {
  if(s_last_time.hours >= 12) {
    text_layer_set_text(s_12hour_label, "PM");
  }
  else {
    text_layer_set_text(s_12hour_label, "AM");
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

static void bluetooth_update_proc(Layer *layer, GContext *ctx) {
  if(bluetooth == true) {
    graphics_context_set_fill_color(ctx, text_secondary_colour);
    graphics_fill_circle(ctx, GPoint(2.5,2.5), 2.5);
  }
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "...");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_bluetooth(bool connected) {
  bluetooth = connected;
  vibes_short_pulse();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.minutes = tick_time->tm_min;
  s_last_time.seconds = tick_time->tm_sec;

  handle_battery(battery_state_service_peek());
  
  // Redraw
  if(s_draw_layer) {
    layer_mark_dirty(s_draw_layer);
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  while(t != NULL) {
    switch(t->key) {
    case KEY_SECONDS:
      //APP_LOG(APP_LOG_LEVEL_ERROR, "value: %d", (int)t->value->int32);
      if((int)t->value->int32 == 1)
      {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        second_hand = true;
      }
      else if((int)t->value->int32 == 0)
      {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
        second_hand = false;
      }
      break;
    default:
      //APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    t = dict_read_next(iterator);
  }
}

static void main_window_load(Window *window) {
  s_draw_layer = layer_create(GRect(10, 24, 124, 119));
  if(clock_is_24h_style()) {
    layer_set_update_proc(s_draw_layer, time_update_proc_24);
  }
  else {
    layer_set_update_proc(s_draw_layer, time_update_proc);
  }
  layer_add_child(window_get_root_layer(window), s_draw_layer);

  s_date_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_get_root_layer(window), s_date_layer);

  s_bluetooth_layer = layer_create(GRect(134, 5, 5, 5));
  layer_set_update_proc(s_bluetooth_layer, bluetooth_update_proc);
  layer_add_child(window_get_root_layer(window), s_bluetooth_layer);

  if(!clock_is_24h_style()) {
    s_12hour_layer = layer_create(GRect(0, 14, 144, 14));
    layer_set_update_proc(s_12hour_layer, twelve_hour_update_proc);
    layer_add_child(window_get_root_layer(window), s_12hour_layer);
  }

  s_day_label = text_layer_create(GRect(5, 147, 65, 20));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorBlack);
  text_layer_set_text_color(s_day_label, text_primary_colour);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_day_label, GTextAlignmentLeft);

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_month_label = text_layer_create(GRect(74, 147, 65, 20));
  text_layer_set_text(s_month_label, s_month_buffer);
  text_layer_set_background_color(s_month_label, GColorBlack);
  text_layer_set_text_color(s_month_label, text_secondary_colour);
  text_layer_set_font(s_month_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_month_label, GTextAlignmentRight);

  layer_add_child(s_date_layer, text_layer_get_layer(s_month_label));

  s_battery_layer = text_layer_create(GRect(5, 0, 100, 14));
  text_layer_set_text_color(s_battery_layer, text_primary_colour);
  text_layer_set_background_color(s_battery_layer, GColorBlack);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_text(s_battery_layer, "100%");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));

  if(!clock_is_24h_style()) {
    s_12hour_label = text_layer_create(GRect(0, 0, 144, 14));
    text_layer_set_text_color(s_12hour_label, text_secondary_colour);
    text_layer_set_background_color(s_12hour_label, GColorBlack);
    text_layer_set_font(s_12hour_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_12hour_label, GTextAlignmentCenter);
    text_layer_set_text(s_12hour_label, "AM");
    layer_add_child(s_12hour_layer, text_layer_get_layer(s_12hour_label));
  }
}

static void main_window_unload(Window *window) {
  layer_destroy(s_draw_layer);
  layer_destroy(s_date_layer);
  layer_destroy(s_bluetooth_layer);
  layer_destroy(s_12hour_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_month_label);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_12hour_label);
}

static void init() {
  //Pebble colour setup
  #ifdef PBL_COLOR
    primary_colour = GColorFromRGB(170,170,170);
    highlight_colour = GColorFromRGB(255,69,0);
    text_primary_colour = GColorWhite;
    text_secondary_colour = GColorFromRGB(255,69,0);
    midday_highlight_colour = GColorFromRGB(255,69,0);
  //Original Pebble setup
  #else
    primary_colour = GColorWhite;
    highlight_colour = GColorBlack;
    text_primary_colour = GColorWhite;
    text_secondary_colour = GColorWhite;
    midday_highlight_colour = GColorWhite;
  #endif

  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, GColorBlack);

  window_stack_push(s_main_window, true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}