#include <pebble.h>
#include "message_types.h"
#include "bus_detail.h"
#include "pgbus.h"
#include "util.h"

#define NUM_INFO_MENU_ITEMS 2
#define NUM_STOP_MENU_ITEMS 3

#define NUM_SECTIONS 2
#define STOP_BUFFER_SIZE 100
#define DELAY_BUFFER_SIZE 80
static const int REFRESH_INTERVAL = 27;

static Window *s_window;

static struct PGBus s_bus;
static char *s_bus_bearing = NULL;

static char *s_delay_subtitle_buf = NULL;

static SimpleMenuItem s_info_menu_items[NUM_INFO_MENU_ITEMS];

static SimpleMenuSection s_info_menu_section = {
    .items = s_info_menu_items,
    .num_items = NUM_INFO_MENU_ITEMS,
    .title = "Bus Detail"
};

static SimpleMenuItem s_stop_menu_items[NUM_STOP_MENU_ITEMS];

static SimpleMenuSection s_stop_menu_section = {
    .items = s_stop_menu_items,
    .num_items = NUM_STOP_MENU_ITEMS,
    .title = "Upcoming Stops"
};

static char *s_stop_buffers[NUM_STOP_MENU_ITEMS] = { NULL };

static SimpleMenuSection s_menu_sections[NUM_SECTIONS];

static SimpleMenuLayer *s_menu_layer;

static void send_bus_detail_msg()
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) PGMessageTypeBusDetail);

    dict_write_cstring(iter, PGKeyVehicleId, s_bus.vehicleId);

    dict_write_cstring(iter, PGKeyTripId, s_bus.tripId);

    app_message_outbox_send();
}

static void window_load(Window *window)
{
    s_delay_subtitle_buf = malloc(DELAY_BUFFER_SIZE * sizeof(char));
    for (int i = 0; i < NUM_STOP_MENU_ITEMS; ++i) {
        s_stop_buffers[i] = malloc(STOP_BUFFER_SIZE * sizeof(char));
    }

    s_info_menu_items[0] = (SimpleMenuItem) {
        .title = s_bus.description,
        .subtitle = s_bus_bearing
    };
    s_info_menu_items[1] = (SimpleMenuItem) {
        .title = "Loading..",
        .subtitle = NULL
    };

    s_menu_sections[0] = s_info_menu_section;
    s_menu_sections[1] = s_stop_menu_section;

    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    s_menu_layer = simple_menu_layer_create(window_bounds,
        window,
        s_menu_sections,
        NUM_SECTIONS,
        NULL);
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
    send_bus_detail_msg();
}

static void window_unload(Window *window)
{
    free(s_delay_subtitle_buf);
    SimpleMenuItem empty_item = { NULL, NULL, NULL, NULL };

    for (int i = 0; i < NUM_STOP_MENU_ITEMS; ++i) {
        free(s_stop_buffers[i]);
        s_stop_menu_items[i] = empty_item;
    }
    for (int i = 0; i < NUM_INFO_MENU_ITEMS; ++i) {
        s_info_menu_items[i] = empty_item;
    }

    simple_menu_layer_destroy(s_menu_layer);
    window_destroy(window);
    app_message_deregister_callbacks();
    tick_timer_service_unsubscribe();
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{

    if (tick_time->tm_sec % REFRESH_INTERVAL == 0) {
        send_bus_detail_msg();
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
    Tuple *t = dict_read_first(iterator);
    while (t != NULL) {
        switch (t->key) {
            case PGKeyMessageType:
                break;
            case PGKeyBusDetailDelay: {
                SimpleMenuItem *delay_item = &s_info_menu_items[1];
                if (delay_item->subtitle == NULL) {
                    delay_item->title = "Delay";
                    delay_item->subtitle = s_delay_subtitle_buf;
                }
                strncpy(s_delay_subtitle_buf, t->value->cstring, DELAY_BUFFER_SIZE);
                break;
            }
            default: {
                int index = t->key - 2;
                SimpleMenuItem *item = &s_stop_menu_items[index];
                char *stop_buf = s_stop_buffers[index];
                strncpy(stop_buf, t->value->cstring, STOP_BUFFER_SIZE);
                const char *parts[2];
                split_semi_delimited(stop_buf, parts);
                item->title = parts[0];
                item->subtitle = parts[1];
                break;
            }
        }

        t = dict_read_next(iterator);
    }
    layer_mark_dirty(simple_menu_layer_get_layer(s_menu_layer));
}

static void split_bus_bearing()
{
    s_bus_bearing = s_bus.description;
    while (*s_bus_bearing != ' ') {
        s_bus_bearing++;
    }
    *s_bus_bearing = '\0';
    s_bus_bearing++;
}

Window *create_bus_detail_window(struct PGBus *bus)
{
    s_window = window_create();
    memcpy(&s_bus, bus, sizeof(struct PGBus));
    split_bus_bearing();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    app_message_register_inbox_received(inbox_received_callback);

    return s_window;
}
