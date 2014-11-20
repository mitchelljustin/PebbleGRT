#include <pebble.h>
#include "pgbus.h"
#include "util.h"

struct PGBus *pgbus_create()
{
    struct PGBus *bus = malloc(sizeof(struct PGBus));
    return bus;
}

void pgbus_destroy(struct PGBus *bus)
{
    free(bus);
}

struct PGBus *pgbus_parse_from_string(const char *source)
{
    char source_cpy[256];
    strcpy(source_cpy, source);
    APP_LOG(APP_LOG_LEVEL_INFO, "splitting '%s'", source_cpy);
    const char *parts[4];
    split_semi_delimited(source_cpy, parts);
    struct PGBus *bus = pgbus_create();
    for (int i = 0; i < 4; ++i) {
        APP_LOG(APP_LOG_LEVEL_INFO, "part %d: '%s'", i, parts[i]);
    }
    strncpy(bus->distance, parts[0], PGBusDistanceMaxStringLength);
    strncpy(bus->description, parts[1], PGBusDescriptionMaxStringLength);
    strncpy(bus->tripId, parts[2], PGBusTripIdMaxStringLength);
    strncpy(bus->vehicleId, parts[3], PGBusVehicleIdMaxStringLength);
    return bus;
}
