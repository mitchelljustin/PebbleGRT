#include <pebble.h>
#include "nearby_stops.h"
#include "defines.h"
#include "route_picker.h"
#include "stop_info.h"

#define NUM_STOPS 10

#define TITLE_MAX_BUFFER_LEN 128

#define LOADING_STRING "Loading.."

struct stop_s {
    char distance[TITLE_MAX_BUFFER_LEN];
    char description[TITLE_MAX_BUFFER_LEN];
    int32_t stop_id;
};

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection nearby_stops_menu_section;
    struct stop_s nearby_stops[NUM_STOPS];
    SimpleMenuItem nearby_stops_items[NUM_STOPS];
    int nearby_stop_selected_index;
} S;

static void send_phone_message_nearby_stops() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeNearbyStops);

    app_message_outbox_send();
}

static void route_picker_returned(route_id_t route_id) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "route id picked: %li", route_id);
    stop_id_t stop_id = S.nearby_stops[S.nearby_stop_selected_index].stop_id;
    push_stop_info_window(stop_id, route_id);
}

static void nearby_stop_selected(int index, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "stop selected at index: %i", index);
    S.nearby_stop_selected_index = index;
    push_route_picker_window(route_picker_returned);
}

static void nearby_stops_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    for (int i = 0; i < NUM_STOPS; ++i) {
        char *title = S.nearby_stops[i].distance;
        char *subtitle = S.nearby_stops[i].description;
        subtitle[0] = '\0';
        strncpy(title, LOADING_STRING, strlen(LOADING_STRING));
        S.nearby_stops_items[i] = (SimpleMenuItem) {
            .title = title,
            .subtitle = subtitle,
            .callback = nearby_stop_selected
        };
    }

    S.nearby_stops_menu_section = (SimpleMenuSection) {
        .title = "Nearby Stops",
        .num_items = NUM_STOPS,
        .items = S.nearby_stops_items
    };

    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &S.nearby_stops_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_nearby_stops();
}

static void nearby_stops_window_unload(Window *window) {
    app_message_deregister_callbacks();
    simple_menu_layer_destroy(S.menu_layer);
}

void nearby_stops_app_message_received(DictionaryIterator *iterator, void *context) {
    int32_t index = 0;
    char *name = NULL;
    char *distance = NULL;
    int32_t stop_id = 0;

    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                break;
            case PGKeyStopIndex:
                index = t->value->int32;
                break;
            case PGKeyStopName:
                name = t->value->cstring;
                break;
            case PGKeyStopDistance:
                distance = t->value->cstring;
                break;
            case PGKeyStopId:
                stop_id = t->value->int32;
                break;
            default:
                break;
                
        }
        t = dict_read_next(iterator);
    }

    struct stop_s *stop = &S.nearby_stops[index];
    strncpy(stop->distance, distance, TITLE_MAX_BUFFER_LEN);
    strncpy(stop->description, name, TITLE_MAX_BUFFER_LEN);
    stop->stop_id = stop_id;

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nearby stop: i=%li, name='%s', distance='%s'", index, name, distance);
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! %x", reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


void push_nearby_stops_window(int index, void *context) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = nearby_stops_window_load,
        .unload = nearby_stops_window_unload
    });

    app_message_register_inbox_received(nearby_stops_app_message_received);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    window_stack_push(S.window, true);
}

