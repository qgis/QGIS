#!/usr/bin/python

# test script to export a sample QGIS project file to mapserver
import ms_export
ex = ms_export.Qgis2Map('./test1.qgs', './test1.map')
ex.setOptions( 'Meters', 'JPEG', 'TestMap', '800', '600', '', '', '')

ex.writeMapFile()
