#include <pebble.h>
#include "pgbus.h"

struct PGBus *pgbus_create() {
  struct PGBus *bus = malloc(sizeof(struct PGBus));
  return bus;
}
void pgbus_destroy(struct PGBus *bus) {
  free(bus);
}

struct PGBus *pgbus_parse_from_string(const char *source) {
  char source_cpy[256];
  int source_idx = 0;
  strcpy(source_cpy, source);
  const char *parts[4];
  parts[0] = source_cpy;
  for (int part_index = 1; part_index < 4; part_index++) {
    while (source_cpy[source_idx] != ';') {
      source_idx += 1;
    }
    source_cpy[source_idx] = '\0';
    source_idx += 1;
    parts[part_index] = &source_cpy[source_idx];
  }
  struct PGBus *bus = pgbus_create();
  strncpy(bus->distance,    parts[0],   PGBusDistanceMaxStringLength);
  strncpy(bus->description, parts[1],   PGBusDescriptionMaxStringLength);
  strncpy(bus->vehicleId,   parts[2],   PGBusVehicleIdMaxStringLength);
  strncpy(bus->tripId,      parts[3],   PGBusTripIdMaxStringLength);
  return bus;
}
