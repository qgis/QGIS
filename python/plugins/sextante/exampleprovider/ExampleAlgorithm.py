# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : July 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from sextante.core.Sextante import Sextante
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector

from sextante.outputs.OutputVector import OutputVector


class ExampleAlgorithm(GeoAlgorithm):
    '''This is an example algorithm that takes a vector layer and creates
    a new one just with just those features of the input layer that are
    selected.

    It is meant to be used as an example of how to create your own SEXTANTE
    algorithms and explain methods and variables used to do it. An algorithm
    like this will be available in all SEXTANTE elements, and there is not need
    for additional work.

    All SEXTANTE algorithms should extend the GeoAlgorithm class.
    '''

    # constants used to refer to parameters and outputs.
    # They will be used when calling the algorithm from another algorithm,
    # or when calling SEXTANTE from the QGIS console.
    OUTPUT_LAYER = "OUTPUT_LAYER"
    INPUT_LAYER = "INPUT_LAYER"

    def defineCharacteristics(self):
        '''Here we define the inputs and output of the algorithm, along
        with some other properties
        '''

        # the name that the user will see in the toolbox
        self.name = "Create copy of layer"

        # the branch of the toolbox under which the algorithm will appear
        self.group = "Algorithms for vector layers"

        # we add the input vector layer. It can have any kind of geometry
        # It is a mandatory (not optional) one, hence the False argument
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", [ParameterVector.VECTOR_TYPE_ANY], False))

        # we add a vector layer as output
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer with selected features"))

    def processAlgorithm(self, progress):
        '''Here is where the processing itself takes place'''

        # the first thing to do is retrieve the values of the parameters
        # entered by the user
        inputFilename = self.getParameterValue(self.INPUT_LAYER)
        output = self.getOutputValue(self.OUTPUT_LAYER)

        # input layers vales are always a string with its location.
        # That string can be converted into a QGIS object (a QgsVectorLayer in
        # this case) using the Sextante.getObjectFromUri() method.
        vectorLayer = QGisLayers.getObjectFromUri(inputFilename)

        # And now we can process

        # First we create the output layer. The output value entered by the user
        # is a string containing a filename, so we can use it directly
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" )
        provider = vectorLayer.dataProvider()
        writer = QgsVectorFileWriter(output,
                                     systemEncoding,
                                     provider.fields(),
                                     provider.geometryType(),
                                     provider.crs()
                                    )

        # Now we take the features from input layer and add them to the output.
        # Method features() returns an iterator, considiring the selection that
        # might exisist in layer and SEXTANTE configuration that indicates
        # should algorithm use only selected features or all of them
        features = QGisLayers.features(vectorLayer)
        for f in features:
            writer.addFeature(f)

        # There is nothing more to do here. We do not have to open the layer
        # that we have created. SEXTANTE will take care of that, or will handle
        # it if this algorithm is executed within a complex model
