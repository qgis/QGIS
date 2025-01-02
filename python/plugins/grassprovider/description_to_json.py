"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************

Parses .txt algorithm description files and builds aggregated .json
description
"""

import argparse
import json
import os
from pathlib import Path


def main(description_folder: str, output_file: str):
    from .parsed_description import ParsedDescription

    algorithms = []
    folder = Path(description_folder)
    for description_file in folder.glob("*.txt"):

        description = ParsedDescription.parse_description_file(
            description_file, translate=False
        )

        ext_path = description_file.parents[1].joinpath(
            "ext", description.name.replace(".", "_") + ".py"
        )
        if ext_path.exists():
            description.ext_path = description.name.replace(".", "_")

        algorithms.append(description.as_dict())

    Path(output_file).parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, "w", encoding="utf8") as f_out:
        f_out.write(json.dumps(algorithms, indent=2))


parser = argparse.ArgumentParser(
    description="Parses GRASS .txt algorithm "
    "description files and builds "
    "aggregated .json description"
)

parser.add_argument("input", help="Path to the description directory")
parser.add_argument("output", help="Path to the output algorithms.json file")
args = parser.parse_args()

if not os.path.isdir(args.input):
    raise ValueError(f"Input directory '{args.input}' is not a directory.")

main(args.input, args.output)
