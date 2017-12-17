var ssid = getE('ssid');
var ssidHidden = getE('ssidHidden');
var password = getE('password');
var apChannel = getE('apChannel');
var macAp = getE('macAp');
var randMacAp = getE('randMacAp');
var apScanHidden = getE('apScanHidden');
var scanTime = getE('scanTime');
var timeout = getE('timeout');
var deauthReason = getE('deauthReason');
var packetRate = getE('packetRate');
var saved = getE('saved');
var clientNames = getE('clientNames');
var useLed = getE('useLed');
/*var channelHop = getE('channelHop');*/
var multiAPs = getE('multiAPs');
var multiAttacks = getE('multiAttacks');
var cMac = getE('cMac');
var cName = getE('cName');
var macInterval = getE('macInterval');
var beaconInterval = getE('beaconInterval');
var ledPin = getE('ledPin');
var res;

function getData() {
  getResponse("settings.json", function(responseText) {
	try {
        res = JSON.parse(responseText);
    } catch(e) {
        showMessage("Error: reset the settings.");
		return;
    }
	ssid.value = res.ssid;
	ssidHidden.checked = res.ssidHidden;
	password.value = res.password;
	apChannel.value = res.apChannel;
	macAp.value = res.macAp;
	randMacAp.checked = res.randMacAp;
	apScanHidden.checked = res.apScanHidden;
	scanTime.value = res.clientScanTime;
	timeout.value = res.attackTimeout;
	deauthReason.value = res.deauthReason;
	packetRate.value = res.attackPacketRate;
	useLed.checked = res.useLed;
	/*channelHop.checked = res.channelHop;*/
	multiAPs.checked = res.multiAPs;
	multiAttacks.checked = res.multiAttacks;
	macInterval.value = res.macInterval;
	beaconInterval.checked = res.beaconInterval;
	ledPin.value = res.ledPin;
  });
}

function saveSettings() {
  saved.innerHTML = "saving...";
  var url = "settingsSave.json";
  url += "?ssid=" + ssid.value;
  url += "&ssidHidden=" + ssidHidden.checked;
  url += "&password=" + password.value;
  url += "&apChannel=" + apChannel.value;
  url += "&macAp=" + macAp.value;
  url += "&randMacAp=" + randMacAp.checked;
  url += "&apScanHidden=" + apScanHidden.checked;
  url += "&scanTime=" + scanTime.value;
  url += "&timeout=" + timeout.value;
  url += "&deauthReason=" + deauthReason.value;
  url += "&packetRate=" + packetRate.value;
  url += "&useLed=" + useLed.checked;
  /*url += "&channelHop=" + channelHop.checked;*/
  url += "&multiAPs="+multiAPs.checked;
  url += "&multiAttacks="+multiAttacks.checked;
  url += "&macInterval="+macInterval.value;
  url += "&beaconInterval="+beaconInterval.checked;
  url += "&ledPin="+ledPin.value;

  getResponse(url, function(responseText) {
    if (responseText == "true") {
      getData();
      saved.innerHTML = "saved";
    }
    else showMessage("response error settingsSave.json");
  });
}

function resetSettings() {
  getResponse("settingsReset.json", function(responseText) {
    if (responseText == "true") {
      getData();
      saved.innerHTML = "saved";
    }
    else showMessage("response error settingsReset.json");
  });
}

function restart(){
	getResponse("restartESP.json?", function(){});
}

getData();