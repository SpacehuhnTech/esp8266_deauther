var table = document.getElementsByTagName('table')[0];
var networkInfo = getE('networksFound');
var scanInfo = getE('scanInfo');
var apMAC = getE('apMAC');
var startStopScan = getE('startStopScan');
var selectAllBtns = getE('selectAllBtns');
var autoScan = false;

function toggleScan(onoff) {
  if (onoff && !autoScan) scanInfo.style.visibility = 'hidden';
  else scanInfo.style.visibility = 'visible';
}

function compare(a, b) {
  if (a.r > b.r) return -1;
  if (a.r < b.r) return 1;
  return 0;
}

function getStatus(enc, hid) {
  /*
  if (enc == 8) return "WPA*";
  else if (enc == 4) return "WPA2";
  else if (enc == 2) return "WPA";
  else if (enc == 7) return "none";
  else if (enc == 5) return "WEP";
  */
	var buff = "";
	if (enc != 7) buff += "&#128274;  ";
	if (hid == 1) buff += "&#128123;  ";
	return buff;
}

function getResults() {
  toggleScan(true);
  getResponse("APScanResults.json", function(responseText) {
    var res;
    try {
      res = JSON.parse(responseText);
    } catch(e) {
      // wut
      showMessage(_("JSON Parsing failed :-("), 2500);
      return;
    }
    // TODO: more sanity checks on res && res.aps
    res.aps = res.aps.sort(compare);
    networkInfo.innerHTML = res.aps.length;
    apMAC.innerHTML = "";
	
	if (res.multiAPs == 1) selectAllBtns.style.visibility = 'visible';
	else selectAllBtns.style.visibility = 'hidden';
		
    var tr = '';
    if (res.aps.length > 0) {
		tr += '<tr><th>Ch</th><th>' + _('SSID') + '</th><th> </th><th>' + _('RSSI') + '</th><th>' + _('Select') + '</th></tr>';
	}	
	
    for (var i = 0; i < res.aps.length; i++) {

      if (res.aps[i].se == 1) tr += '<tr class="selected">';
      else tr += '<tr>';
      tr += '<td>' + res.aps[i].c + '</td>';
      tr += '<td>' + escapeHTML(res.aps[i].ss) + '</td>';
      tr += '<td>' + getStatus(res.aps[i].e, res.aps[i].h) + '</td>';
	  //tr += '<td>' + res.aps[i].r + ' <meter value="' + res.aps[i].r + '" max="-30" min="-100" low="-80" high="-60" optimum="-50"></meter></td>';
	  var _width = res.aps[i].r + 130;
	  var _color;
	  if(res.aps[i].r < -80) _color = "meter_red";
	  else if(res.aps[i].r < -60) _color = "meter_orange";
	  else _color = "meter_green";
	  tr += '<td><div class="meter_background"> <div class="meter_forground '+_color+'" style="width: '+_width+'%;"><div class="meter_value">' + res.aps[i].r + '</div></div> </div></td>';

      if (res.aps[i].se) {
        tr += '<td><button class="select" onclick="select(' + res.aps[i].i + ')">' + _('deselect') + '</button></td>';
        apMAC.innerHTML = res.aps[i].m;
      }
      else tr += '<td><button class="select" onclick="select(' + res.aps[i].i + ')">' + _('select') + '</button></td>';
      tr += '</tr>';
    }
    table.innerHTML = tr;
  });
}

function scan() {
  toggleScan(false);
  getResponse("APScan.json", function(responseText) {
    if (responseText == "true") getResults();
    else showMessage(_("response error APScan.json"));
	toggleScan(true);
  });
}

function select(num) {
  getResponse("APSelect.json?num=" + num, function(responseText) {
    if (responseText == "true") getResults();
    else showMessage(_("response error APSelect.json"));
  });
}

getResults();