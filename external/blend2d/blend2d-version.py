#!/usr/bin/env python3

import os
import re

BLEND2D_DIR = os.path.dirname(os.path.abspath(__file__))

def read_text_file(file_name):
  with open(file_name, "r", encoding="utf-8") as f:
    return f.read()

def get_version():
  try:
    api_h = read_text_file(os.path.join(BLEND2D_DIR, "blend2d", "core", "api.h"))
    ver = re.search("#define BL_VERSION BL_MAKE_VERSION\\((\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\)", api_h)
    if ver:
        return "{}.{}.{}".format(ver[1], ver[2], ver[3])
  except:
    pass
  return "unknown"

if __name__ == "__main__":
  print(get_version())
