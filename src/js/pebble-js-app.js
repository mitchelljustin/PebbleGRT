// type location: {
//    lat: string,
//    lon: string,
// }

var DISTANCE_LIMIT = 20; // km

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
    if (distance < DISTANCE_LIMIT) {
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

Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");

    getBuses();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getBuses();
  }
);
