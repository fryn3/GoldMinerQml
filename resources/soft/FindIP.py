import pymiwifi
import json
import sys

psw = sys.argv[1]
# psw = 'VideoServer'
try:
    miwifi = pymiwifi.MiWiFi(address="http://miwifi.com/")
    miwifi.login(psw)
except json.decoder.JSONDecodeError:
    miwifi = pymiwifi.MiWiFi(address="http://router.miwifi.com/")
    miwifi.login(psw)

dev = miwifi.device_list()
for i, val in enumerate(dev['list']):
    # print(val)
    print(f'ip: {val["ip"][0]["ip"]} MAC: {val["mac"]} oName: {val["oname"]}')

print(f'ip: 192.168.28.68 MAC: AA:AA:AA:AA:AA:01 oName: lwip0')



