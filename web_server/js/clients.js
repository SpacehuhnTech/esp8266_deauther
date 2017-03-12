var table = document.getElementsByTagName('table')[0];
var scanBtn = document.getElementById("startScan");
var scanTime = document.getElementById("scanTime");
var clientsFound = document.getElementById("clientsFound");
var scanStatus = document.getElementById("clientScanStatus");

var res;

function compare(a, b) {
  if (a.p > b.p) return -1;
  if (a.p < b.p) return 1;
  return 0;
}

function toggleBtn(onoff) {
  if (onoff) {
    scanBtn.style.visibility = 'visible';
  } else {
    scanBtn.style.visibility = 'hidden';
  }
}

function getResults(reload) {
  if (reload == undefined) {
    reload = false;
  }
  getResponse("ClientScanResults.json", function(responseText) {
    res = JSON.parse(responseText);
    res.clients = res.clients.sort(compare);

    clientsFound.innerHTML = "Client devices found: " + res.clients.length;

    var tr = '';
    if (res.clients.length > 0) tr += '<tr><th>Pkts</th><th>Vendor</th><th>Name</th><th>MAC</th><th>AP</th><th>Select</th></tr>';

    for (var i = 0; i < res.clients.length; i++) {

      if (res.clients[i].s == 1) tr += '<tr class="selected">';
      else tr += '<tr>';
      tr += '<td>' + res.clients[i].p + '</td>';
      tr += '<td>' + res.clients[i].v + '</td>';
      tr += '<td>' + res.clients[i].n + ' <a class="blue" onclick="changeName(' + res.clients[i].i + ')">edit</a></td>';
      tr += '<td>' + res.clients[i].m + '</td>';
      tr += '<td>' + res.clients[i].a + '</td>';

      if (res.clients[i].s == 1) tr += '<td><button class="marginNull selectedBtn" onclick="select(' + res.clients[i].i + ')">deselect</button></td>';
      else tr += '<td><button class="marginNull" onclick="select(' + res.clients[i].i + ')">select</button></td>';

      tr += '</tr>';
    }
    table.innerHTML = tr;
    toggleBtn(true);
    scanStatus.innerHTML = "";

    if (reload == true) location.reload();

  }, function() {
    location.reload();
  }, 6000);

}

function scan() {
  getResponse("ClientScan.json?time=" + scanTime.value, function(responseText) {
    if (responseText == "true") {
      scanStatus.innerHTML = "scanning...";
      toggleBtn(false);
      setTimeout(function() {
        scanStatus.innerHTML = "reconnecting...";
        getResults(true);
      }, scanTime.value * 1000);
    }
    else alert(responseText);
  });
}

function select(num) {
  getResponse("clientSelect.json?num=" + num, function(responseText) {
    if (responseText == "true") getResults();
    else alert("error :/");
  });
}

function changeName(id) {
  var newName = prompt("Name for " + res.clients[id].m);
  if (newName != null) {
    getResponse("setName.json?id=" + id + "&name=" + newName, function(responseText) {
      if (responseText == "true") getResults();
      else alert("error");
    });
  }
}

getResponse("ClientScanTime.json", function(responseText) {
  scanTime.value = responseText;
});

getResults();