#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by: xdavidhu

import os
import gzip
import argparse
import binascii
from pathlib import Path, PurePath
try:
    from css_html_js_minify.minify import process_single_html_file, process_single_js_file, process_single_css_file
except ModuleNotFoundError:
    print("\n[!] Requirements are not satisfied. Please install the 'anglerfish' package by running 'sudo python3 -m pip install anglerfish'.\n")
    exit()

parser = argparse.ArgumentParser(usage="webConverter.py --repopath [path-to-repo]")
parser.add_argument("--repopath", type=str,
                    help='Path to the repo, if not set make sure to run the script from [repo]/utils/web_converter_python/ directory')

print("\nwebConverter for the deauther2.0 by @xdavidhu\n")

args = parser.parse_args()
if args.repopath != None:
    parent = args.repopath
    print("[+] Using manual path '" + args.repopath + "'\n")
else:
    p = Path.cwd()
    parent = p.parent.parent
license_file_path = str(os.path.join(str(parent), "LICENSE"))
q = PurePath('esp8266_deauther')
arduino_file_path = str(os.path.join(str(parent / q), "webfiles.h"))
datadir = parent / q
q = PurePath('web_interface')
dir = parent / q
q = PurePath('data')
datadir = datadir / q
if not os.path.exists(str(datadir)):
    os.mkdir(str(datadir))
q = PurePath('web')
compressed = datadir / q
if not os.path.exists(str(compressed)):
    os.mkdir(str(compressed))

html_files = []
css_files = []
js_files = []
lang_files = []
progmem_definitions = ""
copy_files_function = ""
webserver_events = ""
load_lang = ""

filelist = Path(dir).glob('**/*')
for x in filelist:
    if x.is_file():
        if x.parts[-2] == "compressed" or x.parts[-3] == "compressed":
            continue
        if x.suffix == ".html":
            html_files.append(x)
        elif x.suffix == ".css":
            css_files.append(x)
        elif x.suffix == ".js":
            js_files.append(x)
        elif x.suffix == ".lang":
            lang_files.append(x)

for file in html_files:
    base_file = os.path.basename(str(file))
    original_file = str(file)
    new_file = str(os.path.join(str(compressed), str(base_file)))
    print("[+] Minifying " + base_file + "...")
    process_single_html_file(original_file, output_path=new_file)
    print("[+] Compressing " + base_file + "...")
    f_in = open(new_file, encoding='UTF-8')
    content = f_in.read()
    f_in.close()
    os.remove(new_file)
    with gzip.GzipFile(new_file + ".gz", mode='w') as fo:
        fo.write(content.encode("UTF-8"))
    f_in = open(new_file + ".gz", 'rb')
    content = f_in.read()
    f_in.close()
    array_name = base_file.replace(".", "")
    hex_formatted_content = ""
    hex_content = binascii.hexlify(content)
    hex_content = hex_content.decode("UTF-8")
    hex_content = [hex_content[i:i+2] for i in range(0, len(hex_content), 2)]
    for char in hex_content:
        hex_formatted_content += "0x" + char + ", "
    hex_formatted_content = hex_formatted_content[:-2]
    progmem_definitions += "const char " + array_name + "[] PROGMEM = {" + hex_formatted_content + "};\n"
    copy_files_function += '  if(!LittleFS.exists("/web/' + base_file + '.gz") || force) progmemToSpiffs(' + array_name + ', sizeof(' + array_name + '), "/web/' + base_file + '.gz");\n'
    webserver_events += 'server.on("/' + base_file + '", HTTP_GET, [](){\n  sendProgmem(' + array_name + ', sizeof(' + array_name + '), W_HTML);\n});\n'

