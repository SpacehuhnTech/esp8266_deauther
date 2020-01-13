#/usr/bin/env python3
# This scripts downloads the last OUI manufaturer file from the Whireshark
# project and converts it to esp8266_deauther format
#
# Copyright (c) 2018 xdavidhu
# https://github.com/xdavidhu/
#

import argparse
from urllib.request import urlopen

DEFAULT_URL = "https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob_plain;f=manuf"
DEFAULT_FILENAME = "../esp8266_deauther/vendor_list.h"

macs = []
vendors = []
tempVendors = []

def padhex(s):
    return '0x' + s[2:].zfill(2)

def parse_options():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", help="Output file name for macs list", default=DEFAULT_FILENAME)
    parser.add_argument("-s", "--small", action='store_true', help="Generate small file only with most used 5000 macs")
    parser.add_argument("-u", "--url", help="Wireshark oui/manuf file url", default=DEFAULT_URL)

    opt = parser.parse_args()

    return opt

def generate_lists(url, output, small):
    global tempVendors
    global vendors
    global macs

    print(f"Downloading MAC list from '{url}'...", end='', flush=True)
    data = urlopen(url)

    print("Done")

    print("Parsing lines...", end='', flush=True)

    lines = data.readlines()

    for line in lines:
        line = line.decode()
        if line.startswith('#') or line.startswith('\n'):
            continue
        mac, short_desc, *rest = line.strip().split('\t')
        short_desc = short_desc[0:8]
        short_desc = short_desc.encode("ascii", "ignore").decode()
        mac_octects = len(mac.split(':'))
        if mac_octects == 6:
            continue
        else:
            inList = False
            for vendor in tempVendors:
                if vendor[0] == short_desc:
                    inList = True
                    vendor[1] += 1
                    break
            if not inList:
                tempVendors.append([short_desc, 1])

    print("Done")

    if small:
        print("Limiting list to the most used 5k vendors")
        tempVendors.sort(key=lambda x: x[1])
        tempVendors.reverse()
        tempVendors = tempVendors[:5000]

    for vendor in tempVendors:
        vendors.append(vendor[0])

    print("Generating arrays...", end='', flush=True)

    for line in lines:
        line = line.decode()
        if line.startswith('#') or line.startswith('\n'):
            continue
        mac, short_desc, *rest = line.strip().split('\t')
        short_desc = short_desc[0:8]
        short_desc = short_desc.encode("ascii", "ignore").decode()
        mac_octects = len(mac.split(':'))
        if mac_octects == 6:
            continue
        else:
            for vendor in vendors:
                if vendor == short_desc:
                    index = vendors.index(vendor)
                    macs.append([mac, index])

    print("Done")

    generate_files(output)

def generate_files(output):
    global tempVendors
    global vendors
    global macs

    print("Creating file string for vendors...", end='', flush=True)

    # 'vendors' list
    vendorsTxt = ""
    for vendor in vendors:
        vendor = vendor.ljust(8, '\0')
        hex_vendor = ", 0x".join("{:02x}".format(ord(c)) for c in vendor)
        line = "0x" + hex_vendor
        vendorsTxt += line + ",\n"
    vendorsTxt = vendorsTxt[:-2] + "\n"

    print("Done")
    print("Creating file string for macs...", end='', flush=True)

    # 'macs' list
    macsTxt = ""
    for mac in macs:
        macaddr = mac[0]
        vendorindex = mac[1]
        (oc1, oc2, oc3) = macaddr.split(':')
        if vendorindex > 255:
            num = vendorindex
            index_bytes = []
            while num > 0:
                byte = num % 0x100
                index_bytes.append(byte)
                num //= 0x100
            hex_index = ""
            for byte in index_bytes:
                hex_index += padhex(hex(byte)) + ", "
            hex_index = hex_index[:-2]
        else:
            hex_index = padhex(hex(vendorindex)) + ", 0x00"
        line = "0x" + oc1.upper() + ", " + "0x" + oc2.upper() + ", " + "0x" + oc3.upper() + ", " + hex_index
        macsTxt += line + ",\n"
    macsTxt = macsTxt[:-2] + "\n"

    print("Done")
    print(f"Writing strings to '{output}'...", end='', flush=True)

    # Saving to file
    if output:
        with open(output, 'w') as out_file:
            out_file.write("#pragma once\n/*\n  Based on Wireshark manufacturer database\n  source: https://www.wireshark.org/tools/oui-lookup.html\n  Wireshark is released under the GNU General Public License version 2\n*/\n\n")
            out_file.write("const static uint8_t vendor_names[] PROGMEM = {\n")
            out_file.write(vendorsTxt)
            out_file.write("};\n")
            out_file.write("const static uint8_t vendor_macs[] PROGMEM = {\n")
            out_file.write(macsTxt)
            out_file.write("};\n")
            out_file.close()

    print("Done.")

if __name__ == "__main__":
    print("Starting MAC vendor list generator")
    options = parse_options()
    generate_lists(options.url, options.output, options.small)
