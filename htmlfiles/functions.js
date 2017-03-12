function getResponse(adr, callback, timeoutCallback, timeout){
	if(timeoutCallback === undefined) { 
		timeoutCallback = function(){
			location.reload()
		};
	}
	if(timeout === undefined) { 
		timeout = 8000; 
	}
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if(xmlhttp.readyState == 4){
	    if(xmlhttp.status == 200) callback(xmlhttp.responseText);
			else timeoutCallback();
		}
	};
	xmlhttp.open("GET", adr, true);
	xmlhttp.send();
	xmlhttp.timeout = timeout;
  xmlhttp.ontimeout = timeoutCallback;
}