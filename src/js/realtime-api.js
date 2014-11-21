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
    request.open("GET", "http://realtimemap.grt.ca/Stop/GetBusInfo?VehicleId=" + vehicleId + "&TripId=" + tripId);
    request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
    request.onload = function () {
        if (request.status == 200) {
            var stops = JSON.parse(request.responseText)["stopTimes"];
            var nextStop = stops[0];
            var delayTotalSeconds = nextStop["Delay"] * -2;
            var delayString = makeDelayString(delayTotalSeconds);
            var stopsForPebble = stops.map(function (stop) {
                var minutes = stop["Minutes"];
                var minString;
                if (minutes < 0) {
                    minString = "Late"
                } else if (minutes == 0) {
                    minString = "Due"
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