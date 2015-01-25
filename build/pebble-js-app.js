function degToRad(deg) {
    return deg * 0.0174532925;
}

// Source: http://stackoverflow.com/a/20642344
function geoDistance(lat1, lon1, lat2, lon2) {
    // returns the distance in km between the pair of latitude and longitudes provided in decimal degrees
    var R = 6371; // km
    var dLat = degToRad(lat2 - lat1);
    var dLon = degToRad(lon2 - lon1);
    var a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
        Math.cos(degToRad(lat1)) * Math.cos(degToRad(lat2)) *
        Math.sin(dLon / 2) * Math.sin(dLon / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var d = R * c;
    return d;
}

function extendWithArray(obj, array, startIndex) {
    for (var i = 0; i < array.length; i++) {
        var key = "" + (i + startIndex);
        var value = array[i];
        obj[key] = value;
    }
}
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
    console.log("Getting bus detail for vehicleId=" + vehicleId + " tripId=" + tripId);

    function busInfoCallback(info) {
        var msg = {};
        msg["PGKeyMessageType"] = 1;
        msg["PGKeyBusDetailDelay"] = info.delay;
        extendWithArray(msg, info.stops, 2);

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
var GRT = {};

GRT.Bus = function (info) {
    this.description = info["Trip"]["Headsign"];
    this.tripId = info["TripId"];
    this.vehicleId = info["VehicleId"];
};

GRT.filterCloseBuses = function (myLoc, allBuses, limit) {
    var closeBuses = [];
    for (busIndex in allBuses) {
        var busInfo = allBuses[busIndex];
        var lat = busInfo["Latitude"];
        var lon = busInfo["Longitude"];
        var distance = geoDistance(lat, lon, myLoc.lat, myLoc.lon);
        if (distance < DEFAULT_BUS_DISTANCE_LIMIT) {
            var bus = new GRT.Bus(busInfo);
            bus.distance = Math.round(distance * 10) / 10;
            closeBuses.push(bus);
        }
    }
    closeBuses.sort(function (b1, b2) {
        if (b1.distance < b2.distance) {
            return -1;
        } else if (b1.distance > b2.distance) {
            return 1;
        }
        return 0;
    });
    closeBuses = closeBuses.slice(0, limit);
    return closeBuses;
};

GRT.getClosestBuses = function (myLoc, limit, callback) {
    var request = new XMLHttpRequest();
    request.open("GET", "http://realtimemap.grt.ca/Map/GetVehicles");
    request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
    request.onload = function () {
        if (request.status == 200) {
            var buses = JSON.parse(request.responseText);
            console.log("Parsing " + buses.length + " buses");
            var closeBuses = GRT.filterCloseBuses(myLoc, buses, limit);
            callback(closeBuses);
        } else {
            console.log("Error with request: " + request.statusText);
        }
    };
    console.log("Sending request to http://realtimemap.grt.ca/Map/GetVehicles");
    request.send();
};

function makeDelayString(delayTotalSeconds) {
    var delayMinutes = Math.floor(delayTotalSeconds / 60);
    var delaySeconds = Math.round(((delayTotalSeconds / 60) - delayMinutes) * 60);
    var delayString = "";
    if (delayMinutes.length != 0) {
        delayString += delayMinutes + "m "
    }
    delayString += delaySeconds + "s";
    return delayString;
}

GRT.getBusInfo = function (myLoc, vehicleId, tripId, callback) {
    var request = new XMLHttpRequest();
    var url = "http://realtimemap.grt.ca/Stop/GetBusInfo?" +
        "VehicleId=" + encodeURIComponent(vehicleId) +
        "&TripId=" + encodeURIComponent(tripId);
    console.log("--> "+url);
    request.open("GET", url);
    request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
    request.onload = function () {
        if (request.status == 200) {
            var stops = JSON.parse(request.responseText)["stopTimes"];
            var nextStop = stops[0];
            var delayTotalSeconds = nextStop["Delay"] * -2;
            var delayString = makeDelayString(delayTotalSeconds);
            var stopsForPebble = stops.map(function (stop) {
                var minutes = stop["Minutes"] + Math.floor(delayTotalSeconds / 60);
                var minString = "N/A";
                if (minutes == 0) {
                    minString = "< 1 minute"
                } else if (minutes == 1) {
                    minString = "1 minute"
                } else {
                    minString = minutes + " minutes"
                }
                return minString + ";" + stop["Name"];
            });
            callback({
                delay: delayString,
                stops: stopsForPebble
            });
        } else {
            console.log("Error with request: " + request.statusText);
        }
    };
    request.send();
};