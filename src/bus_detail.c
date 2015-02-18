#include <pebble.h>
#include "bus_detail.h"
#include "defines.h"

#define NUM_BUSES 10

#define NUM_INFO_MENU_ITEMS 2
#define NUM_STOP_MENU_ITEMS 3

#define NUM_SECTIONS 2

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
    SimpleMenuSection bus_detail_menu_section;
    SimpleMenuItem info_menu_items[NUM_INFO_MENU_ITEMS];
    SimpleMenuItem stop_menu_items[NUM_STOP_MENU_ITEMS];
    SimpleMenuSection info_menu_section;
    SimpleMenuSection stop_menu_section;
    SimpleMenuSection menu_sections[NUM_SECTIONS];

    char bus_title[TITLE_BUFFER_MAX_LEN];
    char bus_subtitle[TITLE_BUFFER_MAX_LEN];

    char trip_id[ID_BUFFER_MAX_LEN];
    char vehicle_id[ID_BUFFER_MAX_LEN];
    
    struct stop_s stops[NUM_STOP_MENU_ITEMS];
    
    char delay_subtitle_buf[TITLE_BUFFER_MAX_LEN];
} S;


static void send_phone_message_bus_detail();

static void bus_detail_timer(struct tm *tick_time, TimeUnits units_changed) {
    if (tick_time->tm_sec % 30 == 0) {
        send_phone_message_bus_detail();
    }
}

static void bus_detail_window_load(Window *window) {
    strncpy(S.delay_subtitle_buf, LOADING_STRING, TITLE_BUFFER_MAX_LEN);

    for (int i = 0; i < NUM_STOP_MENU_ITEMS; ++i) {
        struct stop_s *stop = &S.stops[i];
        SimpleMenuItem *menu_item = &S.stop_menu_items[i];
        menu_item->title = stop->time;
        menu_item->subtitle = stop->name;
        strncpy(stop->time, LOADING_STRING, TITLE_BUFFER_MAX_LEN);
        stop->name[0] = '\0';
    }

    S.info_menu_items[0] = (SimpleMenuItem) {
        .title = S.bus_title,
        .subtitle = S.bus_subtitle
    };
    S.info_menu_items[1] = (SimpleMenuItem) {
        .title = "Delay",
        .subtitle = S.delay_subtitle_buf
    };

    S.info_menu_section = (SimpleMenuSection) {
        .title = "Bus Detail",
        .num_items = NUM_INFO_MENU_ITEMS,
        .items = S.info_menu_items
    };

    S.stop_menu_section = (SimpleMenuSection) {
        .title = "Upcoming Stops",
        .num_items = NUM_STOP_MENU_ITEMS,
        .items = S.stop_menu_items
    };

    S.menu_sections[0] = S.info_menu_section;
    S.menu_sections[1] = S.stop_menu_section;

    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    S.menu_layer = simple_menu_layer_create(window_bounds,
        window,
        S.menu_sections,
        NUM_SECTIONS,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(S.menu_layer));

    send_phone_message_bus_detail();
}

static void send_phone_message_bus_detail() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeBusDetail);
    dict_write_cstring(iter, PGKeyBusTripId, S.trip_id);
    dict_write_cstring(iter, PGKeyBusVehicleId, S.vehicle_id);

    app_message_outbox_send();
}

static void bus_detail_window_unload(Window *window) {
    app_message_deregister_callbacks();
    simple_menu_layer_destroy(S.menu_layer);
}

void bus_detail_app_message_received(DictionaryIterator *iterator, void *context) {
    enum MessageType message_type = MessageTypeNone;

    int32_t index = 0;

    char *stop_name_or_delay = NULL;
    char *stop_time = NULL;

    Tuple *t = dict_read_first(iterator);

    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                message_type = (enum MessageType) t->value->uint8;
            case PGKeyBusDetailStopName:
                stop_name_or_delay = t->value->cstring;
                break;
            case PGKeyBusDetailDelay:
                stop_name_or_delay = t->value->cstring;
                break;
            case PGKeyBusDetailStopTime:
                stop_time = t->value->cstring;
                break;
            case PGKeyBusDetailIndex:
                index = t->value->int32;
                break;
            default:
                break;
        }
        t = dict_read_next(iterator);
    }

    switch (message_type) {
        case MessageTypeBusDetailDelay:
            strncpy(S.delay_subtitle_buf, stop_name_or_delay, TITLE_BUFFER_MAX_LEN);
            break;
        case MessageTypeBusDetailStop: {
            struct stop_s *stop = &S.stops[index];
            strncpy(stop->name, stop_name_or_delay, TITLE_BUFFER_MAX_LEN);
            strncpy(stop->time, stop_time, TITLE_BUFFER_MAX_LEN);
            break;
        }
        default:
            break;
    }

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));
}

void push_bus_detail_window(char trip_id[], char vehicle_id[], char title[], char subtitle[]) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = bus_detail_window_load,
        .unload = bus_detail_window_unload
    });

    strncpy(S.trip_id, trip_id, ID_BUFFER_MAX_LEN);
    strncpy(S.vehicle_id, vehicle_id, ID_BUFFER_MAX_LEN);
    strncpy(S.bus_title, title, TITLE_BUFFER_MAX_LEN);
    strncpy(S.bus_subtitle, subtitle, TITLE_BUFFER_MAX_LEN);

    app_message_register_inbox_received(bus_detail_app_message_received);

    tick_timer_service_subscribe(SECOND_UNIT, bus_detail_timer);

    window_stack_push(S.window, true);
}
