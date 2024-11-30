#!/usr/bin/env python3

###########################################################################
#    doxygen_space.py
#    ---------------------
#    begin                : October 2016
#    copyright            : (C) 2016 by Nyall Dawson
#    email                : nyall dot dawson at gmail dot com
#
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################
import re
import sys


def exit_with_error(message):
    sys.exit(f"! Doxygen formatting error: {message}")


def process_file(file_path):
    with open(file_path) as file:
        input_lines = file.readlines()

    output = []
    inside_dox_block = False
    inside_dox_list = False
    previous_was_blankline = False
    previous_was_dox_blankline = False
    just_finished_a_list = False
    buffered_line = ""

    i = 0
    while i < len(input_lines):
        line = input_lines[i].rstrip()
        is_blank_line = not line.strip()

        if re.match(r"^\s*(?:#ifdef|#ifndef|#else|#endif)", line):
            output.append(line)
            i += 1
            continue

        if match := re.match(r"^(\s*)/\*[*!]\s*([^\s*].*)\s*$", line):
            # Convert blocks starting with /*! format to /** standard,
            # and convert
            # /**Some docs
            # to
            # /**
            #  * Some docs
            indent, content = match.groups()
            output.append(f"{indent}/**")
            line = f"{indent} * {content[0].upper()}{content[1:]}"

        if match := re.match(
            r"^(.*)/\*[!*](?!\*)(<*)[ \t\r\n\f]*(.*?)[ \t\r\n\f]*\*/[ \t\r\n\f]*$", line
        ):
            # Convert single line doxygen blocks:
            #  /*!< comment */   to   //!< comment
            #  /** comment */    to   //! comment
            prefix, tag, content = match.groups()
            line = f"{prefix}//!{tag} {content}"

        if match := re.match(r"^(.*)//!<\s*(.)(.*)$", line):
            # Uppercase initial character in //!< comment
            prefix, first, remaining = match.groups()
            line = f"{prefix}//!< {first.upper()}{remaining}"

        if match := re.match(r"^(.*)\\param ([\w_]+)\s+[:,.-]\s*(.*?)$", line):
            # Standardize \param
            prefix, param, suffix = match.groups()
            line = f"{prefix}\\param {param} {suffix}"

        if "//!<" in line and (
            match := re.match(
                r"^(.*)\.\s*[Ss]ince (?:QGIS )?(\d+\.\d+(?:\.\d+)?)[.]?$", line
            )
        ):
            # Use \since annotation
            prefix, version = match.groups()
            line = f"{prefix} \\since QGIS {version}"

        if "//!<" in line and (
            match := re.match(
                r"^(.*?)\s*\([Ss]ince (?:QGIS )?(\d+\.\d+(?:\.\d+)?)[.)]+$", line
            )
        ):
            # Use \since annotation
            prefix, version = match.groups()
            line = f"{prefix} \\since QGIS {version}"

        if match := re.match(r"^(.*)\\since (?:QGIS )?(\d+\.\d+(?:\.\d+)?)[.]?$", line):
            # Standard since annotation
            prefix, version = match.groups()
            line = f"{prefix}\\since QGIS {version}"

        if match := re.match(
            r"^(.*)\\deprecated[,.:]? (?:[dD]eprecated )?(?:[sS]ince )?(?:QGIS )?(\d+\.\d+(?:\.\d+)?)[,\s.\-]*(.*?)$",
            line,
        ):
            # Standardize deprecated annotation
            prefix, version, suffix = match.groups()
            if suffix:
                if suffix.startswith("("):
                    if suffix.endswith(")"):
                        suffix = suffix[1:-1]
                    elif suffix.endswith(")."):
                        suffix = suffix[1:-2]
                suffix = suffix[0].upper() + suffix[1:]
                if not suffix.endswith("."):
                    suffix += "."
                line = f"{prefix}\\deprecated QGIS {version}. {suffix}"
            else:
                line = f"{prefix}\\deprecated QGIS {version}"
        elif re.match(r"^(.*)\\deprecated", line):
            exit_with_error(
                "\\deprecated MUST be followed by the correct version number, eg 'QGIS 3.40'"
            )

        if match := re.match(r"^(\s*)//!\s*(.*?)$", line):
            indentation, comment = match.groups()
            # found a //! comment
            # check next line to see if it also begins with //!
            if i + 1 < len(input_lines) and re.match(
                r"^\s*//!\s*(.*?)$", input_lines[i + 1]
            ):
                # we are in a multiline //! comment block, convert to /** block
                if not previous_was_blankline:
                    output.append("")
                output.append(f"{indentation}/**")
                output.append(f"{indentation} * {comment}")
                while i + 1 < len(input_lines) and (
                    next_match := re.match(r"^\s*//!\s*(.*?)$", input_lines[i + 1])
                ):
                    next_comment = next_match.group(1)
                    if next_comment:
                        output.append(f"{indentation} * {next_comment}")
                    else:
                        output.append(f"{indentation} *")
                    i += 1
                output.append(f"{indentation} */")
            else:
                output.append(line)
        elif inside_dox_block:
            # replace "* abc" style doxygen lists with correct "- abc" formatting
            line = re.sub(r"^(\s+)\*\s{1,10}\*", r"\1* -", line)

            if re.match(r"^\s*\*\s*$", line):
                previous_was_dox_blankline = True
                if inside_dox_list:
                    inside_dox_list = False
                    just_finished_a_list = True
                    buffered_line = line
                    # print("end list")
                else:
                    output.append(line)
            elif match := re.match(r"^(\s*)\*\s*-(?![-\d>]) *(.*)$", line):
                indent, content = match.groups()
                if not inside_dox_list and not previous_was_dox_blankline:
                    output.append(f"{indent}*")
                if just_finished_a_list:
                    # print("just finished a list, continuing the same one!!")
                    buffered_line = ""
                # print("start list")
                output.append(f"{indent}* - {content}")
                inside_dox_list = True
                just_finished_a_list = False
            elif inside_dox_list and (match := re.match(r"^(\s*)\*\s{2,}(.*)$", line)):
                # print("list continuation")
                indent, content = match.groups()
                output.append(f"{indent}*   {content}")
            elif inside_dox_list and (match := re.match(r"^(\s*)\*(?!/)", line)):
                inside_dox_list = False
                indent = match.group(1)
                # print("end list without line break")
                output.append(f"{indent}*")
                output.append(line)
                just_finished_a_list = True
            elif re.match(r"^(\s*)\*/\s*$", line):
                inside_dox_block = False
                inside_dox_list = False
                just_finished_a_list = False
                if buffered_line:
                    output.append(buffered_line)
                    buffered_line = ""
                output.append(line)
                # print("end_block")
            else:
                if buffered_line:
                    output.append(buffered_line)
                    buffered_line = ""

                if not re.match(r"^\s*[#*]", line) and (
                    match := re.match(r"^(\s*?)(\s?)(.+?)$", line)
                ):
                    indent, space, content = match.groups()
                    line = f"{indent}* {content}"

                output.append(line)
                # print("normal dox")
                previous_was_dox_blankline = False
                just_finished_a_list = False
        elif match := re.match(r"^(\s*)/\*\*(?!\*)\s*(.*)$", line):
            indent, content = match.groups()
            # Space around doxygen start blocks (force blank line before /**)
            if not previous_was_blankline:
                output.append("")
            if content:
                # new line after /** begin block
                output.append(f"{indent}/**")
                output.append(f"{indent} * {content}")
            else:
                output.append(line)
            inside_dox_block = True
            # print("start_block")
        else:
            if buffered_line:
                output.append(buffered_line)
                buffered_line = ""
            output.append(line)

        i += 1
        previous_was_blankline = is_blank_line

    with open(file_path, "w") as file:
        file.write("\n".join(output) + "\n")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python doxygen_space.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    process_file(file_path)
