#include <pebble.h>
#include "bus_detail.h"
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
    SimpleMenuSection bus_detail_menu_section;
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

static void send_phone_message_bus_detail();

static void bus_detail_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    
    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_bus_detail();
}

static void send_phone_message_bus_detail() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeBusDetail);
    dict_write_cstring(iter, PGKeyBusDetailTripId, S.trip_id);
    dict_write_cstring(iter, PGKeyBusDetailVehicleId, S.vehicle_id);

    app_message_outbox_send();
}

static void bus_detail_window_unload(Window *window) {
    app_message_deregister_callbacks();
    simple_menu_layer_destroy(S.menu_layer);
}

void bus_detail_app_message_received(DictionaryIterator *iterator, void *context) {

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

void push_bus_detail_window(char *trip_id, char *vehicle_id) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = bus_detail_window_load,
        .unload = bus_detail_window_unload
    });
    
    strncpy(S.trip_id, trip_id, 40); 
    strncpy(S.vehicle_id, vehicle_id, 40);

    strncpy(S.delay_subtitle_buf, LOADING_STRING, BUFFER_MAX_LEN);

    for (int i = 0; i < NUM_STOP_MENU_ITEMS; ++i) {
        (&S.stop_menu_items[i])->title = LOADING_STRING;
    }

    S.info_menu_items[0] = (SimpleMenuItem) {
        .title = LOADING_STRING,
        .subtitle = LOADING_STRING
    };
    S.info_menu_items[1] = (SimpleMenuItem) {
        .title = "Delay",
        .subtitle = S.delay_subtitle_buf
    };

    S.menu_sections[0] = S.info_menu_section;
    S.menu_sections[1] = S.stop_menu_section;

    Layer *window_layer = window_get_root_layer(S.window);
    GRect window_bounds = layer_get_bounds(window_layer);

    S.menu_layer = simple_menu_layer_create(window_bounds,
        S.window,
        S.menu_sections,
        NUM_SECTIONS,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    app_message_register_inbox_received(bus_detail_app_message_received);

    window_stack_push(S.window, true);
}
