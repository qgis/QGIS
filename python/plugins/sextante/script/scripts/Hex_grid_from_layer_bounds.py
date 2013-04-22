##[Example scripts]=group
##input=vector
##cellsize=number 1000.0
##grid=output vector
from sextante.core.QGisLayers import QGisLayers

input = sextante.getobject(input)
centerx = (input.extent().xMinimum() + input.extent().xMaximum()) / 2
centery = (input.extent().yMinimum() + input.extent().yMaximum()) / 2
width = (input.extent().xMaximum() - input.extent().xMinimum())
height = (input.extent().yMaximum() - input.extent().yMinimum())
sextante.runalg("qgis:creategrid", cellsize, cellsize, width, height, centerx, centery, 3, grid)
