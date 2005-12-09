#!/usr/bin/python

# test script to export a sample QGIS project file to mapserver
import ms_export
ex = ms_export.Qgis2Map('./lakes.qgs', './lakes.map')
ex.setOptions( 'Meters', 'PNG', 'TestMap', '800', '600', '','','template', 'header', 'footer')

ex.writeMapFile()
