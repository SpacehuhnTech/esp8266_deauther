Use this converter to minify and gzip everything in the `web_interface` folder and put it in `esp8266_deauther/data/web/`.  
This script will also generate a new `webfiles.h` file and replace the old in `esp8266_deauther`.

Copyright goes to [@xdavidhu](http://github.com/xdavidhu/).  

**A few notes:**  
- you need python3 to run this script
- you need to install the anglerfish package: `sudo python3 -m pip install anglerfish`
- be sure to run the script from its current position
- `.lang` files will always go in the `/lang` folder
- `.js` files will always go int the `/js` folder
- `.json` files will be ignored and not copied
- only `.html` and `.css` will be minified before being gzipped (minifying JS can make problems)
