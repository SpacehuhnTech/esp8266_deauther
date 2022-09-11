/* localstorage polyfill (supports Safari Private browsing mode /w fallback to cookie */

// Refer to https://gist.github.com/remy/350433
try {
    // Test webstorage existence.
    if (!window.localStorage || !window.sessionStorage) throw "exception";
    // Test webstorage accessibility - Needed for Safari private browsing.
    localStorage.setItem('storage_test', 1);
    localStorage.removeItem('storage_test');
} catch(e) {
    (function () {
        var Storage = function (type) {
            function createCookie(name, value, days) {
                var date, expires;

                if (days) {
                    date = new Date();
                    date.setTime(date.getTime()+(days*24*60*60*1000));
                    expires = "; expires="+date.toGMTString();
                } else {
                    expires = "";
                }
                document.cookie = name+"="+value+expires+"; path=/";
            }

            function readCookie(name) {
                var nameEQ = name + "=",
                    ca = document.cookie.split(';'),
                    i, c;

                for (i=0; i < ca.length; i++) {
                    c = ca[i];
                    while (c.charAt(0)==' ') {
                        c = c.substring(1,c.length);
                    }

                    if (c.indexOf(nameEQ) == 0) {
                        return c.substring(nameEQ.length,c.length);
                    }
                }
                return null;
            }

            function setData(data) {
                // Convert data into JSON and encode to accommodate for special characters.
                data = encodeURIComponent(JSON.stringify(data));
                // Create cookie.
                if (type == 'session') {
                    createCookie(getSessionName(), data, 365);
                } else {
                    createCookie('localStorage', data, 365);
                }
            }

            function clearData() {
                if (type == 'session') {
                    createCookie(getSessionName(), '', 365);
                } else {
                    createCookie('localStorage', '', 365);
                }
            }

            function getData() {
                // Get cookie data.
                var data = type == 'session' ? readCookie(getSessionName()) : readCookie('localStorage');
                // If we have some data decode, parse and return it.
                return data ? JSON.parse(decodeURIComponent(data)) : {};
            }

            function getSessionName() {
                // If there is no name for this window, set one.
                // To ensure it's unquie use the current timestamp.
                if(!window.name) {
                    window.name = new Date().getTime();
                }
                return 'sessionStorage' + window.name;
            }

            // Initialise if there's already data.
            var data = getData();

            return {
                length: 0,
                clear: function () {
                    data = {};
                    this.length = 0;
                    clearData();
                },
                getItem: function (key) {
                    return data[key] === undefined ? null : data[key];
                },
                key: function (i) {
                    // not perfect, but works
                    var ctr = 0;
                    for (var k in data) {
                        if (ctr == i) return k;
                        else ctr++;
                    }
                    return null;
                },
                removeItem: function (key) {
                    delete data[key];
                    this.length--;
                    setData(data);
                },
                setItem: function (key, value) {
                    data[key] = value+''; // forces the value to a string
                    this.length++;
                    setData(data);
                }
            };
        };

        // Replace window.localStorage and window.sessionStorage with out custom
        // implementation.
        var localStorage = new Storage('local');
        var sessionStorage = new Storage('session');
        window.localStorage = localStorage;
        window.sessionStorage = sessionStorage;
        // For Safari private browsing need to also set the proto value.
        window.localStorage.__proto__ = localStorage;
        window.sessionStorage.__proto__ = sessionStorage;
    })();
}


// returns "index" or "apscan" or "attack" .... etc
var page = location.href.split("/").slice(-1)[0].split('.')[0];
if(page=='') page = 'index';

var l10n = {};

var language = 'en';
 
storage = window.sessionStorage;// window.localStorage is too persistent for what it's worth

function fetchl10n(url) {
  fetch(url)
    .then((resp) => resp.json()) // Transform the data into json
    .then(function(data) {
    l10n = data;
    console.log('saving l10n to storage');
    storage.setItem('l10n', JSON.stringify(l10n));
    loadl10n();
    
  }).catch(function(error) {
    // If there is any error you will catch them here
    console.log(error);
    alert('l10n FAIL: Could not fetch language file ' + url);
    throw('Web UI l10n Failed, check your JSON files');
  });
}

function loadl10n() {
 
  // load language
  var nav =       document.querySelector('nav') || { innerHTML:'' };
  var container = document.querySelector('.container');

  var blocks = {
    navHTML:       nav.innerHTML,
    containerHTML: container.innerHTML
  }

  if(blocks.navHTML!='') {
    for(prop in l10n.navStrings) {
      blocks.navHTML = blocks.navHTML.replace('{' + prop + '}', l10n.navStrings[prop]);
    }
  }

  for(prop in l10n.pageStrings[page]) {
    //console.log('parsing ' + prop);
    blocks.containerHTML = blocks.containerHTML.replace('{' + prop + '}', l10n.pageStrings[page][prop]);
  }

  nav.innerHTML = blocks.navHTML;
  container.innerHTML = blocks.containerHTML;
  
  loadComplete();

}

function loadComplete() {
  // l10n load complete, proceed to the page duty
  const scriptcontext = document.querySelector('script[data-src]');
  if(scriptcontext) {
    scriptcontext.src = scriptcontext.getAttribute('data-src');
  }
}


if(storage) {
  console.log('storage available');
  if(storageLang = storage.getItem('language')) {
    language = storageLang;
    console.log('got language from storage: ' + language);
  } else {
    // first load, save language to storage
    console.log('storing language: ' + language);
    storage.setItem('language', language);
  }

  if(storagel10n = storage.getItem('l10n')) {
    console.log('got l10n from storage');
    l10n = JSON.parse(storagel10n);
    // best use case: language is session preloaded, no neet to fetch JSON
    loadl10n();
  } else {
    // fetch it
    console.log('will fetch l10n json');
    let url = '/l10n/' + language + '.json';
    fetchl10n(url);
  }

} else {

  // using default language
  let url = '/l10n/' + language + '.json';
  fetchl10n(url);
  // alert("no cookie or local/session storage");

}
 

if(page=="settings") {
  // enable language switcher 
  var flagbuttons = document.querySelectorAll('.flag');
  for(i=0;i<flagbuttons.length;i++) {
    var button = flagbuttons[i];
    if(button.getAttribute('data-lang') == language) {
      button.classList.add('selected'); 
    }
    button.addEventListener('click', function(evt) {
      let language = evt.target.getAttribute('data-lang');
      let url = '/l10n/' + language + '.json';
      fetchl10n(url);
      console.log('storing language: ' + language);
      storage.setItem('language', language);
      setTimeout(function() {
        document.location.reload();
      }, 1500);
    }, false);
  }
}