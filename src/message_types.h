#define PGKeyMessageType 0
#define PGKeyBusName 1
#define PGKeyBusDistance 2
#define PGKeyBusIndex 3
#define PGKeyBusVehicleId 4
#define PGKeyBusTripId 5
#define PGKeyStopName 6
#define PGKeyStopDistance 7
#define PGKeyStopIndex 8
#define PGKeyBusDetailStopName 9
#define PGKeyBusDetailStopTime 10
#define PGKeyBusDetailDelay 11
#define PGKeyBusDetailIndex 12

enum MessageType {
    MessageTypeNone = 9999,
    MessageTypeNearbyBuses = 0,
    MessageTypeBusDetail = 1,
    MessageTypeNearbyStops = 2,
    MessageTypeBusDetailDelay = 3,
    MessageTypeBusDetailStop = 4
};