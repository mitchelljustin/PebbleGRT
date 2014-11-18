#include <pebble.h>
#include "message_types.h"

#define MAX_NUM_MENU_ITEMS 7
static const int REFRESH_INTERVAL = 27;

static Window *s_window;

static SimpleMenuItem s_menu_items[MAX_NUM_MENU_ITEMS];

static SimpleMenuSection s_default_menu_section = {
  .items = s_menu_items,
  .num_items = MAX_NUM_MENU_ITEMS,
  .title = "Nearby Buses"
};

static SimpleMenuLayer *s_nearby_buses_layer;

static void send_report_nearby_buses_msg() {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, (uint8_t) PBMessageTypeReportNearbyBuses);

  // Send the message!
  app_message_outbox_send();
}

static void window_load(Window *window) {
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
  send_report_nearby_buses_msg();
}

static void window_unload() {
  simple_menu_layer_destroy(s_nearby_buses_layer);
  window_destroy(s_window);
  app_message_deregister_callbacks();
  tick_timer_service_unsubscribe();
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

  if(tick_time->tm_sec % REFRESH_INTERVAL == 0) {
    send_report_nearby_buses_msg();
  }
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

  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case 0:
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TYPE received with value %d", (int)t->value->int32);
        break;
      default: {
        int index = t->key - 1;
        char *bus_string = t->value->cstring;
        APP_LOG(APP_LOG_LEVEL_INFO, "'%lu' received with value %s", t->key, bus_string);
        const char *description = split_distance_and_desc(bus_string);
        SimpleMenuItem bus_item = {
          .title = bus_string,
          .subtitle = description,
          .icon = NULL,
          .callback = NULL
        };
        s_menu_items[index] = bus_item;
        break;
      }
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }

  layer_mark_dirty(simple_menu_layer_get_layer(s_nearby_buses_layer));
}


Window *create_nearby_buses_window() {
  s_window = window_create();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  return s_window;
}
