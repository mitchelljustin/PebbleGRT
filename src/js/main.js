/**
 * Created by mitch on 15-02-07.
 */

var MessageTypeNearbyBuses = 0;
var MessageTypeBusDetail = 1;
var MessageTypeNearbyStops = 2;
var MessageTypeBusDetailDelay = 3;
var MessageTypeBusDetailStop = 4;
var MessageTypeStopInfo = 5;

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

function sendPiecewiseMessages(messages) {
    console.log("Sending "+ messages.length + " messages to Pebble");
    var index = 0;
    var json = null;
    function pebbleAckCallback() {
        if (index == messages.length) {
            return;
        }
        json = messages[index];
        index += 1;
        sendJsonToPebble(json, pebbleAckCallback);
    }
    pebbleAckCallback();
}

function makeDistanceString(distance) {
    if (distance >= 1.0) {
        return distance + "km"
    }
    return distance * 1000 + "m"
}

function sendJsonToPebble(json, callback) {
    var string = JSON.stringify(json);
    console.log("Sending message to Pebble: " + string);

    Pebble.sendAppMessage(json,
        function () {
            console.log("Message sent.");
            if (callback) {
                callback();
            }
        },
        function () {
            console.log("ERROR: could not send message: \n" + string);
        });
}

function nearbyBuses() {
    function closeBusesCallback(buses) {
        console.log("Nearby buses: \n"+ JSON.stringify(buses));
        var messages = [];
        for (var index = 0; index < buses.length; index++) {
            var bus = buses[index];
            var name = bus.description;
            var distance = makeDistanceString(bus.distance);
            var msg = {
                "PGKeyMessageType": MessageTypeNearbyBuses,
                "PGKeyBusName": name,
                "PGKeyBusDistance": distance,
                "PGKeyBusIndex": index,
                "PGKeyBusTripId": bus.tripId,
                "PGKeyBusVehicleId": bus.vehicleId
            };
            messages.push(msg);
        }
        sendPiecewiseMessages(messages);
    }

    getGeoLocation(function (loc) {
        GRT.findNearbyBuses(loc, closeBusesCallback);
    });
}

function busDetail(vehicleId, tripId) {
    console.log("Getting bus detail for vehicleId=" + vehicleId + " tripId=" + tripId);

    function busInfoCallback(info) {
        var messages = [
        {
            "PGKeyMessageType": MessageTypeBusDetailDelay,
            "PGKeyBusDetailDelay": info.delay
        }];
        Array.prototype.push.apply(messages, info.stops.map(function (stop) {
            stop["PGKeyMessageType"] = MessageTypeBusDetailStop;
            return stop;
        }));

        sendPiecewiseMessages(messages);
    }

    getGeoLocation(function (loc) {
        GRT.getBusInfo(loc, vehicleId, tripId, busInfoCallback);
    });
}

function nearbyStops() {
    getGeoLocation(function (loc) {
        var messages = [];
        var stops = GRT.findNearbyStops(loc);
        for (var index = 0; index < stops.length; index++) {
            var stop = stops[index];
            var description = stop.name;
            var distance = makeDistanceString(stop.distance);
            var msg = {
                "PGKeyMessageType": MessageTypeNearbyStops,
                "PGKeyStopName": description,
                "PGKeyStopDistance": distance,
                "PGKeyStopIndex": index,
                "PGKeyStopId": stop.id
            };
            messages.push(msg);
        }
        sendPiecewiseMessages(messages);
    });
}

function stopInfo(stopId, routeId) {
    GRT.getStopInfo(stopId, routeId, function(stops) {
        stops.forEach(function(stop) {
            stop["PGKeyMessageType"] = MessageTypeStopInfo;
        });
        sendPiecewiseMessages(stops);
    });
}

Pebble.addEventListener('appmessage',
    function (e) {
        var data = e.payload;
        console.log("Received message from Pebble: " + JSON.stringify(data));
        var messageType = data["PGKeyMessageType"];
        switch (messageType) {
            case MessageTypeNearbyBuses:
                nearbyBuses();
                break;
            case MessageTypeBusDetail:
                var vehicleId = data["PGKeyBusVehicleId"];
                var tripId = data["PGKeyBusTripId"];
                busDetail(vehicleId, tripId);
                break;
            case MessageTypeNearbyStops:
                nearbyStops();
                break;
            case MessageTypeStopInfo:
                var stopId = data["PGKeyStopId"];
                var routeId = data["PGKeyRouteId"];
                stopInfo(stopId, routeId);
        }
    }
);
