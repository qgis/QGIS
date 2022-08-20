# -*- coding: utf-8 -*-

"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import (QgsProcessing,
                       QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSink)
from qgis import processing
from qgis.processing import alg


# Processing scripts can be written using the @alg decorator, which simplifies
# the creation of algorithms and the specification of inputs and outputs.

# 'name' is the algorithm name, which must not be localised and is used for
# identifying the algorithm.
# 'label' is the translated algorithm name, which should be used for any
# user-visible display of the algorithm name.
# 'group' is the unique ID of the group this algorithm belongs to. This string
# must not be localised.
# 'group_label' is the name of the group this algorithm belongs to. This string
# should be localised.
# Use alg.tr() to get a translatable string.
@alg(name='myscriptdecorators', label=alg.tr('My Script (with decorators)'),
     group='examplescripts', group_label=alg.tr('Example scripts'))
# 'INPUT' is the recommended name for the main input parameter
@alg.input(type=alg.SOURCE, name='INPUT', label='Input layer')
# 'OUTPUT' is the recommended name for the main output parameter
@alg.input(type=alg.SINK, name='OUTPUT', label='Output layer')
# help() returns a localised short helper string for the algorithm. This string
# should provide a basic description about what the algorithm does and the
# parameters and outputs associated with it.
@alg.help(alg.tr('Example algorithm short description'))
def myAlgorithm(instance, parameters, context, feedback, inputs):
    """
    Here is where the processing itself takes place.
    """

    # Retrieve the feature source and sink. The 'dest_id' variable is used
    # to uniquely identify the feature sink, and must be included in the
    # dictionary returned by the processAlgorithm function.
    source = instance.parameterAsSource(
        parameters,
        'INPUT',
        context
    )

    # If source was not found, throw an exception to indicate that the algorithm
    # encountered a fatal error. The exception text can be any string, but in this
    # case we use the pre-built invalidSourceError method to return a standard
    # helper text for when a source cannot be evaluated
    if source is None:
        raise QgsProcessingException(instance.invalidSourceError(parameters,
                                                                 'INPUT'))

    (sink, dest_id) = instance.parameterAsSink(
        parameters,
        'OUTPUT',
        context,
        source.fields(),
        source.wkbType(),
        source.sourceCrs()
    )

    # Send some information to the user
    feedback.pushInfo('CRS is {}'.format(source.sourceCrs().authid()))

    # If sink was not created, throw an exception to indicate that the algorithm
    # encountered a fatal error. The exception text can be any string, but in this
    # case we use the pre-built invalidSinkError method to return a standard
    # helper text for when a sink cannot be evaluated
    if sink is None:
        raise QgsProcessingException(instance.invalidSinkError(parameters,
                                                               'OUTPUT'))

    # Compute the number of steps to display within the progress bar and
    # get features from source
    total = 100.0 / source.featureCount() if source.featureCount() else 0
    features = source.getFeatures()

    for current, feature in enumerate(features):
        # Stop the algorithm if cancel button has been clicked
        if feedback.isCanceled():
            break

        # Add a feature in the sink
        sink.addFeature(feature, QgsFeatureSink.FastInsert)

        # Update the progress bar
        feedback.setProgress(int(current * total))

    # To run another Processing algorithm as part of this algorithm, you can use
    # processing.run(...). Make sure you pass the current context and feedback
    # to processing.run to ensure that all temporary layer outputs are available
    # to the executed algorithm, and that the executed algorithm can send feedback
    # reports to the user (and correctly handle cancellation and progress reports!)
    if False:
        buffered_layer = processing.run("native:buffer", {
            'INPUT': dest_id,
            'DISTANCE': 1.5,
            'SEGMENTS': 5,
            'END_CAP_STYLE': 0,
            'JOIN_STYLE': 0,
            'MITER_LIMIT': 2,
            'DISSOLVE': False,
            'OUTPUT': 'memory:'
        }, context=context, feedback=feedback)['OUTPUT']

    # Return the results of the algorithm. In this case our only result is
    # the feature sink which contains the processed features, but some
    # algorithms may return multiple feature sinks, calculated numeric
    # statistics, etc. These should all be included in the returned
    # dictionary, with keys matching the feature corresponding parameter
    # or output names.
    return {'OUTPUT': dest_id}
