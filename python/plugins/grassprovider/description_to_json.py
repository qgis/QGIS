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

import json

from grassprovider.Grass7Utils import (
    Grass7Utils,
    ParsedDescription
)


base_description_folders = [f for f in Grass7Utils.grassDescriptionFolders()
    if f != Grass7Utils.userDescriptionFolder()]

for folder in base_description_folders:
    algorithms = []
    for description_file in folder.glob('*.txt'):
        description = ParsedDescription.parse_description_file(
            description_file)
        algorithms.append(description.as_dict())

    with open(folder / 'algorithms.json', 'wt', encoding='utf8') as f_out:
        f_out.write(json.dumps(algorithms, indent=2))
