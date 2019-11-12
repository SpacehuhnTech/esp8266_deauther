let langJson = {};

const getE = (name) => document.getElementById(name);

const esc = str => {
	if (str) {
		return str.toString()
			.replace(/&/g, "&amp;")
			.replace(/</g, "&lt;")
			.replace(/>/g, "&gt;")
			.replace(/\"/g, "&quot;")
			.replace(/\'/g, "&#39;")
			.replace(/\//g, "&#x2F;");
	}

	return "";
}

const convertLineBreaks = str => {
	if (str) {
		str = str.toString();
		str = str.replace(/(?:\r\n|\r|\n)/g, '<br>');
		return str;
	}

	return "";
}

const showMessage = (msg, closeAfter) => {
	const elmt = getE("error");
	elmt.innerHTML = `${esc(msg)}<a onclick='closeMessage()' id='closeError'>x</a>`;

	elmt.classList.remove("hide");
	elmt.classList.add("show");

	if(closeAfter !== undefined){
		setTimeout(closeMessage, closeAfter);
	}
}

const closeMessage = () => {
	const elmt = getE("error");

	elmt.innerHTML = "";
	elmt.classList.remove("show");
	elmt.classList.add("hide");
}

const getFile = (
	adr,
	callback = () => undefined,
	timeout = 8000,
	method = "GET",
	onTimeout = showMessage(`ERROR: timeout loading file ${adr}`),
	onError = showMessage(`ERROR: loading file: ${adr}`)
) => {
	/* fallback stuff */
	if(adr === undefined) return;

	/* create request */
	const request = new XMLHttpRequest();

	/* set parameter for request */
	request.open(method, encodeURI(adr), true);
	request.timeout = timeout;
	request.ontimeout = onTimeout;
  request.onerror = onError;
	request.overrideMimeType("application/json");

	request.onreadystatechange = () => {
		if (this.readyState == 4 && this.status == 200) {
			callback(this.responseText);
		}
	};

	/* send request */
	request.send();

	console.log(adr);
}

const lang = key => convertLineBreaks(esc(langJson[key]));

const parseLang = fileStr => {
	langJson = JSON.parse(fileStr);

	if (langJson["lang"] !== "en") {
		// no need to update the HTML
		const elements = document.querySelectorAll("[data-translate]");

		[...elements].forEach(item => {
			item.innerHTML = lang(element.getAttribute("data-translate"));
		})
	}

	document.querySelector('html').setAttribute("lang", langJson["lang"]);

	if(typeof load !== 'undefined') load();
}

const loadLang = () => {
	const language = "default"; //navigator.language.slice(0, 2);
	const langFunc = () => getFile("lang/en.lang", parseLang);

	getFile(
		`lang/${language}.lang`,
		parseLang,
		2000,
		"GET",
		langFunc,
		langFunc
	);
}
