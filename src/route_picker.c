#include <pebble.h>
#include "route_picker.h"
#include "message_types.h"

#define NUM_BUSES 10

#define NUM_INFO_MENU_ITEMS 2
#define NUM_STOP_MENU_ITEMS 3

#define NUM_SECTIONS 2

#define BUFFER_MAX_LEN 128

#define LOADING_STRING "Loading.."

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection route_picker_menu_section;
    SimpleMenuItem info_menu_items[NUM_INFO_MENU_ITEMS];
    SimpleMenuItem stop_menu_items[NUM_STOP_MENU_ITEMS];
    SimpleMenuSection info_menu_section;
    SimpleMenuSection stop_menu_section;
    SimpleMenuSection menu_sections[NUM_SECTIONS];
    
    char trip_id[40];
    char vehicle_id[40];

    char stop_buffers[BUFFER_MAX_LEN][NUM_STOP_MENU_ITEMS];
    char delay_subtitle_buf[BUFFER_MAX_LEN];
} S;

static void send_phone_message_route_picker();

static void route_picker_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    
    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_route_picker();
}

static void send_phone_message_route_picker() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeBusDetail);
    dict_write_cstring(iter, PGKeyBusDetailTripId, S.trip_id);
    dict_write_cstring(iter, PGKeyBusDetailVehicleId, S.vehicle_id);

    app_message_outbox_send();
}

static void route_picker_window_unload(Window *window) {
    app_message_deregister_callbacks();
    simple_menu_layer_destroy(S.menu_layer);
}

void route_picker_app_message_received(DictionaryIterator *iterator, void *context) {

    int32_t index = 0;

    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            default:
                break;
        }
        t = dict_read_next(iterator);
    }

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));
}

void push_route_picker_window(void (*callback)(int32_t)) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = route_picker_window_load,
        .unload = route_picker_window_unload
    });

    Layer *window_layer = window_get_root_layer(S.window);
    GRect window_bounds = layer_get_bounds(window_layer);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    app_message_register_inbox_received(route_picker_app_message_received);

    window_stack_push(S.window, true);
}
