#!/usr/bin/python
#***************************************************************************
#    test_export.py
#    --------------------------------------
#   Date                 : Sun Sep 16 12:34:29 AKDT 2007
#   Copyright            : (C) 2007 by Gary E. Sherman
#   Email                : sherman at mrcc dot com
#***************************************************************************
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#***************************************************************************/


# Test script to export a sample QGIS project file to mapserver
# No template of header/footer information is included in the map file
# To change units, image type, name, and dimensions, modify the ex.setOptions line
#
import sys
import ms_export
if len(sys.argv) == 3:
  ex = ms_export.Qgis2Map(sys.argv[1], sys.argv[2])
  ex.setOptions( 'Meters', 'JPEG', 'TestMap', '800', '600', '', '', '')
  ex.writeMapFile()
else:
  print "Test script to export a QGIS project file to a MapServer map file"
  print "Specify the QGIS project file and a file name for the map file to be created:"
  print "  text_export.py my_qgis_project.qgs my_output_map.map"
