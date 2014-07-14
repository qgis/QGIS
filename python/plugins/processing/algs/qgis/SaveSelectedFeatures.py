# -*- coding: utf-8 -*-

"""
***************************************************************************
    SaveSelectedFeatures.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SaveSelectedFeatures(GeoAlgorithm):
    """This is an example algorithm that takes a vector layer and
    creates a new one just with just those features of the input
    layer that are selected.

    It is meant to be used as an example of how to create your own
    Processing algorithms and explain methods and variables used
    to do it. An algorithm like this will be available in all
    Processing elements, and there is not need for additional work.

    All geoprocessingalgorithms should extend the GeoAlgorithm class.
    """

    # Constants used to refer to parameters and outputs. They will be
    # used when calling the algorithm from another algorithm, or when
    # calling from the QGIS console.

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'

    def defineCharacteristics(self):
        """Here we define the inputs and output of the algorithm, along
        with some other properties.

        This will give the algorithm its semantics, and allow to use it
        in the modeler. As a rule of thumb, do not produce anything not
        declared here. It will work fine in the toolbox, but it will
        not work in the modeler. If that's what you intend, then set
        self.showInModeler = False
        """

        # The name that the user will see in the toolbox
        self.name = 'Save selected features'

        # The branch of the toolbox under which the algorithm will
        # appear
        self.group = 'Vector general tools'

        # We add the input vector layer. It can have any kind of
        # geometry. It is a mandatory (not optional) one, hence the
        # False argument
        self.addParameter(ParameterVector(self.INPUT_LAYER, 'Input layer',
                          [ParameterVector.VECTOR_TYPE_ANY], False))

        # We add a vector layer as output
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                       'Output layer with selected features'))

    def processAlgorithm(self, progress):
        """Here is where the processing itself takes place."""

        # The first thing to do is retrieve the values of the
        # parameters entered by the user. The getParameterValue will
        # return the value with its corresponding type, strings in
        # the case of inputs and outputs
        inputFilename = self.getParameterValue(self.INPUT_LAYER)

        # The output. It will get the value of the destinatation file
        # entered by the user. If the user select "Save to temporary
        # file", when we arrive here it will already have an asigned
        # value, which will be a temporary file using the first
        # supported file format of the corresponding algorithm
        # provider.
        output = self.getOutputFromName(self.OUTPUT_LAYER)

        # input layers values are always a string with its location.
        # That string can be converted into a QGIS object
        # (a QgsVectorLayer in this case) using the
        # processing.getObject() method
        vectorLayer = dataobjects.getObjectFromUri(inputFilename)

        # And now we can process

        # First we create the output layer. To do so, we call the
        # getVectorWriter method in the Output object. That will give
        # us a VectorWriter, that we can later use to add features.
        # The destination file has its format selected based on the
        # file extension. If the selected format is not supported, the
        # first available format from the provider is used, and the
        # corresponding file extension appended.

        # vectorLayer.crs() is the layer crs. By default all resulting
        # layers are assumed to be in the same crs are the inputs, and
        # will be loaded with this assumptions when executed from the
        # toolbox. The self.crs variable has to be changed in case this
        # is not true, or in case there are no input layer from which
        # the output crs can be infered


        provider = vectorLayer.dataProvider()

        writer = output.getVectorWriter(provider.fields(),
                provider.geometryType(), vectorLayer.crs())

        # Now we take the selected features and add them to the output
        # layer
        features = vector.features(vectorLayer)
        total = len(features)
        for (i, feat) in enumerate(features):
            writer.addFeature(feat)

            # We use the progress object to communicate with the user
            progress.setPercentage(100 * i / float(total))
        del writer

        # There is nothing more to do here. We do not have to open the
        # layer that we have created. The Processing framework will
        # take care of that, or will handle it if this algorithm is
        # executed within a complex model.
