// type location: {
//    lat: string,
//    lon: string,
// }

var DEFAULT_BUS_DISTANCE_LIMIT = 20; // km
var MAX_NUM_CLOSE_BUSES = 6;

var PGTypeReportCloseBuses = 0;
var PGTypeBusDetail = 1;

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

function reportClosestBuses() {
    function closeBusesCallback(buses) {
        var busStrings = buses.map(encodeBus);

        console.log("Sending to Pebble:");
        for (index in busStrings) {
            console.log("[" + index + "] \"" + busStrings[index] + "\"");
        }

        var msg = {
            "PGKeyMessageType": PGTypeReportCloseBuses
        };
        extendWithArray(msg, busStrings, 1);

        console.log("Sending message: " + JSON.stringify(msg));

        Pebble.sendAppMessage(msg,
            function () {
                console.log("Message sent.");
            },
            function () {
                console.log("ERROR: could not send message: \n" + JSON.stringify(msg));
            });
    }

    getGeoLocation(function (myLoc) {
        GRT.getClosestBuses(myLoc, MAX_NUM_CLOSE_BUSES, closeBusesCallback);
    });
}

function busDetail(vehicleId, tripId) {
    console.log("Getting bus detail for vehicleId="+vehicleId+" tripId="+tripId);

    function busInfoCallback(info) {
        var msg = {};
        msg["PGKeyMessageType"] = 1;
        msg["PGKeyBusDetailDelay"] = info.delay;

        console.log("Sending message: "+JSON.stringify(msg));

        Pebble.sendAppMessage(msg,
            function () {
                console.log("Message sent.");
            },
            function () {
                console.log("ERROR: could not send message: \n" + JSON.stringify(msg));
            });
    }

    getGeoLocation(function (myLoc) {
        GRT.getBusInfo(myLoc, vehicleId, tripId, busInfoCallback);
    });
}

Pebble.addEventListener('appmessage',
    function (e) {
        var data = e.payload;
        console.log("AppMessage received: " + JSON.stringify(data));
        switch (data["PGKeyMessageType"]) {
            case PGTypeReportCloseBuses:
                reportClosestBuses();
                break;
            case PGTypeBusDetail:
                busDetail(data["PGKeyVehicleId"], data["PGKeyTripId"]);
                break;
        }
    }
);
