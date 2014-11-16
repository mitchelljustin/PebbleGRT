function jsonRequest(url, params, method, callback) {
  params = params || {};
  url += "?";
  for (key in params) {
    url += key + "=" + params[key];
  }
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(JSON.parse(this.responseText));
  };
  xhr.open(method, url);
  xhr.send();
}

function locationSuccess(pos) {
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getBuses() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the app is opened
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
