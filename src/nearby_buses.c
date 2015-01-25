#include <pebble.h>

void push_nearby_buses_window(int index, void *context) {
   Window *newWindow = window_create();
   window_stack_push(newWindow, true);
}
