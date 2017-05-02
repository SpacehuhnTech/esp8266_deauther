function showMessage(msg, closeAfter){
	document.getElementById("error").innerHTML = msg;
	if(closeAfter !== undefined){
		setTimeout(function(){
			document.getElementById("error").innerHTML = "";
		},closeAfter);
	}
}

function getE(name){
	return document.getElementById(name);
}

function getResponse(adr, callback, timeoutCallback, timeout, method){
	if(timeoutCallback === undefined) {
		timeoutCallback = function(){
			showMessage("error loading "+adr);
		};
	}
	if(timeout === undefined) timeout = 8000; 
	if(method === undefined) method = "GET";
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if(xmlhttp.readyState == 4){
			if(xmlhttp.status == 200){
				showMessage("");
				callback(xmlhttp.responseText);
			}
			else timeoutCallback();
		}
	};
	xmlhttp.open(method, adr, true);
	xmlhttp.send();
	xmlhttp.timeout = timeout;
	xmlhttp.ontimeout = timeoutCallback;
}