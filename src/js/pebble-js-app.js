var seconds = window.localStorage.getItem('seconds') ? window.localStorage.getItem('seconds') : 1;

Pebble.addEventListener('ready',
  function(e) {
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show configuration page
  Pebble.openURL('http://www.bytesizeadventures.com/equaliser-config.html?seconds=' + seconds);
});

Pebble.addEventListener('webviewclosed', function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    seconds = configuration.seconds;
    window.localStorage.setItem('seconds', seconds);
    Pebble.sendAppMessage({seconds: parseInt(seconds, 10)});
    //console.log(seconds);
  }
);