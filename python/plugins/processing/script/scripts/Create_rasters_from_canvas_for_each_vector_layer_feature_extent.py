##[Example scripts]=group
##Create rasters from canvas for each vector feature extent=name
##vector_layer=vector
##scale=number 250000.0
##dpi=number 96
##image_width_mm=number 0
##image_height_mm=number 0
##output_directory=folder
##outputDir=output string

from qgis.core import *
from qgis.gui import *
from qgis.utils import iface
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os

# Loop though grid layerfeatures
layer = processing.getObject(vector_layer)
provider = layer.dataProvider()

# Get map renderer
mapRenderer = iface.mapCanvas().mapRenderer()

# Create a new composition
c = QgsComposition(mapRenderer)
c.setPrintResolution(dpi)

# Add a composer map object
x, y = 0, 0
composerMap = QgsComposerMap(c, x, y, image_width_mm, image_height_mm)
c.addComposerMap(composerMap)
composerMap.setBackgroundEnabled(False)

# World file
c.setWorldFileMap(composerMap)
c.setGenerateWorldFile( True )

# Get vector layer features
feats = processing.features(layer)
nFeat = len(feats)
i = 0
progress.setText(u'%s tiles to be generated' % nFeat)

# Iterate over the features
for feat in feats:
    # Log progression
    progress.setPercentage(int(100 * i / nFeat))
    i+=1

    # Get the feature bouding box
    geom = feat.geometry()
    rect = geom.boundingBox()

    # Recalculate paper width and height if not given
    if not image_width_mm and scale > 0:
        image_width_mm = rect.width() * 1000 / scale
    if not image_height_mm and scale > 0:
        image_height_mm = rect.height() * 1000 / scale

    # Calculate image size in pixel
    inch2mm = 25.4
    image_width_pixel = int(image_width_mm * dpi / inch2mm)
    image_height_pixel = int(image_height_mm * dpi / inch2mm)
    progress.setText(u'Feature %s - Image width : %s * %s mm / %s * %s pixels' % (
            i,
            image_width_mm,
            image_height_mm,
            image_width_pixel,
            image_height_pixel
        )
    )

    # Set paper and composerMap width and height
    c.setPaperSize(image_width_mm, image_height_mm)
    composerMap.setItemPosition(x, y, image_width_mm, image_height_mm)

    # Set the map extent and scale
    composerMap.setNewExtent(rect)
    if scale > 0:
        composerMap.setNewScale(scale)

    # Image destination path
    outputPath= "tile_%s_%s" % (scale, i)
    outputImagePath = os.path.join(output_directory, outputPath + '.png')

    # Generate image from composition
    myImage = c.printPageAsRaster(0)
    myImage.save(outputImagePath)

    # Generate World file
    wf = c.computeWorldFileParameters()
    outputWorldPath = os.path.join(output_directory, outputPath + '.pgw')
    with open(outputWorldPath, 'w') as f:
        f.write('%s\n' % wf[0])
        f.write('%s\n' % wf[1])
        f.write('%s\n' % wf[3])
        f.write('%s\n' % wf[4])
        f.write('%s\n' % wf[2])
        f.write('%s\n' % wf[5])

# export chosen output directory as a output variable
outputDir = output_directory
