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
