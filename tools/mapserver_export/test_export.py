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


# test script to export a sample QGIS project file to mapserver
import ms_export
ex = ms_export.Qgis2Map('./test1.qgs', './test1.map')
ex.setOptions( 'Meters', 'JPEG', 'TestMap', '800', '600', '', '', '')

ex.writeMapFile()
