from subprocess import Popen, PIPE

repo_dir = 'C:/AtmosicSDK/5.3.0/platform/atm2/ATM22xx-x1x/examples/lunch_beacon'

# Open the file containing the LunchTrak ID
with open(f'{repo_dir}/program/last_lunchtrak_id.txt', 'r') as f:
    lunchtrak_id = f.readline().strip()

# Convert the LunchTrak ID to an integer
id_as_int = int(lunchtrak_id.replace(' ', ''), 16)

# Increment the integer by 1
id_as_int += 1

# Convert the integer back to hex
new_lunchtrak_id = format(id_as_int, '02x')

# Program LunchTrak Tag with LSB byte
p = Popen(['make', 'run_all', f'LUNCHTRAK_ID="{new_lunchtrak_id}"'], cwd='C:/AtmosicSDK/5.3.0/platform/atm2/ATM22xx-x1x/examples/lunch_beacon')
p.wait()

# Write the new MAC address to a file
with open(f'{repo_dir}/program/last_lunchtrak_id.txt', 'w') as f:
    f.write(new_lunchtrak_id)

print(f'Done! Program LunchTrak Beacon with LUNCHTRAK_ID="{new_lunchtrak_id}"')