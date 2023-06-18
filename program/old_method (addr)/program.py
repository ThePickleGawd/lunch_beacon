from subprocess import Popen, PIPE

repo_dir = 'C:/AtmosicSDK/5.3.0/platform/atm2/ATM22xx-x1x/examples/lunch_beacon'

# Open the file containing the Bluetooth MAC address
with open(f'{repo_dir}/program/last_bd_addr.txt', 'r') as f:
    mac_address = f.readline().strip()

# Convert the MAC address to an integer
mac_int = int(mac_address.replace(' ', ''), 16)

# Increment the integer by 1
mac_int += 1

# Convert the integer back to a MAC address string in hex format with spaces
new_mac_address = ' '.join([format((mac_int >> i) & 0xff, '02x') for i in range(40, -1, -8)])

# Program LunchTrak Tag with LSB byte
lsb_mac_address = ' '.join([format((mac_int >> i) & 0xff, '02x') for i in range(40, -1, -8)][::-1])
p = Popen(['make', 'run_all', f'USER_BD_ADDR="{lsb_mac_address}"'], cwd='C:/AtmosicSDK/5.3.0/platform/atm2/ATM22xx-x1x/examples/lunch_beacon')
p.wait()

# Write the new MAC address to a file
with open(f'{repo_dir}/program/last_bd_addr.txt', 'w') as f:
    f.write(new_mac_address)

print(f'Done! Program LunchTrak Beacon with BD_ADDR="{new_mac_address}"')