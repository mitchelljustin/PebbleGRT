#define PGBusDistanceMaxStringLength 10
#define PGBusDescriptionMaxStringLength 60
#define PGBusVehicleIdMaxStringLength 15
#define PGBusTripIdMaxStringLength 15

struct PGBus
{
    char distance[PGBusDistanceMaxStringLength];
    char description[PGBusDescriptionMaxStringLength];
    char vehicleId[PGBusVehicleIdMaxStringLength];
    char tripId[PGBusTripIdMaxStringLength];
};

struct PGBus *pgbus_create();

void pgbus_destroy(struct PGBus *);

struct PGBus *pgbus_parse_from_string(const char *);
