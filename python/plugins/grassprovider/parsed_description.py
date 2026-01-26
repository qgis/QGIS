"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, List, Dict


@dataclass
class ParsedDescription:
    """
    Results of parsing a description file
    """

    GROUP_ID_REGEX = re.compile(r"^[^\s(]+")

    grass_command: Optional[str] = None
    short_description: Optional[str] = None
    name: Optional[str] = None
    display_name: Optional[str] = None
    group: Optional[str] = None
    group_id: Optional[str] = None

    ext_path: Optional[str] = None

    hardcoded_strings: list[str] = field(default_factory=list)
    param_strings: list[str] = field(default_factory=list)

    def as_dict(self) -> dict:
        """
        Returns a JSON serializable dictionary representing the parsed
        description
        """
        return {
            "name": self.name,
            "display_name": self.display_name,
            "command": self.grass_command,
            "short_description": self.short_description,
            "group": self.group,
            "group_id": self.group_id,
            "ext_path": self.ext_path,
            "hardcoded_strings": self.hardcoded_strings,
            "parameters": self.param_strings,
        }

    @staticmethod
    def from_dict(description: dict) -> "ParsedDescription":
        """
        Parses a dictionary as a description and returns the result
        """

        from qgis.PyQt.QtCore import QCoreApplication

        result = ParsedDescription()
        result.name = description.get("name")
        result.display_name = description.get("display_name")
        result.grass_command = description.get("command")
        result.short_description = QCoreApplication.translate(
            "GrassAlgorithm", description.get("short_description")
        )
        result.group = QCoreApplication.translate(
            "GrassAlgorithm", description.get("group")
        )
        result.group_id = description.get("group_id")
        result.ext_path = description.get("ext_path")
        result.hardcoded_strings = description.get("hardcoded_strings", [])
        result.param_strings = description.get("parameters", [])

        return result

    @staticmethod
    def parse_description_file(
        description_file: Path, translate: bool = True
    ) -> "ParsedDescription":
        """
        Parses a description file and returns the result
        """

        result = ParsedDescription()

        with description_file.open() as lines:
            # First line of the file is the Grass algorithm name
            line = lines.readline().strip("\n").strip()
            result.grass_command = line
            # Second line if the algorithm name in Processing
            line = lines.readline().strip("\n").strip()
            result.short_description = line
            if " - " not in line:
                result.name = result.grass_command
            else:
                result.name = line[: line.find(" ")].lower()
            if translate:
                from qgis.PyQt.QtCore import QCoreApplication

                result.short_description = QCoreApplication.translate(
                    "GrassAlgorithm", line
                )
            else:
                result.short_description = line

            result.display_name = result.name
            # Read the grass group
            line = lines.readline().strip("\n").strip()
            if translate:
                from qgis.PyQt.QtCore import QCoreApplication

                result.group = QCoreApplication.translate("GrassAlgorithm", line)
            else:
                result.group = line

            result.group_id = (
                ParsedDescription.GROUP_ID_REGEX.search(line).group(0).lower()
            )

            # Then you have parameters/output definition
            line = lines.readline().strip("\n").strip()
            while line != "":
                line = line.strip("\n").strip()
                if line.startswith("Hardcoded"):
                    result.hardcoded_strings.append(line[len("Hardcoded|") :])
                result.param_strings.append(line)
                line = lines.readline().strip("\n").strip()
        return result
