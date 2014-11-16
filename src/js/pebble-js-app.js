// type location: {
//    lat: string,
//    lon: string,
// }

var DEFAULT_BUS_DISTANCE_LIMIT = 20; // km
var MAX_NUM_CLOSE_BUSES = 5;

var TYPE_KEY = '0';
var TYPE_VALUE_CLOSE_BUSES = 0;

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
    console.log(JSON.stringify({
      lat1: lat1,
      lon1: lon1,
      lat2: lat2,
      lon2: lon2,
      R: R,
      dLat: dLat,
      dLon: dLon,
      a: a,
      c: c,
      d: d
    }));
    return d;
}

function filterCloseBuses(myLoc, allBuses, limit) {
  var closeBuses = [];
  for (busIndex in allBuses) {
    var busInfo = allBuses[busIndex];
    var lat = busInfo["Latitude"];
    var lon = busInfo["Longitude"];
    var description = busInfo["Trip"]["Headsign"];
    var distance = geoDistance(lat, lon, myLoc.lat, myLoc.lon);
    console.log("'"+description+"': "+distance+" km");
    if (distance < DEFAULT_BUS_DISTANCE_LIMIT) {
      closeBuses.push({
        "description": description,
        "distance": Math.round(distance * 10) / 10
      });
    }
  }
  closeBuses.sort(function(b1, b2) {
    if (b1.distance < b2.distance) {
      return -1;
    } else if (b1.distance > b2.distance) {
      return 1;
    }
    return 0;
  });
  closeBuses = closeBuses.slice(0, limit);
  return closeBuses;
}

function getClosestBuses(myLoc, limit, callback) {
  var request = new XMLHttpRequest();
  request.open("GET", "http://realtimemap.grt.ca/Map/GetVehicles");
  request.setRequestHeader("Referer", "http://realtimemap.grt.ca/Map");
  request.onload = function(response) {
    if (request.status == 200) {
      console.log("Request success: \n"+response);
      var buses = JSON.parse(request.responseText);
      var closeBuses = filterCloseBuses(myLoc, buses, limit);
      console.log("Parsed close buses: \n" + JSON.stringify(closeBuses));
      callback(closeBuses);
    } else {
      console.log("Error with request: " + request.statusText);
    }
  };
  console.log("Sending request to http://realtimemap.grt.ca/Map/GetVehicles");
  request.send();
}

function getGeoLocation(onSuccess) {
  var locationOptions = {
    enableHighAccuracy: true,
    maximumAge: 10000,
    timeout: 10000
  };
  navigator.geolocation.getCurrentPosition(function(pos) {
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

function extendWithArray(obj, array, startIndex) {
  for (var i = 0; i < array.length; i++) {
    var key = "" + (i + startIndex);
    var value = array[i];
    console.log("Setting '"+key+"' to '"+JSON.stringify(value)+"'");
    obj[key] = value;
  }
}

function reportClosestBuses() {
  function closeBusesCallback(buses) {
    var busStrings = buses.map(
      function (bus) {
        return bus.distance + "km;" + bus.description;
      }
    );

    var msg = {
      TYPE_KEY: TYPE_VALUE_CLOSE_BUSES
    };
    extendWithArray(msg, busStrings, 1);

    Pebble.sendAppMessage(msg,
      function () {
        console.log("Message sent: \n" + JSON.stringify(msg));
      },
      function () {
        console.log("ERROR: could not send message: \n" + JSON.stringify(msg));
      });
  }

  getGeoLocation(function(myLoc) {
    getClosestBuses(myLoc, MAX_NUM_CLOSE_BUSES, closeBusesCallback);
  });
}

Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");
    reportClosestBuses();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    reportClosestBuses();
  }
);