for file in css_files:
    base_file = os.path.basename(str(file))
    original_file = str(file)
    new_file = str(os.path.join(str(compressed), str(base_file)))
    print("[+] Minifying " + base_file + "...")
    process_single_css_file(original_file, output_path=new_file)
    print("[+] Compressing " + base_file + "...")
    f_in = open(new_file, encoding='UTF-8')
    content = f_in.read()
    f_in.close()
    os.remove(new_file)
    with gzip.GzipFile(new_file + ".gz", mode='w') as fo:
        fo.write(content.encode("UTF-8"))
    f_in = open(new_file + ".gz", 'rb')
    content = f_in.read()
    f_in.close()
    array_name = base_file.replace(".", "")
    hex_formatted_content = ""
    hex_content = binascii.hexlify(content)
    hex_content = hex_content.decode("UTF-8")
    hex_content = [hex_content[i:i+2] for i in range(0, len(hex_content), 2)]
    for char in hex_content:
        hex_formatted_content += "0x" + char + ", "
    hex_formatted_content = hex_formatted_content[:-2]
    progmem_definitions += "const char " + array_name + "[] PROGMEM = {" + hex_formatted_content + "};\n"
    copy_files_function += '  if(!LittleFS.exists("/web/' + base_file + '.gz") || force) progmemToSpiffs(' + array_name + ', sizeof(' + array_name + '), "/web/' + base_file + '.gz");\n'
    webserver_events += 'server.on("/' + base_file + '", HTTP_GET, [](){\n  sendProgmem(' + array_name + ', sizeof(' + array_name + '), W_CSS);\n});\n'

for file in js_files:
    q = PurePath('js')
    compressed_js = compressed / q
    if not os.path.exists(str(compressed_js)):
        os.mkdir(str(compressed_js))
    base_file = os.path.basename(str(file))
    original_file = str(file)
    new_file = str(os.path.join(str(compressed_js), str(base_file)))
    #print("[+] Minifying " + base_file + "...")
    #process_single_js_file(original_file, output_path=new_file)
    print("[+] Compressing " + base_file + "...")
    f_in = open(original_file, encoding='UTF-8')
    content = f_in.read()
    f_in.close()
    #os.remove(new_file)
    with gzip.GzipFile(new_file + ".gz", mode='w') as fo:
        fo.write(content.encode("UTF-8"))
    f_in = open(new_file + ".gz", 'rb')
    content = f_in.read()
    f_in.close()
    array_name = base_file.replace(".", "")
    hex_formatted_content = ""
    hex_content = binascii.hexlify(content)
    hex_content = hex_content.decode("UTF-8")
    hex_content = [hex_content[i:i+2] for i in range(0, len(hex_content), 2)]
    for char in hex_content:
        hex_formatted_content += "0x" + char + ", "
    hex_formatted_content = hex_formatted_content[:-2]
    progmem_definitions += "const char " + array_name + "[] PROGMEM = {" + hex_formatted_content + "};\n"
    copy_files_function += '  if(!LittleFS.exists("/web/js/' + base_file + '.gz") || force) progmemToSpiffs(' + array_name + ', sizeof(' + array_name + '), "/web/js/' + base_file + '.gz");\n'
    webserver_events += 'server.on("/js/' + base_file + '", HTTP_GET, [](){\n  sendProgmem(' + array_name + ', sizeof(' + array_name + '), W_JS);\n});\n'

