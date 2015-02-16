#include <pebble.h>
#include "stop_info.h"

#define NUM_STOP_MENU_ITEMS 3

#define ID_BUFFER_MAX_LEN 40
#define TITLE_BUFFER_MAX_LEN 128

#define LOADING_STRING "Loading.."

struct stop_s {
    char name[TITLE_BUFFER_MAX_LEN];
    char time[TITLE_BUFFER_MAX_LEN];
};

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection stop_info_menu_section;
    SimpleMenuItem stop_menu_items[NUM_STOP_MENU_ITEMS];

    char bus_title[TITLE_BUFFER_MAX_LEN];
    char bus_subtitle[TITLE_BUFFER_MAX_LEN];

    stop_id_t stop_id;
    route_id_t route_id;
    
    struct stop_s stops[NUM_STOP_MENU_ITEMS];
} S;

static void send_phone_message_stop_info();

static void stop_info_window_load(Window *window) {

    for (int i = 0; i < NUM_STOP_MENU_ITEMS; ++i) {
        struct stop_s *stop = &S.stops[i];
        SimpleMenuItem *menu_item = &S.stop_menu_items[i];
        menu_item->title = stop->time;
        menu_item->subtitle = stop->name;
        strncpy(stop->time, LOADING_STRING, TITLE_BUFFER_MAX_LEN);
        stop->name[0] = '\0';
    }

    S.stop_info_menu_section = (SimpleMenuSection) {
        .title = "Upcoming Buses",
        .items = S.stop_menu_items,
        .num_items = NUM_STOP_MENU_ITEMS
    };

    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &S.stop_info_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_stop_info();
}

static void send_phone_message_stop_info() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeStopInfo);
    dict_write_int32(iter, PGKeyStopId, S.stop_id);
    dict_write_int32(iter, PGKeyRouteId, S.route_id);

    app_message_outbox_send();
}

static void stop_info_window_unload(Window *window) {
    app_message_deregister_callbacks();
    simple_menu_layer_destroy(S.menu_layer);
}

void stop_info_app_message_received(DictionaryIterator *iterator, void *context) {
    int32_t index = 0;

    char *stop_name = NULL;
    char *stop_time = NULL;

    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                break;
            case PGKeyStopInfoIndex:
                index = t->value->int32;
                break;
            case PGKeyStopInfoStopTime:
                stop_time = t->value->cstring;
                break;
            case PGKeyStopInfoStopName:
                stop_name = t->value->cstring;
                break;
            default:
                break;
        }
        t = dict_read_next(iterator);
    }

    struct stop_s *stop = &S.stops[index];
    strncpy(stop->time, stop_time, TITLE_BUFFER_MAX_LEN);
    strncpy(stop->name, stop_name, TITLE_BUFFER_MAX_LEN);

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));
}

void push_stop_info_window(stop_id_t stop_id, route_id_t route_id) {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "stop info: stop_id = %li, route_id = %li", stop_id, route_id);
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = stop_info_window_load,
        .unload = stop_info_window_unload
    });

    S.stop_id = stop_id;
    S.route_id = route_id;

    app_message_register_inbox_received(stop_info_app_message_received);

    window_stack_push(S.window, true);
}
