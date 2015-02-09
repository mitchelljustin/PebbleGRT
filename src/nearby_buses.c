#include <pebble.h>
#include "nearby_buses.h"

#define MAX_NUM_BUSES 10

#define NEARBY_BUS_TITLE_MAX_LEN 128

#define LOADING_STRING "Loading.."

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection nearby_buses_menu_section;
    char nearby_buses_items_titles[NEARBY_BUS_TITLE_MAX_LEN][10];
    SimpleMenuItem nearby_buses_items[MAX_NUM_BUSES];
} S;

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    for (int i = 0; i < MAX_NUM_BUSES; ++i) {
        strncpy(S.nearby_buses_items_titles[i], LOADING_STRING, strlen(LOADING_STRING));
        S.nearby_buses_items[i] = (SimpleMenuItem) {
            .title = S.nearby_buses_items_titles[i]
        };
    }

    S.nearby_buses_menu_section = (SimpleMenuSection) {
        .title = "Nearby Buses",
        .num_items = MAX_NUM_BUSES,
        .items = S.nearby_buses_items
    };

    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &S.nearby_buses_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));
}

static void window_unload(Window *window) {
    simple_menu_layer_destroy(S.menu_layer);
}

void app_message_received(DictionaryIterator *iterator, void *context) {
    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {

        t = dict_read_next(iterator);
    }
}

void push_nearby_buses_window(int index, void *context) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    app_message_register_inbox_received(app_message_received);

    window_stack_push(S.window, true);
}

