#!/usr/bin/env python3

import sys
import json
from urllib.request import urlopen # using urllib since it is a standard module (vs. requests)
from urllib.error import URLError
import argparse

parser = argparse.ArgumentParser(description='Determines if a pull request has a defined label')
parser.add_argument('pull_request', type=int,
                    help='pull request id')
parser.add_argument('label', type=int,
                    help='label ID')

args = parser.parse_args()

url = "https://api.github.com/repos/qgis/QGIS/pulls/{}".format(args.pull_request)

try:
    data = urlopen(url).read().decode('utf-8')
except URLError as err:
    print("URLError: ".format(err.reason))
    sys.exit(1)

obj = json.loads(data)

for label in obj['labels']:
    if label["id"] == args.label:
        print("true")
        sys.exit(0)

print("label not found")
sys.exit(1)
