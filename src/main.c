#include <pebble.h>
#include "nearby_buses.h"
#include "nearby_stops.h"

static struct {
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection actions_menu_section;
    SimpleMenuItem actions_menu_items[2];
    Window *window;
} S;

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &S.actions_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));
}

static void window_unload(Window *window) {
    simple_menu_layer_destroy(S.menu_layer);
}

static void init() {
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    S.actions_menu_items[0] = (SimpleMenuItem) {
        .title = "Nearby Buses",
        .callback = push_nearby_buses_window
    };
    S.actions_menu_items[1] = (SimpleMenuItem) {
        .title = "Nearby Stops",
        .callback = push_nearby_stops_window
    };
    S.actions_menu_section = (SimpleMenuSection) {
        .num_items = 2,
        .items = S.actions_menu_items,
        .title = "PebbleGRT"
    };

    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    window_stack_push(S.window, true);
}

static void deinit() {
    window_destroy(S.window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
    return 0;
}