for file in lang_files:
    q = PurePath('lang')
    compressed_lang = compressed / q
    if not os.path.exists(str(compressed_lang)):
        os.mkdir(str(compressed_lang))
    base_file = os.path.basename(str(file))
    original_file = str(file)
    new_file = str(os.path.join(str(compressed_lang), str(base_file)))
    print("[+] Compressing " + base_file + "...")
    f_in = open(original_file, encoding='UTF-8')
    content = f_in.read()
    f_in.close()
    with gzip.GzipFile(new_file + ".gz", mode='w') as fo:
        fo.write(content.encode("UTF-8"))
    f_in = open(new_file + ".gz", 'rb')
    content = f_in.read()
    f_in.close()
    array_name = base_file.replace(".", "")
    lang_name = base_file.replace(".lang", "")
    hex_formatted_content = ""
    hex_content = binascii.hexlify(content)
    hex_content = hex_content.decode("UTF-8")
    hex_content = [hex_content[i:i+2] for i in range(0, len(hex_content), 2)]
    for char in hex_content:
        hex_formatted_content += "0x" + char + ", "
    hex_formatted_content = hex_formatted_content[:-2]
    progmem_definitions += "const char " + array_name + "[] PROGMEM = {" + hex_formatted_content + "};\n"
    copy_files_function += '  if(!LittleFS.exists("/web/lang/' + base_file + '.gz") || force) progmemToSpiffs(' + array_name + ', sizeof(' + array_name + '), "/web/lang/' + base_file + '.gz");\n'
    webserver_events += 'server.on("/lang/' + base_file + '", HTTP_GET, [](){\n  sendProgmem(' + array_name + ', sizeof(' + array_name + '), W_JSON);\n});\n'
    if(len(load_lang) > 0):
        load_lang += '    else if(String(settings::getWebSettings().lang) == "'+lang_name+'") sendProgmem(' + array_name + ', sizeof(' + array_name + '), W_JSON);\n'
    else:
        load_lang += '    if(String(settings::getWebSettings().lang) == "'+lang_name+'") sendProgmem(' + array_name + ', sizeof(' + array_name + '), W_JSON);\n'

base_file = os.path.basename(license_file_path)
new_file = str(os.path.join(str(compressed), str("LICENSE")))
print("[+] Compressing " + base_file + "...")
f_in = open(license_file_path, encoding='UTF-8')
content = f_in.read()
f_in.close()
with gzip.GzipFile(new_file + ".gz", mode='w') as fo:
    fo.write(content.encode("UTF-8"))
f_in = open(new_file + ".gz", 'rb')
content = f_in.read()
f_in.close()
array_name = base_file.replace(".", "")
hex_formatted_content = ""
hex_content = binascii.hexlify(content)
hex_content = hex_content.decode("UTF-8")
hex_content = [hex_content[i:i+2] for i in range(0, len(hex_content), 2)]
for char in hex_content:
    hex_formatted_content += "0x" + char + ", "
hex_formatted_content = hex_formatted_content[:-2]
progmem_definitions += "const char " + array_name + "[] PROGMEM = {" + hex_formatted_content + "};\n"
copy_files_function += '    if(!LittleFS.exists("/web/' + base_file + '.gz") || force) progmemToSpiffs(' + array_name + ', sizeof(' + array_name + '), "/web/' + base_file + '.gz");\n'

print("[+] Saving everything into webfiles.h...")
f = open(arduino_file_path, 'w')
f.write("#ifndef webfiles_h\n")
f.write("#define webfiles_h\n")
f.write("\n")
f.write("// comment that out if you want to save program memory and know how to upload the web files to the SPIFFS manually\n")
f.write("#define USE_PROGMEM_WEB_FILES \n")
f.write("\n")
f.write("#ifdef USE_PROGMEM_WEB_FILES\n")
f.write(progmem_definitions)
f.write("#endif\n")
f.write("\n")
f.write("void copyWebFiles(bool force){\n")
f.write("#ifdef USE_PROGMEM_WEB_FILES\n")
f.write("if(settings::getWebSettings().use_spiffs){\n")
f.write(copy_files_function)
f.write("}\n")
f.write("#endif\n")
f.write("}\n")
f.write("\n")
f.write("#endif")
f.close()

print("\n[+] Done, happy uploading :)")
print("Here are the updated functions for wifi.cpp, in case you added or removed files:")
print();
print('if(!settings::getWebSettings().use_spiffs){')
print('  server.on("/", HTTP_GET, [](){')
print('  sendProgmem(indexhtml, sizeof(indexhtml), W_HTML);')
print('});')
print(webserver_events)
print('}')
print('server.on("/lang/default.lang", HTTP_GET, [](){')
print("  if(!settings::getWebSettings().use_spiffs){")
print(load_lang)
print('    else handleFileRead("/web/lang/"+String(settings::getWebSettings().lang)+".lang");')
print('  } else {')
print('    handleFileRead("/web/lang/"+String(settings::getWebSettings().lang)+".lang");')
print('  }')
print("});")
