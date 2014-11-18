#include <pebble.h>
#include "nearby_buses.h"

static Window *s_main_window;

static void open_nearby_buses(int index, void *context);

static SimpleMenuItem s_main_menu_items[1];

static SimpleMenuSection s_main_menu_section = {
  .items = s_main_menu_items,
  .num_items = 1,
  .title = "Actions"
};

static SimpleMenuLayer *s_main_menu_layer;

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  s_main_menu_layer = simple_menu_layer_create(window_bounds,
                                              window,
                                              &s_main_menu_section,
                                              1,
                                              NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_main_menu_layer));
}

static void main_window_unload(Window *window) {
  simple_menu_layer_destroy(s_main_menu_layer);
}

static void open_nearby_buses(int index, void *context) {
  Window *nearby_buses_window = create_nearby_buses_window();
  window_stack_push(nearby_buses_window, true);
}

static void setup_main_menu_items() {
  s_main_menu_items[0] = (SimpleMenuItem) {
    .title = "Nearby buses",
    .callback = open_nearby_buses
  };
}

static void init() {
  setup_main_menu_items();

  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
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
