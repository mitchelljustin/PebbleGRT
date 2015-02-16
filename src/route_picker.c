#include <pebble.h>
#include "route_picker.h"

#define NUM_ROUTES ARRAY_LENGTH(AVAILABLE_ROUTES)

static struct {
    Window *window;
    void (*callback)(int32_t);

    SimpleMenuLayer *menu_layer;
    SimpleMenuSection route_picker_menu_section;
    
    SimpleMenuItem route_items[NUM_ROUTES];
} S;


static void route_selected_callback(int index, void *context) {
    route_id_t route_id = AVAILABLE_ROUTES[index];
    S.callback(route_id);
    S.callback = NULL;
    window_stack_pop(true);
}

static void route_picker_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    for (int i = 0; i < NUM_ROUTES; ++i) {
        S.route_items[i] = (SimpleMenuItem) {
            .title = AVAILABLE_ROUTE_NAMES[i],
            .callback = route_selected_callback
        };
    }

    S.route_picker_menu_section = (SimpleMenuSection) {
        .title = "Choose a Route",
        .num_items = NUM_ROUTES,
        .items = S.route_items
    };

    S.menu_layer = simple_menu_layer_create(
        window_bounds,
        window,
        &S.route_picker_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));
}

static void route_picker_window_unload(Window *window) {
    simple_menu_layer_destroy(S.menu_layer);
}

void push_route_picker_window(void (*callback)(int32_t)) {
    S.window = window_create();
    S.callback = callback;

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = route_picker_window_load,
        .unload = route_picker_window_unload
    });

    window_stack_push(S.window, true);
}
