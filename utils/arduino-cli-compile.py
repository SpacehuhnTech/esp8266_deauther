#!/usr/bin/env python3

# inside esp8266_deauther/esp8266_deauther
# call this script
# python3 ../utils/arduino-cli-compile.py 2.5.0

import subprocess
import os
import sys

boards = [
    "NODEMCU",
    "WEMOS_D1_MINI",
    "HACKHELD_VEGA",
    "MALTRONICS",
    "DISPLAY_EXAMPLE_I2C",
    "DISPLAY_EXAMPLE_SPI",
    "DSTIKE_DEAUTHER_V1",
    "DSTIKE_DEAUTHER_V2",
    "DSTIKE_DEAUTHER_V3",
    "DSTIKE_DEAUTHER_V3_5",
    "DSTIKE_D_DUINO_B_V5_LED_RING",
    "DSTIKE_DEAUTHER_BOY",
    "DSTIKE_NODEMCU_07",
    "DSTIKE_NODEMCU_07_V2",
    "DSTIKE_DEAUTHER_OLED",
    "DSTIKE_DEAUTHER_OLED_V1_5_S",
    "DSTIKE_DEAUTHER_OLED_V1_5",
    "DSTIKE_DEAUTHER_OLED_V2",
    "DSTIKE_DEAUTHER_OLED_V2_5",
    "DSTIKE_DEAUTHER_OLED_V3",
    "DSTIKE_DEAUTHER_OLED_V3_5",
    "DSTIKE_DEAUTHER_OLED_V4",
    "DSTIKE_DEAUTHER_OLED_V5",
    "DSTIKE_DEAUTHER_OLED_V6",
    "DSTIKE_DEAUTHER_MOSTER",
    "DSTIKE_DEAUTHER_MOSTER_V2",
    "DSTIKE_DEAUTHER_MOSTER_V3",
    "DSTIKE_DEAUTHER_MOSTER_V4",
    "DSTIKE_DEAUTHER_MOSTER_V5",
    "DSTIKE_USB_DEAUTHER",
    "DSTIKE_USB_DEAUTHER_V2",
    "DSTIKE_DEAUTHER_WATCH",
    "DSTIKE_DEAUTHER_WATCH_V2",
    "DSTIKE_DEAUTHER_MINI",
    "DSTIKE_DEAUTHER_MINI_EVO",
    "LYASI_7W_E27_LAMP",
    "AVATAR_5W_E14_LAMP",
]

version = sys.argv[1]

folder = f"../build_{version}"
os.system(f"mkdir {folder}")

for board in boards:
    print(f"Compiling {board}...", flush=True)

    if os.path.exists(f"{folder}/esp8266_deauther_{version}_{board}.bin"):
        print("Already compiled")
        continue

    os.system(f"arduino-cli cache clean")
    command = f"arduino-cli compile --fqbn deauther:esp8266:generic --build-property \"build.extra_flags=-DESP8266 -D{board}\" --output-dir {folder}"
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    process.wait()
    os.system(
        f"mv {folder}/esp8266_deauther.ino.bin {folder}/esp8266_deauther_{version}_{board}.bin")
    print(f"OK")

os.system(f"rm {folder}/esp8266_deauther.ino.elf")
os.system(f"rm {folder}/esp8266_deauther.ino.map")

print("Finished :)")
