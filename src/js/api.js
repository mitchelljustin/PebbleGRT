/**
 * Created by mitch on 15-01-24.
 */


var stopsMaxDistance = 20; // Km
var busesMaxDistance = 20; // Km

var resultsCountLimit = 6;

var GRT = {};

GRT.Bus = function (info) {
    this.description = info["Trip"]["Headsign"];
    this.tripId = info["TripId"];
    this.vehicleId = info["VehicleId"];
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

GRT.getBusInfo = function (loc, vehicleId, tripId, callback) {
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

    function makeMinutesString(minutes) {
        var minString = "N/A";
        if (minutes == 0) {
            minString = "< 1 minute"
        } else if (minutes == 1) {
            minString = "1 minute"
        } else {
            minString = minutes + " minutes"
        }
        return minString;
    }

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
            var delayTotalSeconds = nextStop["Delay"] * -2;
            var delayString = makeDelayString(delayTotalSeconds);
            var stopsForPebble = stops.map(function (stop) {
                var minutes = stop["Minutes"] + Math.floor(delayTotalSeconds / 60);
                var stopName = stop["Name"];
                return makeMinutesString(minutes) + ";" + stopName;
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