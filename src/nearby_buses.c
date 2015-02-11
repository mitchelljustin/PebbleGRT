#include <pebble.h>
#include "nearby_buses.h"
#include "message_types.h"

#define NUM_BUSES 10

#define BUFFER_MAX_LEN 128

#define LOADING_STRING "Loading.."

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection nearby_buses_menu_section;
    char nearby_buses_items_titles[NUM_BUSES][BUFFER_MAX_LEN];
    char nearby_buses_items_subtitles[NUM_BUSES][BUFFER_MAX_LEN];
    SimpleMenuItem nearby_buses_items[NUM_BUSES];
} S;

static void send_phone_message_nearby_buses();

static void nearby_buses_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    for (int i = 0; i < NUM_BUSES; ++i) {
        char *title = S.nearby_buses_items_titles[i];
        char *subtitle = S.nearby_buses_items_subtitles[i];
        subtitle[0] = '\0';
        strncpy(title, LOADING_STRING, strlen(LOADING_STRING));
        S.nearby_buses_items[i] = (SimpleMenuItem) {
            .title = title,
            .subtitle = subtitle
        };
    }

    S.nearby_buses_menu_section = (SimpleMenuSection) {
        .title = "Nearby Buses",
        .num_items = NUM_BUSES,
        .items = S.nearby_buses_items
    };

    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &S.nearby_buses_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_nearby_buses();
}

static void send_phone_message_nearby_buses() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeNearbyBuses);

    app_message_outbox_send();
}

static void nearby_buses_window_unload(Window *window) {
    app_message_deregister_callbacks();
    simple_menu_layer_destroy(S.menu_layer);
}

void nearby_buses_app_message_received(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "bus received");

    int32_t index = 0;
    char *name = NULL;
    char *distance = NULL;

    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                break;
            case PGKeyBusIndex:
                index = t->value->uint32;
                break;
            case PGKeyBusName:
                name = t->value->cstring;
                break;
            case PGKeyBusDistance:
                distance = t->value->cstring;
                break;
            default:
                break;
                
        }
        t = dict_read_next(iterator);
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nearby bus: { i: %li, name: %s, distance: %s }", index, name, distance);

    strncpy(S.nearby_buses_items_titles[index], distance, BUFFER_MAX_LEN);
    strncpy(S.nearby_buses_items_subtitles[index], name, BUFFER_MAX_LEN);

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));
}

void push_nearby_buses_window(int index, void *context) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = nearby_buses_window_load,
        .unload = nearby_buses_window_unload
    });

    app_message_register_inbox_received(nearby_buses_app_message_received);

    window_stack_push(S.window, true);
}

