var table = document.getElementsByTagName('table')[0];
var networkInfo = document.getElementById('networksFound');
var scanBtn = document.getElementById('apScanStart');
var scanInfo = document.getElementById('scanInfo');
var apMAC = document.getElementById('apMAC');
var startStopScan = document.getElementById('startStopScan');
var autoScan = false;
var canScan = true;

function toggleBtn(onoff) {
  if (onoff && !autoScan) {
    scanInfo.style.visibility = 'hidden';
    scanBtn.style.visibility = 'visible';
  } else {
    scanInfo.style.visibility = 'visible';
    scanBtn.style.visibility = 'hidden';
  }
}

function compare(a, b) {
  if (a.r > b.r) return -1;
  if (a.r < b.r) return 1;
  return 0;
}

function getEncryption(num) {
  if (num == 8) return "WPA*";
  else if (num == 4) return "WPA2";
  else if (num == 2) return "WPA";
  else if (num == 7) return "none";
  else if (num == 5) return "WEP";
}

function getResults() {
  toggleBtn(true);
  getResponse("APScanResults.json", function(responseText) {
    var res = JSON.parse(responseText);
    res.aps = res.aps.sort(compare);
    networkInfo.innerHTML = "Networks found: " + res.aps.length;
    apMAC.innerHTML = "";

    var tr = '';
    if (res.aps.length > 0) tr += '<tr><th>Ch</th><th>SSID</th><th>RSSI</th><th>Encrypt.</th><th>Select</th></tr>';

    for (var i = 0; i < res.aps.length; i++) {

      if (res.aps[i].se == 1) tr += '<tr class="selected">';
      else tr += '<tr>';
      tr += '<td>' + res.aps[i].c + '</td>';
      tr += '<td>' + res.aps[i].ss + '</td>';
      tr += '<td>' + res.aps[i].r + ' <meter value="' + res.aps[i].r + '" max="-30" min="-100" low="-80" high="-60" optimum="-50"></meter></td>';
      tr += '<td>' + getEncryption(res.aps[i].e) + '</td>';

      if (res.aps[i].se) {
        tr += '<td><button class="marginNull selectedBtn" onclick="select(' + res.aps[i].i + ')">deselect</button></td>';
        apMAC.innerHTML = res.aps[i].m;
      }
      else tr += '<td><button class="marginNull" onclick="select(' + res.aps[i].i + ')">select</button></td>';
      tr += '</tr>';
    }
    table.innerHTML = tr;
    canScan = true;
  });
}

function scan() {
  canScan = false;
  toggleBtn(false);
  getResponse("APScan.json", function(responseText) {
    if (responseText == "true") getResults();
    else alert("error");
    toggleBtn(true);
  });
}

function startConScan() {
  if (autoScan) {
    autoScan = false;
    startStopScan.innerHTML = "start";
    toggleBtn(true);
  } else {
    autoScan = true;
    startStopScan.innerHTML = "stop";
    toggleBtn(false);
  }
}

function select(num) {
  getResponse("APSelect.json?num=" + num, function(responseText) {
    if (responseText == "true") getResults();
    else alert("error");
  });
}

getResults();

setInterval(function() {
  if (autoScan && canScan) scan();
}, 1000);