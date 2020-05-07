let settingsJson = {};

const load = () => {
	getFile("settings.json", res => {
		settingsJson = JSON.parse(res);
		draw();
	});
}

const draw = () => {
	let html = "";

	for (var key in settingsJson) {
		key = esc(key);

		if (settingsJson.hasOwnProperty(key)) {
			html += "<div class='row'>"
				+ "<div class='col-6'>"
				+ "<label class='settingName "+(typeof settingsJson[key] == "boolean" ? "labelFix":"")+"' for='"+key+"'>"+key+":</label>"
				+ "</div>"
				+ "<div class='col-6'>";
				
			if(typeof settingsJson[key] == "boolean"){
				html += "<label class='checkBoxContainer'><input type='checkbox' name='"+key+"' "+(settingsJson[key] ? "checked" : "")+" onchange='save(\""+key+"\",!settingsJson[\""+key+"\"])'><span class='checkmark'></span></label>";
			}else if(typeof settingsJson[key] == "number"){
				html += "<input type='number' name='"+key+"' value="+settingsJson[key]+" onchange='save(\""+key+"\",parseInt(this.value))'>";
			}else if(typeof settingsJson[key] == "string"){
				html += "<input type='text' name='"+key+"' value='"+settingsJson[key].toString()+"' "+(key=="version"?"readonly":"")+" onchange='save(\""+key+"\",this.value)'>";
			}
			
			html += "</div>"
				+ "</div>"
				+ "<div class='row'>"
				+ "<div class='col-12'>"
				+ "<p>"+lang("setting_"+key)+"</p>"
				+ "<hr>"
				+ "</div>"
				+ "</div>";
		}
	}
	getE("settingsList").innerHTML = html;
}

const save = (key, value) => {
	if (key) {
		settingsJson[key] = value;
		getFile(`run?cmd=set ${key} "${value}"`);
	} else {
		getFile("run?cmd=save settings", load);
	}
}