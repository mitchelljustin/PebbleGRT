#define PGKeyMessageType 0
#define PGKeyBusDetailDelay 1
#define PGKeyVehicleId 1
#define PGKeyTripId 2
#define PGKeyBusName 1
#define PGKeyBusDistance 2
#define PGKeyBusIndex 3
#define PGKeyStopName 1
#define PGKeyStopDistance 2
#define PGKeyStopIndex 3

enum MessageType {
    MessageTypeNearbyBuses = 0,
    MessageTypeBusDetail = 1,
    MessageTypeNearbyStops = 2
};