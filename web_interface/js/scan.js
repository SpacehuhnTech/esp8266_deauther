let nameJson = [];
let scanJson = {
	aps: [],
	stations: []
};

const drawScan = () => {
	let html;

	// Access Points
	getE("apNum").innerHTML = scanJson.aps.length;
	html = `
		<tr>
			<th class="id"></th>
			<th class="ssid">SSID</th>
			<th class="name">Name</th>
			<th class="ch">Ch</th>
			<th class="rssi">RSSI</th>
			<th class="enc">Enc</th>
			<th class="lock"></th>
			<th class="mac">MAC</th>
			<th class="vendor">Vendor</th>
			<th class="selectColumn"></th>
			<th class="remove"></th>
		</tr>
	`

	scanJson.aps.forEach((item, index) => {
		const selected = item[item.length - 1];
		const width = parseInt(item[3]) + 130;
		let color = '';

		if (width < 50) color = "meter_red";
		else if (width < 70) color = "meter_orange";
		else color = "meter_green";

		html += `
			<tr ${selected ? "class='selected'>" : ""}>
				<td class="id">${index}</td>
				<td class="ssid">${esc(item[0])}</td>
				<td class="name">
					${item[1].length > 0 ? esc(item[1]) : `<button onclick="add(0, ${i})">lang("add")</button>`}
				</td>
				<td class="ch">${esc(item[2])}</td>"
				<td class="rssi">
					<div class="meter_background">
						<div class="meter_forground ${color}" style="width: ${width}%;">
							<div class="meter_value">
								item[3]
							</div>
						</div>
					</div>
				</td>
				<td class="enc">${esc(item[4])}</td>
				<td class="lock">${item[4] === "-" ? "" : "&#x1f512;"}</td>
				<td class="mac">${esc(item[5])}</td>
				<td class="vendor">${esc(item[6])}</td>
				<td class="selectColumn">
					<label class="checkBoxContainer">
						<input type="checkbox ${selected ? "checked" : ""}" onclick="selectRow(0, ${i}, ${selected})">
						<span class="checkmark"></span>
					</label>
				</td>
				<td class="remove">
					<button class='red' onclick="remove(0, ${i})">X</button>
				</td>
			</tr>
		`;
	})

	getE("apTable").innerHTML = html;

	// Stations
	getE("stNum").innerHTML = scanJson.stations.length;

	html = `
		<tr>
			<th class="id"></th>
			<th class="vendor">Vendor</th>
			<th class="mac">MAC</th>
			<th class="ch">Ch</th>
			<th class="name">Name</th>
			<th class="pkts">Pkts</th>
			<th class="ap">AP</th>
			<th class="lastseen">Last seen</th>
			<th class="selectColumn"></th>
			<th class="remove"></th>
		</tr>
	`;

	scanJson.stations.forEach((item, index) => {
		const selected = item[item.length - 1];
		const ap = item[5] >= 0 ? esc(scanJson.aps[item[5]][0]) : "";

		html += `
			<tr ${selected ? "class='selected'>" : ""}>
				<td class="id">${i}</td>
				<td class="vendor">${esc(item[3])}</td>
				<td class="mac">${esc(item[0])}</td>
				<td class="ch">${esc(item[1])}</td>
				<td class="name">
					${item[2].length > 0 ? esc(item[2]) : `<button onclick="add(1, ${index})">lang("add")</button>`}
				</td>
				<td class="pkts">${esc(item[4])}</td>
				<td class="ap">${ap}</td>
				<td class="lastseen">${esc(item[6])}</td>
				<td class="selectColumn">
					<label class="checkBoxContainer">
						<input type="checkbox" ${selected ? "checked" : ""} onclick="selectRow(1, ${index}, ${selected})">
						<span class="checkmark"></span>
					</label>
				</td>
				<td class="remove">
					<button class="red" onclick="remove(1, ${index})">X</button>
				</td>
			</tr>
		`;
	})

	getE("stTable").innerHTML = html;
}

const drawNames = () => {
	getE("nNum").innerHTML = nameJson.length;
	let html = `
		<tr>
			<th class="id"></th>
			<th class="mac">MAC</th>
			<th class="vendor">Vendor</th>
			<th class="name">Name</th>
			<th class="ap">AP-BSSID</th>
			<th class="ch">Ch</th>
			<th class="save"></th>
			<th class="selectColumn"></th>
			<th class="remove"></th>
		</tr>`

	nameJson.forEach((item, index) => {
		const selected = item[item.length - 1];

		html += `
			<tr ${selected ? "class='selected'>" : ""}>
				<td class="id">${index}</td>
				<td class="mac" contentEditable="true" id="name_${index}_mac">${esc(item[0])}</td>
				<td class="vendor">${esc(item[1])}</td>
				<td class="name" contentEditable="true" id="name_${index}_name">${esc(item[2].substring(0,16))}</td>
				<td class="ap" contentEditable="true" id="name_${index}_apbssid">${esc(item[3])}</td>
				<td class="ch" contentEditable="true" id="name_${index}_ch">${esc(item[4])}</td>
				<td class="save">
					<button class="green" onclick="save(${index})">${lang("save")}</button>
				</td>
				<td class="selectColumn">
					<label class="checkBoxContainer">
						<input type="checkbox" ${selected ? "checked" : ""} onclick="selectRow(2, ${index}, ${selected})">
						<span class="checkmark"></span>
					</label>
				</td>
				<td class="remove">
					<button class="red" onclick="remove(2, ${index})">X</button>
				</td>
			</tr>
		`;
	})

	getE("nTable").innerHTML = html;
}
				
