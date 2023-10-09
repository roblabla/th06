#!/usr/bin/env python

import csv
import difflib
from pathlib import Path

from generate_function_diff import generate_function_diff

def main():
    base_dir = Path(__file__).parent.parent
    config_dir = base_dir / 'config'

    implemented_csv = config_dir / 'implemented.csv'

    with open(implemented_csv) as f:
        vals = []
        for row in csv.reader(f):
            orig, reimpl = generate_function_diff(row[0])
            vals.append({
                'name': row[0],
                'diff': "\n".join(difflib.unified_diff(orig.split('\n'), reimpl.split('\n'), 'Original', 'Reimplementation', lineterm='')),
                'ratio': difflib.SequenceMatcher(None, orig, reimpl).ratio(),
            })


        print("# Report")
        print("")
        print(f"name | result")
        print(f"-------------")
        for val in vals:
            name = val['name']
            if val['ratio'] != 1:
                name = f'[{name}](#{name})'
            print(f"{val['name']} | {val['ratio'] * 100:.2f}%")


        for val in vals:
            if val['ratio'] != 1:
                print("")
                print("")
                print(f"## {val['name']}")
                print("")
                print("```diff")
                print(val['diff'])
                print("```")


if __name__ == '__main__':
    main()
