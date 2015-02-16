#include <pebble.h>
#include "nearby_buses.h"
#include "message_types.h"
#include "bus_detail.h"

#define NUM_BUSES 10

#define TITLE_BUFFER_MAX_LEN 128

#define ID_BUFFER_MAX_LEN 40

#define LOADING_STRING "Loading.."

struct bus_s {
    bool populated;
    char description[TITLE_BUFFER_MAX_LEN];
    char distance[TITLE_BUFFER_MAX_LEN];
    char trip_id[ID_BUFFER_MAX_LEN];
    char vehicle_id[ID_BUFFER_MAX_LEN];
};

static struct {
    Window *window;
    SimpleMenuLayer *menu_layer;
    SimpleMenuSection nearby_buses_menu_section;
    struct bus_s nearby_buses[NUM_BUSES];
    SimpleMenuItem nearby_buses_items[NUM_BUSES];
} S;

void initialize_buses();

static void send_phone_message_nearby_buses() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, PGKeyMessageType, (uint8_t) MessageTypeNearbyBuses);

    app_message_outbox_send();
}

void nearby_buses_app_message_received(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "bus received");

    int32_t index = 0;
    char *name = NULL;
    char *distance = NULL;
    char *trip_id = NULL;
    char *vehicle_id = NULL;

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
            case PGKeyBusTripId:
                trip_id = t->value->cstring;
                break;
            case PGKeyBusVehicleId:
                vehicle_id = t->value->cstring;
            default:
                break;

        }
        t = dict_read_next(iterator);
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "bus name: %s", name);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bus distance: %s", distance);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bus trip_id: %s", trip_id);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "bus vehicle_id: %s", vehicle_id);

    struct bus_s *bus = &S.nearby_buses[index];
    strncpy(bus->distance, distance, TITLE_BUFFER_MAX_LEN);
    strncpy(bus->description, name, TITLE_BUFFER_MAX_LEN);
    strncpy(bus->vehicle_id, vehicle_id, ID_BUFFER_MAX_LEN);
    strncpy(bus->trip_id, trip_id, ID_BUFFER_MAX_LEN);

    bus->populated = true;

    layer_mark_dirty(simple_menu_layer_get_layer(S.menu_layer));
}

static void nearby_buses_window_appear(Window *window) {
    app_message_register_inbox_received(nearby_buses_app_message_received);
    send_phone_message_nearby_buses();
}

static void nearby_buses_window_disappear(Window *window) {
    app_message_deregister_callbacks();
}

static void nearby_bus_selected_callback(int index, void *context) {
    struct bus_s *bus = &S.nearby_buses[index];
    if (!bus->populated) {
        return;
    }
    char buf[TITLE_BUFFER_MAX_LEN];
    strncpy(buf, bus->description, TITLE_BUFFER_MAX_LEN);
    char *subtitle = buf;
    while (*subtitle != ' ' && *subtitle != '\0') {
        subtitle++;
    }
    *subtitle = '\0';
    subtitle++;
    char *title = buf;
    push_bus_detail_window(bus->trip_id, bus->vehicle_id, title, subtitle);
}

static void nearby_buses_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    initialize_buses();

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

void initialize_buses() {
    for (int i = 0; i < NUM_BUSES; ++i) {
        struct bus_s *bus = &S.nearby_buses[i];
        bus->populated = false;
        char *title = bus->distance;
        char *subtitle = bus->description;
        subtitle[0] = '\0';
        strncpy(title, LOADING_STRING, strlen(LOADING_STRING));
        S.nearby_buses_items[i] = (SimpleMenuItem) {
            .title = title,
            .subtitle = subtitle,
            .callback = nearby_bus_selected_callback
        };
    }
}

static void nearby_buses_window_unload(Window *window) {
    simple_menu_layer_destroy(S.menu_layer);
}

void push_nearby_buses_window(int index, void *context) {
    S.window = window_create();

    window_set_window_handlers(S.window, (WindowHandlers) {
        .load = nearby_buses_window_load,
        .unload = nearby_buses_window_unload,
        .appear = nearby_buses_window_appear,
        .disappear = nearby_buses_window_disappear
    });

    window_stack_push(S.window, true);
}

