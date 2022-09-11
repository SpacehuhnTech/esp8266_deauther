#/usr/bin/env python
"""This scripts downloads the last OUI manufaturer file from the Whireshark
project and converts it to esp8266_deauther format"""

import argparse
from urllib.request import urlopen

WS_MANUF_FILE_URL = "https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob_plain;f=manuf"

def parse_options():
    """Parses command line options"""

    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", help="Output file name (e.g oui.h)")
    parser.add_argument("-u", "--url", help="Wireshark oui/manuf file url")

    opt = parser.parse_args()

    return opt

def generate_oui_h(url, filename):
    """Generates the vendors/oui file"""

    if url:
        data = urlopen(url)
    else:
        data = urlopen(WS_MANUF_FILE_URL)

    out = """#ifndef oui_h
#define oui_h
/*
  Based on Wireshark manufacturer database
  source: https://www.wireshark.org/tools/oui-lookup.html
  Wireshark is released under the GNU General Public License version 2
*/

const static uint8_t data_vendors[] PROGMEM = {///*
"""

    for line in data:
        line = line.decode()

        # Skipping empty lines and comments
        if line.startswith('#') or line.startswith('\n'):
            continue

        mac, short_desc, *rest = line.strip().split('\t')

        # Limiting short_desc to 8 chars
        short_desc = short_desc[0:8]

        # Convert to ascii
        short_desc = short_desc.encode("ascii", "ignore").decode()

        mac_octects = len(mac.split(':'))
        if mac_octects == 6:
            continue
        else:
            # Convert to esp8266_deauther format
            short_desc = short_desc.ljust(8, '\0')
            hex_sdesc = ", 0x".join("{:02x}".format(ord(c)) for c in short_desc)

            (oc1, oc2, oc3) = mac.split(':')

            out = out + ("  0x{}, 0x{}, 0x{}, 0x{},\n".format(oc1.upper(), oc2.upper(), oc3.upper(),
                                                              hex_sdesc.upper().replace('X', 'x')))

    out = out[:-2] # Removing last comma
    out = out + "\n};\n#endif"

    # Saving to file
    if filename:
        with open(filename, 'w') as out_file:
            out_file.write("%s" % out)
    else:
        print(out)

if __name__ == "__main__":
    options = parse_options()
    generate_oui_h(options.url, options.output)

