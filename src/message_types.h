#define PGKeyMessageType 0

#define PGKeyBusName 1
#define PGKeyBusDistance 2
#define PGKeyBusIndex 3

#define PGKeyStopName 1
#define PGKeyStopDistance 2
#define PGKeyStopIndex 3

#define PGKeyBusDetailVehicleId 1
#define PGKeyBusDetailTripId 2
#define PGKeyBusDetailDelay 1
#define PGKeyBusDetailStopName 1
#define PGKeyBusDetailStopTime 2

enum MessageType {
    MessageTypeNearbyBuses = 0,
    MessageTypeBusDetail = 1,
    MessageTypeNearbyStops = 2,
    MessageTypeBusDetailDelay = 3,
    MessageTypeBusDetailStop = 4
};