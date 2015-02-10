/**
 * Created by mitch on 15-02-07.
 */

var PGTypeNearbyBuses = 0;
var PGTypeBusDetail = 1;
var PGTypeNearbyStops = 2;

function getGeoLocation(onSuccess) {
    var locationOptions = {
        enableHighAccuracy: true,
        maximumAge: 10000,
        timeout: 10000
    };
    navigator.geolocation.getCurrentPosition(function (pos) {
        var coords = pos.coords;
        onSuccess({
            lat: coords.latitude,
            lon: coords.longitude
        });
    }, locationError, locationOptions);
}

function locationError(err) {
    console.log('location error (' + err.code + '): ' + err.message);
}

function encodeBus(bus) {
    return [
        bus.distance + "km",
        bus.description,
        bus.tripId,
        bus.vehicleId
    ].join(";");
}

function sendJsonToPebble(json) {
    var string = JSON.stringify(json);
    console.log("Sending message to Pebble: " + string);

    Pebble.sendAppMessage(json,
        function () {
            console.log("Message sent.");
        },
        function () {
            console.log("ERROR: could not send message: \n" + string);
        });
}

function nearbyBuses() {
    function closeBusesCallback(buses) {
        for (var index in buses) {
            var bus = buses[index];
            var name = bus.description;
            var distance = bus.distance + "km";
            var msg = {
                "PGKeyMessageType": PGTypeNearbyBuses,
                "PGKeyBusName": name,
                "PGKeyBusDistance": distance,
                "PGKeyBusIndex": index
            };
            sendJsonToPebble(msg);
        }
    }

    getGeoLocation(function (loc) {
        GRT.findNearbyBuses(loc, closeBusesCallback);
    });
}

function busDetail(vehicleId, tripId) {
    console.log("Getting bus detail for vehicleId=" + vehicleId + " tripId=" + tripId);

    function busInfoCallback(info) {
        var msg = {
            "PGKeyMessageType": PGTypeBusDetail,
            "PGKeyBusDetailDelay": info.delay
        };
        extendWithArray(msg, info.stops, 2);

        sendJsonToPebble(msg);
    }

    getGeoLocation(function (loc) {
        GRT.getBusInfo(loc, vehicleId, tripId, busInfoCallback);
    });
}

function nearbyStops() {
    getGeoLocation(function (loc) {
        var stops = GRT.findNearbyStops(loc);
        for (var index in stops) {
            var stop = stops[index];
            var name = stop.name;
            var distance = stop.distance + "km";
            var msg = {
                "PGKeyMessageType": PGTypeNearbyStops,
                "PGKeyStopName": name,
                "PGKeyStopDistance": distance,
                "PGKeyStopIndex": index
            };
            sendJsonToPebble(msg);
        }
    })
}

Pebble.addEventListener('appmessage',
    function (e) {
        var data = e.payload;
        console.log("Received message from Pebble: " + JSON.stringify(data));
        var messageType = data["PGKeyMessageType"];
        switch (messageType) {
            case PGTypeNearbyBuses:
                nearbyBuses();
                break;
            case PGTypeBusDetail:
                var vehicleId = data["PGKeyVehicleId"];
                var tripId = data["PGKeyTripId"];
                busDetail(vehicleId, tripId);
                break;
            case PGTypeNearbyStops:
                nearbyStops();
                break;
        }
    }
);
