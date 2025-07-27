#!/opt/homebrew/bin/python3

import csv
import sys

#
# Read the file of colors
with open(sys.argv[1], 'r') as file:
    colset = set(line.strip().upper() for line in file)

nchecked = 0

for csvfn in sys.argv[2:]:
    with open(csvfn, 'r') as csvfile:
        csvr = csv.reader(csvfile)
        next(csvr, None)
        for row in csvr:
            if len(row) >= 5:
                #
                # Handle Pulseview output
                if row[4] == "RGB":
                    value = row[5].strip().replace("#","").upper()
                    nchecked = nchecked + 1
                    if len(value) > 0 and value not in colset:
                        print(f"Found {value} in {csvfn} not in the set of colors!")
            elif len(row) >= 3:
                #
                # Handle KingstVIS output
                value = row[2].strip().replace("0x", "").upper()
                nchecked = nchecked + 1
                if len(value) > 0 and value not in colset:
                    print(f"Found {value} in {csvfn} not in the set of colors!")

            
print(f"Checked {nchecked}")
