#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
***************************************************************************
    dump_babel_formats.py
    ---------------------
    Date                 : July 2021
    Copyright            : (C) 2021 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

"""
Dumps a list of babel formats for inclusion in QgsBabelFormatRegistry::QgsBabelFormatRegistry()
"""

__author__ = 'Nyall Dawson'
__date__ = 'July 2021'
__copyright__ = '(C) 2021, Nyall Dawson'

import subprocess
import re


def process_lines(lines):
    """
    Processes lines from gpsbabel -^3 output
    """
    current_line = 0
    format_out = {}
    while current_line < len(lines):
        line = lines[current_line]

        fields = line.split('\t')
        assert len(fields) >= 5, fields

        current_line += 1
        html_pages = []
        while lines[current_line].startswith('http'):
            html_pages.append(lines[current_line])
            current_line += 1

        while current_line < len(lines) and lines[current_line].startswith('option'):
            options = lines[current_line].split('\t')
            assert len(options) >= 9, options

            name, description, option_type, option_def, option_min, option_max, option_html = options[:7]
            # print(name, description, option_type, option_def, option_min, option_max, option_html)

            option_http_pages = []
            current_line += 1
            while current_line < len(lines) and lines[current_line].startswith('http'):
                option_http_pages.append(lines[current_line])
                current_line += 1

        name = fields[2]
        description = fields[4]

        # remove odd comment from description!
        description = description.replace(' [ Get Jonathon Johnson to describe', '')

        read_waypoints = fields[1][0] == 'r'
        read_tracks = fields[1][2] == 'r'
        read_routes = fields[1][4] == 'r'
        write_waypoints = fields[1][1] == 'w'
        write_tracks = fields[1][3] == 'w'
        write_routes = fields[1][5] == 'w'
        is_file_format = fields[0] == 'file'
        is_device_format = fields[0] == 'serial'
        extensions = fields[3].split('/')

        if is_file_format and any([read_routes, read_tracks, read_waypoints]):
            capabilities = []
            if read_waypoints:
                capabilities.append('Qgis::BabelFormatCapability::Waypoints')
            if read_routes:
                capabilities.append('Qgis::BabelFormatCapability::Routes')
            if read_tracks:
                capabilities.append('Qgis::BabelFormatCapability::Tracks')
            capabilities_string = ' | '.join(capabilities)

            extensions_string = '{' + ', '.join([f'QStringLiteral( "{ext.strip()}" )' for ext in extensions if ext.strip()]) + '}'
            format_out[
                name] = f'mImporters[QStringLiteral( "{name}" )] = new QgsBabelSimpleImportFormat( QStringLiteral( "{name}" ), QStringLiteral( "{description}" ), {capabilities_string}, {extensions_string} );'

    for format_name in sorted(format_out.keys(), key=lambda x: x.lower()):
        print(format_out[format_name])


CMD = "gpsbabel -^3"
START_FORMATS_RX = re.compile("^file|serial")

result = subprocess.Popen(CMD, shell=True, stdout=subprocess.PIPE)
found_first_line = False
input_lines = []
for input_line in result.stdout.readlines():
    input_str = input_line.decode()
    if not found_first_line:
        if START_FORMATS_RX.match(input_str):
            found_first_line = True

        else:
            continue

    if input_str.strip():
        input_lines.append(input_str.strip())

process_lines(input_lines)
