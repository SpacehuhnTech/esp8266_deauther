let ssidJson = {
	"random": false,
	"ssids": []
};

const load = () => {
	getFile("run?cmd=save ssids", () =>
		getFile("ssids.json", res => {
			ssidJson = JSON.parse(res);
			draw();
		})
	);
}

const draw = () => {
	let html = `
		<tr>
			<th class="id"></th>
			<th class="ssid"></th>
			<th class="lock"></th>
			<th class="save"></th>
			<th class="remove"></th>
		</tr>
	`;

	ssidJson.ssids.map((item, i) => {

		html += `
			<tr>
				<td class="id">${i}</td>
				<td class="ssid" contenteditable="true" id="ssid_${i}">${esc(ssidJson.ssids[i][0].substring(0, ssidJson.ssids[i][2]))}</td>
				<td class="lock clickable" onclick="changeEnc(${i})" id="enc_${i}">${ssidJson.ssids[i][1] ? "&#x1f512;" : "-"}</td>
				<td class="save">
					<button class="green" onclick="save(${i})">${lang("save")}</button>
				</td>
				<td class="remove">
					<button class="red" onclick="remove(${i})">X</button>
				</td>
			</tr>
		`;
	})

	getE("randomBtn").innerHTML = lang(ssidJson.random ? "disable_random" : "enable_random");
	getE("ssidTable").innerHTML = html;
}

const remove = id => {
	ssidJson.ssids.splice(id, 1);
	getFile(`run?cmd=remove ssid ${id}`);
	draw();
}

const add = () => {
	const ssidStr = getE("ssid").value;

	if (ssidStr.length > 0) {
		const wpa2 = getE("enc").checked;
		const clones = getE("ssidNum").value;
		const force = getE("overwrite").checked;

		const cmdStr = `add ssid "${ssidStr}" ${force ? "-f" : ""} -cl ${clones} ${wpa2 ? -wpa2 : ""}`;

		getFile(`run?cmd=${cmdStr}`);

		for (let i = 0; i < clones; i++) {
			if (ssidJson.ssids.length >= 60) ssidJson.ssids.splice(0,1);
			ssidJson.ssids.push([ssidStr, wpa2]);
		}

		draw();
	}
}

const enableRandom = () => {
	if (ssidJson.random) {
		getFile("run?cmd=disable random", load);
	} else {
		getFile(`run?cmd=enable random ${getE("interval").value}`, load);
	}

}

const disableRandom = () => {

}

const addSelected = () => {
	getFile(`run?cmd=add ssid -s ${getE("overwrite").checked ? "-f":""}`);
}

const changeEnc = id => {
	ssidJson.ssids[id][1] = !ssidJson.ssids[id][1];

	draw();
	save(id);
}

const removeAll = () => {
	ssidJson.ssids = [];

	getFile("run?cmd=remove ssids");
	draw();
}

const save = id => {
	const name = getE(`ssid_${id}`).innerHTML.replace("<br>", "").substring(0, 32);
	const wpa2 = ssidJson.ssids[id][1];
	ssidJson.ssids[id] = [name, wpa2];

	getFile(`run?cmd=replace ssid ${id} -n "${name}" ${wpa2 ? "-wpa2" : ""}`);
}

