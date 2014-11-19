#include <pebble.h>
#include "message_types.h"
#include "pgbus.h"

#define MAX_NUM_NEARBY_BUSES 6
static const int REFRESH_INTERVAL = 27;

static Window *s_window;

static SimpleMenuItem s_menu_items[MAX_NUM_NEARBY_BUSES];
static struct PGBus *s_buses[MAX_NUM_NEARBY_BUSES] = { NULL };

static SimpleMenuSection s_default_menu_section = {
  .items = s_menu_items,
  .num_items = MAX_NUM_NEARBY_BUSES,
  .title = "Nearby Buses"
};

static SimpleMenuLayer *s_nearby_buses_layer;

static void send_report_nearby_buses_msg() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_uint8(iter, PGKeyMessageType, (uint8_t) PGMessageTypeReportNearbyBuses);

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

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);

  while(t != NULL) {
    switch (t->key) {
      case 0:
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TYPE received with value %d", (int)t->value->int32);
        break;
      default: {
        APP_LOG(APP_LOG_LEVEL_INFO, "'%lu' received with value %s", t->key, t->value->cstring);
        int index = t->key - 1;
        struct PGBus **bus = &s_buses[index];
        APP_LOG(APP_LOG_LEVEL_INFO, "Bus at index %i = %p", index, *bus);
        if (*bus != NULL) {
          pgbus_destroy(*bus);
        }
        *bus = pgbus_parse_from_string(t->value->cstring);
        SimpleMenuItem bus_item = {
          .title = (*bus)->distance,
          .subtitle = (*bus)->description,
          .icon = NULL,
          .callback = NULL
        };
        s_menu_items[index] = bus_item;
        break;
      }
    }

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
