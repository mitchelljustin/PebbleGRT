#include <pebble.h>

#define MAX_NUM_MENU_ITEMS 10

static SimpleMenuItem s_menu_items[MAX_NUM_MENU_ITEMS];  

#define KEY_TYPE 0

static Window *s_main_window;
static SimpleMenuLayer *s_nearby_buses_layer;
static SimpleMenuSection s_default_menu_section = {
  .items = s_menu_items,
  .num_items = 10,
  .title = "Nearby Buses"
};

static void main_window_load(Window *window) {
  s_menu_items[0] = (SimpleMenuItem) {
    .title = "Loading.."
  };
  
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  s_nearby_buses_layer = simple_menu_layer_create(window_bounds,
                                                  window,
                                                  &s_default_menu_section,
                                                  1,
                                                  NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_nearby_buses_layer));
}

static void main_window_unload(Window *window) {
  simple_menu_layer_destroy(s_nearby_buses_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

}

static char *split_distance_and_desc(char *bus_string) {
  while (*bus_string != ';') {
    bus_string++;
  }
  *bus_string = '\0';
  return bus_string + 1;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // TODO: parse bus data
    // Get the first pair
  Tuple *t = dict_read_first(iterator);

  int index = 0;
  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case KEY_TYPE:
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TYPE received with value %d", (int)t->value->int32);
        break;
      default: {
        char *bus_string = t->value->cstring;
        const char *description = split_distance_and_desc(bus_string);
        SimpleMenuItem bus_item = {
          .title = bus_string,
          .subtitle = description,
          .icon = NULL,
          .callback = NULL
        };
        s_menu_items[t->key - 1] = bus_item;
        index += 1;
        break;
      }
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
  
  s_default_menu_section.num_items = index + 1;
  layer_mark_dirty(simple_menu_layer_get_layer(s_nearby_buses_layer));
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
