#ifndef __DEFINES_H_
#define __DEFINES_H_

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
#define PGKeyStopId 13
#define PGKeyStopInfoIndex 14
#define PGKeyStopInfoStopName 15
#define PGKeyStopInfoStopTime 16
#define PGKeyRouteId 17

enum MessageType {
    MessageTypeNone = 9999,
    MessageTypeNearbyBuses = 0,
    MessageTypeBusDetail = 1,
    MessageTypeNearbyStops = 2,
    MessageTypeBusDetailDelay = 3,
    MessageTypeBusDetailStop = 4,
    MessageTypeStopInfo = 5
};

typedef int32_t route_id_t;
typedef int32_t stop_id_t;

#endif