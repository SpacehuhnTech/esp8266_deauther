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
    "DEAUTHER",
    "DEAUTHER_V1",
    "DEAUTHER_V2",
    "DEAUTHER_V3",
    "DEAUTHER_V3_5",
    "D_DUINO_B_V5_LED_RING",
    "DEAUTHER_BOY",
    "NODEMCU_07",
    "NODEMCU_07_V2",
    "DEAUTHER_OLED",
    "DEAUTHER_OLED_V1_5_S",
    "DEAUTHER_OLED_V1_5",
    "DEAUTHER_OLED_V2",
    "DEAUTHER_OLED_V2_5",
    "DEAUTHER_OLED_V3",
    "DEAUTHER_OLED_V3_5",
    "DEAUTHER_OLED_V4",
    "DEAUTHER_OLED_V5",
    "DEAUTHER_OLED_V6",
    "DEAUTHER_MOSTER",
    "DEAUTHER_MOSTER_V2",
    "DEAUTHER_MOSTER_V3",
    "DEAUTHER_MOSTER_V4",
    "DEAUTHER_MOSTER_V5",
    "USB_DEAUTHER",
    "USB_DEAUTHER_V2",
    "DEAUTHER_WRISTBAND",
    "DEAUTHER_WATCH",
    "DEAUTHER_WATCH_V2",
    "DEAUTHER_MINI",
    "DEAUTHER_MINI_EVO",
    "LYASI_7W_E27_LAMP",
    "AVATAR_5W_E14_LAMP",
]

version = sys.argv[1]

folder = f"build_{version}"
os.system(f"mkdir {folder}")

for board in boards:
    print(f"Compiling {board}...", end="", flush=True)
    os.system(f"arduino-cli cache clean");
    command = f"arduino-cli compile --fqbn deauther:esp8266:d1_mini --build-properties \"build.extra_flags=-DESP8266 -D{board}\" --output-dir {folder}"
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    process.wait()
    os.system(f"mv {folder}/esp8266_deauther.ino.bin {folder}/esp8266_deauther_{version}_{board}.bin")
    print(f"OK")

os.system("rm build/*.elf && rm build/*.map")
print("Finished :)")