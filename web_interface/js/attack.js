const attackJSON = [
	[false, 0, 0],
	[false, 0, 0],
	[false, 0, 0]
];

const draw = () => {
	getE("deauth").innerHTML = lang(attackJSON[0][0] ? "stop" : "start");
	getE("beacon").innerHTML = lang(attackJSON[1][0] ? "stop" : "start");
	getE("probe").innerHTML = lang(attackJSON[2][0] ? "stop" : "start");

	getE("deauthTargets").innerHTML = esc(`${attackJSON[0][1]}`);
	getE("beaconTargets").innerHTML = esc(`${attackJSON[1][1]}`);
	getE("probeTargets").innerHTML = esc(`${attackJSON[2][1]}`);

	getE("deauthPkts").innerHTML = esc(`${attackJSON[0][2]}/${attackJSON[0][3]}`);
	getE("beaconPkts").innerHTML = esc(`${attackJSON[1][2]}/${attackJSON[1][3]}`);
	getE("probePkts").innerHTML = esc(`${attackJSON[2][2]}/${attackJSON[2][3]}`);

	getE("allpkts").innerHTML = esc(`${attackJSON[3]}`);
}

const start = (mode) => {
	attackJSON[mode][0] = !attackJSON[mode][0];

	getFile(`run?cmd=attack${attackJSON[0][0] ? " -d":""}${attackJSON[1][0] ? " -b":""}${attackJSON[2][0] ? " -p":""}`, draw);
}

const load = () => {
	getFile("attack.json", (response) => {
		attackJSON = JSON.parse(response);
		draw();
	});
}