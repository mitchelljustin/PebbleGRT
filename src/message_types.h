#define PGKeyMessageType 0


#define PGKeyBusDetailDelay 1
#define PGKeyVehicleId 1
#define PGKeyTripId 2

typedef enum
{
    PGMessageTypeReportNearbyBuses,
    PGMessageTypeBusDetail
} PGMessageType;
