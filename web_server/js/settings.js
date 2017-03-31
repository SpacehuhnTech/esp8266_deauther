var nameListTable = document.getElementById('nameList');
var ssid = document.getElementById('ssid');
var ssidHidden = document.getElementById('ssidHidden');
var password = document.getElementById('password');
var apChannel = document.getElementById('apChannel');
var apScanHidden = document.getElementById('apScanHidden');
var scanTime = document.getElementById('scanTime');
var timeout = document.getElementById('timeout');
var deauthReason = document.getElementById('deauthReason');
var packetRate = document.getElementById('packetRate');
var saved = document.getElementById('saved');
var clientNames = document.getElementById('clientNames');
var ssidEnc = document.getElementById('ssidEnc');
var useLed = document.getElementById('useLed');
var channelHop = document.getElementById('channelHop');
var res;

function getData() {
  getResponse("settings.json", function(responseText) {
    res = JSON.parse(responseText);

    ssid.value = res.ssid;
    ssidHidden.checked = res.ssidHidden;
    password.value = res.password;
    apChannel.value = res.apChannel;
    apScanHidden.checked = res.apScanHidden;
    scanTime.value = res.clientScanTime;
    timeout.value = res.attackTimeout;
    deauthReason.value = res.deauthReason;
    packetRate.value = res.attackPacketRate;
    ssidEnc.checked = res.attackEncrypted;
    useLed.checked = res.useLed;
    channelHop.checked = res.channelHop;


    clientNames.innerHTML = "Client Names " + res.nameList.length + "/50";

    var tr = '<tr><th>MAC</th><th>Vendor</th><th>Name</th><th>X</th></tr>';

    for (var i = 0; i < res.nameList.length; i++) {

      tr += '<tr>';
      tr += '<td>' + res.nameList[i].m + '</td>';
      tr += '<td>' + res.nameList[i].v + '</td>';
      tr += '<td>' + res.nameList[i].n + ' <a class="blue" onclick="changeName(' + i + ')">edit</a></td>';
      tr += '<td><button class="marginNull warnBtn" onclick="deleteName(' + i + ')">x</button></td>';

      tr += '</tr>';
    }

    nameListTable.innerHTML = tr;
  });
}

function changeName(id) {
  var newName = prompt("Name for " + res.nameList[id].m);
  if (newName != null) {
    getResponse("editNameList.json?id=" + id + "&name=" + newName, function(responseText) {
      if (responseText == "true") getData();
      else alert("error");
    });
  }
}

function deleteName(id) {
  getResponse("deleteName.json?num=" + id, function(responseText) {
    if (responseText == "true") getData();
    else alert("error");
  });
}

function saveSettings() {
  saved.innerHTML = "saving...";
  var url = "settingsSave.json";
  url += "?ssid=" + ssid.value;
  url += "&ssidHidden=" + ssidHidden.checked;
  url += "&password=" + password.value;
  url += "&apChannel=" + apChannel.value;
  url += "&apScanHidden=" + apScanHidden.checked;
  url += "&scanTime=" + scanTime.value;
  url += "&timeout=" + timeout.value;
  url += "&deauthReason=" + deauthReason.value;
  url += "&packetRate=" + packetRate.value;
  url += "&ssidEnc=" + ssidEnc.checked;
  url += "&useLed=" + useLed.checked;
  url += "&channelHop=" + channelHop.checked;

  getResponse(url, function(responseText) {
    if (responseText == "true") {
      getData();
      saved.innerHTML = "saved";
    }
    else alert("error");
  });
}

function resetSettings() {
  getResponse("settingsReset.json", function(responseText) {
    if (responseText == "true") {
      getData();
      saved.innerHTML = "saved";
    }
    else alert("error");
  });
}

function clearNameList() {
  getResponse("clearNameList.json", function(responseText) {
    if (responseText == "true") getData();
    else alert("error");
  });
}

getData();