var duts;
var elxtime;
function scan(type){
	getE('RButton').disabled = true;
	switch(type){
		case 0:
			getE('scanOne').disabled = true;
			getE('scanZero').style.visibility = 'hidden';
			elxtime = 2450;
			break;
		case 1:
			getE('scanZero').disabled = true;
			getE('scanOne').style.visibility = 'hidden';
			elxtime = parseInt(getE("scanTime").value+"000") + 1500;
	}
	var cmdStr = "scan "
		+ (type == 0 ? "aps " : "stations -t "+getE("scanTime").value+"s")
		+ " -ch "+getE("ch").options[getE("ch").selectedIndex].value;
	getFile("run?cmd="+cmdStr);
	duts = parseInt(type);
	setTimeout(buttonFunc, elxtime)
}

function buttonFunc(){
	switch (duts) {
		case 0:
			getE('scanZero').style.visibility = 'visible';
			getE('scanOne').disabled = false;
			break;
		case 1:
			getE('scanOne').style.visibility = 'visible';
			getE('scanZero').disabled = false;
	}
	getE('RButton').disabled = false;
}

function load(){
	// APs and Stations
	getFile("run?cmd=save scan", () =>
		getFile("scan.json", res => {
			scanJson = JSON.parse(res);
			drawScan();
		})
	);

	// Names
	getFile("run?cmd=save names", () =>
		getFile("names.json", res => {
			nameJson = JSON.parse(res);
			drawNames();
		})
	);
}

const selectRow = (type, id, selected) => {
	if (type === 0 || type === 1) {
		scanJson.aps[id][7] = selected;
		drawScan();
	} else if (type === 2) {
		save(id);
		nameJson[id][5] = selected;
		drawNames();
	}

	switch(type) {
		case 0:
			return getFile(`run?cmd=${selected ? "" : "de" } select ap ${id}`);
		case 1:
			return getFile(`run?cmd=${selected ? "" : "de" } select station ${id}`);
		case 2:
			return getFile(`run?cmd=${selected ? "" : "de" } select name ${id}`);
	}
}

const remove = (type, id) => {
	if (type === 0 || type === 1) {
		scanJson.aps.splice(id, 1);
		drawScan();
	} else if (type === 2) {
		nameJson.splice(id, 1);
		drawNames();
	}

	switch(type) {
		case 0:
			getFile(`run?cmd=remove ap ${id}`);
			break;
		case 1:
			getFile(`run?cmd=remove station ${id}`);
			break;
		case 2:
			getFile(`run?cmd=remove name ${id}`);
	}
}

const save = id => {
	const mac = getE(`name_${id}_mac`).innerHTML.replace("<br>" ,"");
	const name = getE(`name_${id}_name`).innerHTML.replace("<br>", "");
	const apbssid = getE(`name_${id}_apbssid`).innerHTML.replace("<br>", "");
	const ch = getE(`name_${id}_ch`).innerHTML.replace("<br>", "");
	const changed = mac !== nameJson[id][0] || name !== nameJson[id][2] || apbssid !== nameJson[id][3] || ch !== nameJson[id][4];

	if (changed) {
		nameJson[id][0] = mac;
		nameJson[id][2] = name;
		nameJson[id][3] = apbssid;
		nameJson[id][4] = ch;

		if (nameJson[id][0].length !== 17) {
			return showMessage("ERROR: MAC invalid");
		}

		getFile(`run?cmd=replace name ${id} -n "${nameJson[id][2]}" -m "${nameJson[id][0]}" -ch ${nameJson[id][4]} -b "${nameJson[id][3]}" ${nameJson[id][5] ? "-s" : ""}`);

		drawNames();
	}
}

const add = (type, id) => {
	if(nameJson.length >= 25){
		return showMessage("Device Name List is full!");
	}

	switch(type) {
		case 0:
			getFile(`run?cmd=add name "${scanJson.aps[id][0]}" -ap ${id}`);
			scanJson.aps[id][1] = scanJson.aps[id][0]; // name = SSID
			nameJson.push([scanJson.aps[id][5], scanJson.aps[id][6], scanJson.aps[id][0], "", scanJson.aps[id][2], false]);
			drawScan();
			break;
		case 1:
			getFile(`run?cmd=add name "${scanJson.stations[id][0]}" station ${id}`);
			scanJson.stations[id][2] = `device_${nameJson.length}`; // name = device_
			nameJson.push([scanJson.stations[id][0], scanJson.stations[id][3], `device_${nameJson.length}`, scanJson.aps[scanJson.stations[id][5]][5], scanJson.stations[id][1], false]);
			drawScan();
			break;
		case 2:
			getFile(`run?cmd=add name device_${nameJson.length} -m 00:00:00:00:00:00 -ch 1`);
			nameJson.push(["00:00:00:00:00:00", "", `device_${nameJson.length}`, "", 1, false]);
			drawNames();
	}

	drawNames();
}

const selectAll = (type, select) => {
	switch(type) {
		case 0:
			getFile(`run?cmd=${select ? "" : "de"}select aps`);
			scanJson.aps.forEach(item => {
				item[7] = select;
				return item
			})
			drawScan();
			break;
		case 1:
			getFile(`run?cmd=${select ? "" : "de"}select stations`);
			scanJson.stations.forEach(item => {
				item[7] = select;
				return item
			})
			drawScan();
			break;
		case 2:
			getFile(`run?cmd=${select ? "" : "de"}select names`);
			nameJson.stations.forEach(item => {
				item[5] = select;
				return item
			})
			drawNames();
	}
}