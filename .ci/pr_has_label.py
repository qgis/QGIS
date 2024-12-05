#!/usr/bin/env python3

import argparse
import json
import sys

from urllib.error import URLError
from urllib.request import (  # using urllib since it is a standard module (vs. requests)
    urlopen,
)

parser = argparse.ArgumentParser(
    description="Determines if a pull request has a defined label"
)
parser.add_argument("pull_request", type=str, help="pull request id")
parser.add_argument("label", type=int, help="label ID")

args = parser.parse_args()

if args.pull_request == "false":
    print("false")
    sys.exit(1)

url = f"https://api.github.com/repos/qgis/QGIS/pulls/{args.pull_request}"

try:
    data = urlopen(url).read().decode("utf-8")
except URLError as err:
    print(f"URLError: {err.reason}")
    sys.exit(1)

obj = json.loads(data)

for label in obj["labels"]:
    if label["id"] == args.label:
        print("true")
        sys.exit(0)

print("label not found")
sys.exit(1)
