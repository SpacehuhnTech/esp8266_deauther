/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

var ssidJson = { "random": false, "ssids": [] };

function load() {
	getFile("run?cmd=save ssids", function () {
		getFile("ssids.json", function (res) {
			ssidJson = JSON.parse(res);
			showMessage("connected");
			draw();
		});
	});
}

function draw() {
	var html;

	html = "<tr>"
		+ "<th class='id'></th>"
		+ "<th class='ssid'></th>"
		+ "<th class='lock'></th>"
		+ "<th class='save'></th>"
		+ "<th class='remove'></th>"
		+ "</tr>";

	for (var i = 0; i < ssidJson.ssids.length; i++) {
		html += "<tr>"
			+ "<td class='id'>" + i + "</td>" // ID
			+ "<td class='ssid' contenteditable='true' id='ssid_" + i + "'>" + esc(ssidJson.ssids[i][0].substring(0, ssidJson.ssids[i][2])) + "</td>" // SSID
			+ "<td class='lock clickable' onclick='changeEnc(" + i + ")' id='enc_" + i + "'>" + (ssidJson.ssids[i][1] ? "&#x1f512;" : "-") + "</td>" // Enc
			+ "<td class='save'><button class='green' onclick='save(" + i + ")'>" + lang("save") + "</button></td>" // Save
			+ "<td class='remove'><button class='red' onclick='remove(" + i + ")'>X</button></td>" // Remove
			+ "</tr>";
	}

	getE("randomBtn").innerHTML = ssidJson.random ? lang("disable_random") : lang("enable_random");

	getE("ssidTable").innerHTML = html;
}

function remove(id) {
	ssidJson.ssids.splice(id, 1);
	getFile("run?cmd=remove ssid " + id);
	draw();
}

function add() {
	var ssidStr = getE("ssid").value;
	var wpa2 = getE("enc").checked;
	var clones = getE("ssidNum").value;
	var force = getE("overwrite").checked;

	if (ssidStr.length > 0) {
		var cmdStr = "add ssid \"" + ssidStr + "\"" + (force ? " -f" : " ") + " -cl " + clones;
		if (wpa2) cmdStr += " -wpa2";

		getFile("run?cmd=" + cmdStr);

		for (var i = 0; i < clones; i++) {
			if (ssidJson.ssids.length >= 60) ssidJson.ssids.splice(0, 1);
			ssidJson.ssids.push([ssidStr, wpa2]);
		}

		draw();
	}
}

function enableRandom() {
	if (ssidJson.random) {
		getFile("run?cmd=disable random", function () {
			load();
		});
	} else {
		getFile("run?cmd=enable random " + getE("interval").value, function () {
			load();
		});
	}

}

function disableRandom() {

}

function addSelected() {
	getFile("run?cmd=add ssid -s" + (getE("overwrite").checked ? " -f" : ""));
}

function changeEnc(id) {
	ssidJson.ssids[id][1] = !ssidJson.ssids[id][1];
	draw();
	save(id);
}

function removeAll() {
	ssidJson.ssids = [];
	getFile("run?cmd=remove ssids");
	draw();
}

function save(id) {
	var name = getE("ssid_" + id).innerHTML.replace("<br>", "").substring(0, 32);
	var wpa2 = ssidJson.ssids[id][1];
	ssidJson.ssids[id] = [name, wpa2];

	getFile("run?cmd=replace ssid " + id + " -n \"" + name + "\" " + (wpa2 ? "-wpa2" : ""));
}

