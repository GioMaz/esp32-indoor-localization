import numpy as np
import random
import csv
import sys
import argparse
import matplotlib.pyplot as plt

ROOM_WIDTH = 18
ROOM_HEIGHT = 4
NUM_APS = 4

def generate_mac():
    return [random.randint(0x00, 0xFF) for _ in range(6)]

def generate_ap_positions():
    positions = set()
    while len(positions) < NUM_APS:
        x = random.randint(0, ROOM_WIDTH - 1)
        y = random.randint(0, ROOM_HEIGHT - 1)
        positions.add((x, y))
    return list(positions)

def simulate_rssi(ap_x, ap_y, x, y):
    distance = np.sqrt((x - ap_x)**2 + (y - ap_y)**2)
    return -30 - distance * 5 if distance <= 5 else -100

def format_mac(mac):
    return ''.join(f'{b:02X}' for b in mac)

def write_csv(filename, macs, ap_positions, rssi_grid):
    with open(filename, 'w', newline='') as f:
        writer = csv.writer(f)
        
        header = ['x', 'y']
        for i in range(NUM_APS):
            header.extend([f'mac{i+1}', f'rssi{i+1}'])
        writer.writerow(header)
        
        for y in range(ROOM_HEIGHT):
            for x in range(ROOM_WIDTH):
                row = [x, y]
                rssi_values = []
                for (ap_x, ap_y), mac in zip(ap_positions, macs):
                    rssi = simulate_rssi(ap_x, ap_y, x, y)
                    rssi_values.append(rssi)
                    row.extend([format_mac(mac), int(rssi)])
                rssi_grid[y, x] = np.max(rssi_values)
                writer.writerow(row)

def generate_heatmap(rssi_grid, ap_positions, output_image):
    plt.figure(figsize=(ROOM_WIDTH / 2, ROOM_HEIGHT / 2))
    plt.imshow(rssi_grid, cmap='hot', interpolation='nearest', origin='lower')

    plt.colorbar(label='Max RSSI (dBm)')
    
    plt.title('Heatmap of Max RSSI')
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.xticks(np.arange(ROOM_WIDTH))
    plt.yticks(np.arange(ROOM_HEIGHT))
    
    ap_xs = [pos[0] for pos in ap_positions]
    ap_ys = [pos[1] for pos in ap_positions]
    plt.scatter(ap_xs, ap_ys, c='blue', s=100, marker='x', label='Access Point')
    
    for i, (x, y) in enumerate(ap_positions):
        plt.text(x + 0.2, y + 0.2, f'AP{i+1}', color='blue', fontsize=8, weight='bold')

    plt.legend(loc='upper right')
    plt.tight_layout()
    plt.savefig(output_image)
    print(f"Heatmap image saved: {output_image}")


def main():
    parser = argparse.ArgumentParser(description="Generate RSSI dataset with optional heatmap.")
    parser.add_argument("output_csv", help="Output CSV filename")
    parser.add_argument("--heatmap", help="Output image filename for heatmap", default=None)
    args = parser.parse_args()
    
    macs = [generate_mac() for _ in range(NUM_APS)]
    ap_positions = generate_ap_positions()
    rssi_grid = np.zeros((ROOM_HEIGHT, ROOM_WIDTH))

    write_csv(args.output_csv, macs, ap_positions, rssi_grid)
    print(f"CSV file generated: {args.output_csv}")
    
    if args.heatmap:
        generate_heatmap(rssi_grid, ap_positions, args.heatmap)

if __name__ == "__main__":
    main()