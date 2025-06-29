import numpy as np
import random
import csv
import sys

ROOM_WIDTH = 18
ROOM_HEIGHT = 4
NUM_APS = 4

AP_POSITIONS = [(10, 3), (15, 0), (0, 2), (4, 1)]

def generate_mac():
    return [random.randint(0x00, 0xFF) for _ in range(6)]

def simulate_rssi(ap_x, ap_y, x, y):
    distance = np.sqrt((x - ap_x)**2 + (y - ap_y)**2)
    return -30 - distance * 5 if distance <= 5 else -100

def format_mac(mac):
    return ''.join(f'{b:02X}' for b in mac)

def write_csv(filename, macs):
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f)
        
        header = ['x', 'y']
        for i in range(NUM_APS):
            header.extend([f'mac{i+1}', f'rssi{i+1}'])
        writer.writerow(header)
        
        for y in range(ROOM_HEIGHT):
            for x in range(ROOM_WIDTH):
                row = [x, y]
                for (ap_x, ap_y), mac in zip(AP_POSITIONS, macs):
                    rssi = simulate_rssi(ap_x, ap_y, x, y)
                    row.extend([format_mac(mac), int(rssi)])
                writer.writerow(row)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 generate.py <output_file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    macs = [generate_mac() for _ in range(NUM_APS)]
    
    write_csv(filename, macs)
    
    print(f"CSV file generated: {filename}")

if __name__ == "__main__":
    main()