#include <pebble.h>
#include "message_types.h"
#include "bus_detail.h"
#include "pgbus.h"
#include "util.h"

#define NUM_MENU_ITEMS 5
static const int REFRESH_INTERVAL = 27;

static Window *s_window;

static struct PGBus s_bus;
static char *s_bus_direction_desc;

static char s_delay_subtitle[80];

static SimpleMenuItem s_menu_items[NUM_MENU_ITEMS];

static SimpleMenuSection s_default_menu_section = {
    .items = s_menu_items,
    .num_items = NUM_MENU_ITEMS,
    .title = "Bus Detail"
};

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
    s_menu_items[0] = (SimpleMenuItem) {
        .title = s_bus.description,
        .subtitle = s_bus_direction_desc
    };
    s_menu_items[1] = (SimpleMenuItem) {
        .title = "Loading..",
        .subtitle = NULL
    };

    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    s_menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &s_default_menu_section,
        1,
        NULL);
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
    send_bus_detail_msg();
}

static void window_unload(Window *window)
{
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
                APP_LOG(APP_LOG_LEVEL_INFO, "PGKeyMessageType received with value %d", (int) t->value->int32);
                break;
            case PGKeyBusDetailDelay: {
                SimpleMenuItem *delay_item = &s_menu_items[1];
                if (delay_item->subtitle == NULL) {
                    delay_item->title = "Delay";
                    delay_item->subtitle = s_delay_subtitle;
                }
                strcpy(s_delay_subtitle, t->value->cstring);
                break;
            }
            default:break;
        }

        t = dict_read_next(iterator);
    }
    layer_mark_dirty(simple_menu_layer_get_layer(s_menu_layer));
}

static void parse_bus_direction_desc()
{
    s_bus_direction_desc = s_bus.description;
    while (*s_bus_direction_desc != ' ') {
        s_bus_direction_desc++;
    }
    *s_bus_direction_desc = '\0';
    s_bus_direction_desc++;
}

Window *create_bus_detail_window(struct PGBus *bus)
{
    s_window = window_create();
    memcpy(&s_bus, bus, sizeof(struct PGBus));
    parse_bus_direction_desc();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    app_message_register_inbox_received(inbox_received_callback);

    return s_window;
}
