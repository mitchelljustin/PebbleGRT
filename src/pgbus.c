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
    static const int BUF_SIZE = 1024;
    char source_cpy[BUF_SIZE];
    strncpy(source_cpy, source, BUF_SIZE);
    const char *parts[4];
    split_semi_delimited(source_cpy, parts);
    struct PGBus *bus = pgbus_create();
    strncpy(bus->distance,     parts[0], PGBusDistanceMaxStringLength);
    strncpy(bus->description,  parts[1], PGBusDescriptionMaxStringLength);
    strncpy(bus->tripId,       parts[2], PGBusTripIdMaxStringLength);
    strncpy(bus->vehicleId,    parts[3], PGBusVehicleIdMaxStringLength);
    return bus;
}
