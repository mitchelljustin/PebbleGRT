#include <pebble.h>

static Window *s_main_window;

static SimpleMenuItem s_main_menu_items[1];

static SimpleMenuSection s_main_menu_section = {
    .items = s_main_menu_items,
    .num_items = 1,
    .title = "Actions"
};

static SimpleMenuLayer *s_main_menu_layer;

static void main_window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    s_main_menu_layer = simple_menu_layer_create(window_bounds,
        window,
        &s_main_menu_section,
        1,
        NULL);

    layer_add_child(window_layer, simple_menu_layer_get_layer(s_main_menu_layer));
}

static void main_window_unload(Window *window)
{
    simple_menu_layer_destroy(s_main_menu_layer);
}

static void setup_main_menu_items()
{
    s_main_menu_items[0] = (SimpleMenuItem) {
        .title = "Nearby buses"
    };
}

static void init()
{
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    setup_main_menu_items();

    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    window_stack_push(s_main_window, true);
}

static void deinit()
{
    window_destroy(s_main_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
    return 0;
}
