// type location: {
//    lat: string,
//    lon: string,
// }

var DEFAULT_BUS_DISTANCE_LIMIT = 20; // km
var MAX_NUM_CLOSE_BUSES = 5;

var TYPE_KEY = '0';
var TYPE_VALUE_CLOSE_BUSES = 0;

// Source: http://stackoverflow.com/a/20642344
function geoDistance(lat1, lon1, lat2, lon2) {
    // returns the distance in km between the pair of latitude and longitudes provided in decimal degrees
    var R = 6371; // km
    var dLat = (lat2 - lat1).toRad();
    var dLon = (lon2 - lon1).toRad();
    var a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
                    Math.cos(lat1.toRad()) * Math.cos(lat2.toRad()) *
                    Math.sin(dLon / 2) * Math.sin(dLon / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var d = R * c;
    return d;
}

function filterCloseBuses(allBuses, limit) {
  var closeBuses = [];
  for (busInfo in allBuses) {
    if (closeBuses.length == limit) {
      return closeBuses;
    }
    var lat = busInfo["Latitude"];
    var lon = busInfo["Longitude"];
    var description = busInfo["Trip"]["Headsign"];
    var distance = geoDistance(lat, lon, myLoc.lat, myLoc.long);
    if (distance < DEFAULT_BUS_DISTANCE_LIMIT) {
      closeBuses.push({
        "description": description,
        "distance": distance
      });
    }
  }
  return closeBuses;
}

function getClosestBuses(myLoc, limit, callback) {
  var request = new XMLHttpRequest();
  request.open("http://realtimemap.grt.ca/Map/GetVehicles", "GET");
  request.onload = function(response) {
    if (request.readystate == 4 && request.status == 200) {
      var buses = JSON.parse(response);
      var closeBuses = filterCloseBuses(buses, limit);
      callback(closeBuses);
    }
  }
  request.send();
}

function getGeoLocation(onSuccess) {
  var locationOptions = {
    enableHighAccuracy: true,
    maximumAge: 10000,
    timeout: 10000
  };
  navigator.geolocation.getCurrentPosition(function(pos) {
    let coords = pos.coords;
    onSuccess({
      lat: coords.latitude,
      lon: coords.longitude
    });
  }, locationError, locationOptions);
}

function locationError(err) {
  console.log('location error (' + err.code + '): ' + err.message);
}

function extendWithArray(obj, array) {
  var firstFreeIndex = 0;
  while (obj.hasOwnProperty("" + firstFreeIndex)) {
    firstFreeIndex += 1;
  }
  for (var i = 0; i < array.length; i++) {
    obj["" + (i + firstFreeIndex)] = array[i];
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
    extendWithArray(msg, busStrings);

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
