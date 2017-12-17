function getE(name){
	return document.getElementById(name);
}

function escapeHTML(str) {
    return str
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/\"/g, '&quot;')
      .replace(/\'/g, '&#39;')
      .replace(/\//g, '&#x2F;')
}

function showMessage(msg, closeAfter){
	var errorE = getE("error");
	errorE.innerHTML = msg;
	
	errorE.classList.remove('hide');
	errorE.classList.add('show');

	if(closeAfter !== undefined){
		setTimeout(function(){
			errorE.innerHTML = "";
			errorE.classList.remove('show');
			errorE.classList.add('hide');
		},closeAfter);
	}
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