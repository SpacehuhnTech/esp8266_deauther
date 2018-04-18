var attackJSON = [[false,0,0],[false,0,0],[false,0,0]];
			
function draw(){
	getE("deauth").innerHTML = attackJSON[0][0] ? lang("stop") : lang("start");
	getE("beacon").innerHTML = attackJSON[1][0] ? lang("stop") : lang("start");
	getE("probe").innerHTML = attackJSON[2][0] ? lang("stop") : lang("start");
	
	getE("deauthTargets").innerHTML = esc(attackJSON[0][1]+"");
	getE("beaconTargets").innerHTML = esc(attackJSON[1][1]+"");
	getE("probeTargets").innerHTML = esc(attackJSON[2][1]+"");
	
	getE("deauthPkts").innerHTML = esc(attackJSON[0][2] + "/" + attackJSON[0][3]);
	getE("beaconPkts").innerHTML = esc(attackJSON[1][2] + "/" + attackJSON[1][3]);
	getE("probePkts").innerHTML = esc(attackJSON[2][2] + "/" + attackJSON[2][3]);
	
	getE("allpkts").innerHTML = esc(attackJSON[3]+"");
}

function start(mode){
	switch(mode){
		case 0:
			attackJSON[0][0] = !attackJSON[0][0];
			break;
		case 1:
			attackJSON[1][0] = !attackJSON[1][0];
			break;
		case 2:
			attackJSON[2][0] = !attackJSON[2][0];
			break;
	}
	getFile("run?cmd=attack"+(attackJSON[0][0] ? " -d":"")+(attackJSON[1][0] ? " -b":"")+(attackJSON[2][0] ? " -p":""),function(){
		draw();
	});
}

function load(){
	getFile("attack.json",function(response){
		attackJSON = JSON.parse(response);
		draw();
	});
}