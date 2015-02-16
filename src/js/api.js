/**
 * Created by mitch on 15-01-24.
 */


var stopsMaxDistance = 20; // Km
var busesMaxDistance = 20; // Km

var resultsCountLimit = 7;

var GRT = {};

GRT.Bus = function (info) {
    this.description = info["Trip"]["Headsign"] || "No description";
    this.tripId = info["TripId"];
    this.vehicleId = info["VehicleId"];
    this.distance = -1;
};

GRT.findNearbyStops = function (loc) {
    var nearbyStops = [];
    for (var stopIndex in stops) {
        var stop = stops[stopIndex];
        var distance = geoDistance(loc.lat, loc.lon, stop.lat, stop.lon);
        if (distance < stopsMaxDistance) {
            stop.distance = Math.round(distance * 10) / 10;
            nearbyStops.push(stop);
        }
    }
    return sortByDistance(nearbyStops).slice(0, resultsCountLimit);
};

GRT.findNearbyBuses = function (loc, callback) {
    function filterCloseBuses(loc, buses) {
        var closeBuses = [];
        for (var busIndex in buses) {
            var busInfo = buses[busIndex];
            var lat = busInfo["Latitude"];
            var lon = busInfo["Longitude"];
            var distance = geoDistance(lat, lon, loc.lat, loc.lon);
            if (distance < busesMaxDistance) {
                var bus = new GRT.Bus(busInfo);
                bus.distance = Math.round(distance * 10) / 10;
                closeBuses.push(bus);
            }
        }
        return sortByDistance(closeBuses).slice(0, resultsCountLimit);
    }

    var request = new XMLHttpRequest();
    var url = "http://realtimemap.grt.ca/Map/GetVehicles";
    console.log("GET " + url);
    request.open("GET", url);
    request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
    request.onload = function () {
        if (request.status == 200) {
            var buses = JSON.parse(request.responseText);
            var closeBuses = filterCloseBuses(loc, buses);
            callback(closeBuses);
        } else {
            console.log("Error with request: " + request.statusText);
        }
    };
    console.log("Sending request to http://realtimemap.grt.ca/Map/GetVehicles");
    request.send();
};

function makeMinutesString(minutes) {
    var minString;
    if (minutes == 0) {
        minString = "< 1 minute"
    } else if (minutes == 1) {
        minString = "1 minute"
    } else if (minutes < 0) {
        minString = "past"
    } else {
        minString = minutes + " minutes"
    }
    return minString;
}

GRT.getBusInfo = function (loc, vehicleId, tripId, callback) {

    var request = new XMLHttpRequest();
    var url = "http://realtimemap.grt.ca/Stop/GetBusInfo?" +
        "VehicleId=" + encodeURIComponent(vehicleId) +
        "&TripId=" + encodeURIComponent(tripId);
    console.log("GET " + url);
    request.open("GET", url);
    request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
    request.onload = function () {
        if (request.status == 200) {
            var json = JSON.parse(request.responseText);
            var stops = json["stopTimes"];
            var nextStop = stops[0];
            var minutes = nextStop["Minutes"];
            var delayString;
            if (minutes < 0) {
                var delay = (-minutes);
                delayString = "" + delay + " minute" + ((delay == 1) ? "" : "s");
            } else {
                delayString = "None"
            }

            var index = 0;
            var stopsForPebble = stops.map(function (stop) {
                var minutes = stop["Minutes"];
                var stopName = stop["Name"];
                return {
                    "PGKeyBusDetailStopName": stopName,
                    "PGKeyBusDetailStopTime": makeMinutesString(minutes),
                    "PGKeyBusDetailIndex": index++
                };
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

GRT.getStopInfo = function(stopId, routeId, callback) {
    var request = new XMLHttpRequest();
    var url = "http://realtimemap.grt.ca/Stop/GetStopInfo?" +
        "stopId=" + encodeURIComponent(stopId) +
        "&routeId=" + encodeURIComponent(routeId);
    console.log("GET " + url);
    request.open("GET", url);
    request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
    request.onload = function () {
        if (request.status == 200) {
            var json = JSON.parse(request.responseText);
            var buses = json["stopTimes"];
            var index = 0;

            if (buses.length != 0) {
                buses = buses.map(function (bus) {
                    var minutes = bus["Minutes"];
                    var stopName = bus["HeadSign"];
                    return {
                        "PGKeyStopInfoStopName": stopName,
                        "PGKeyStopInfoStopTime": makeMinutesString(minutes),
                        "PGKeyStopInfoIndex": index++
                    };
                });
            } else {
                buses = [null, null, null].map(function(n) {
                    return {
                        "PGKeyStopInfoStopName": "N/A",
                        "PGKeyStopInfoStopTime": "N/A",
                        "PGKeyStopInfoIndex": index++
                    };
                });
            }
            callback(buses);
        } else {
            console.log("Error with request: " + request.statusText);
        }
    };
    request.send();

};