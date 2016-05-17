# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_net_distance.py
    ---------------------
    Date                 : December 2015
    Copyright            : (C) 2015 by Médéric Ribreux
    Email                : medspx at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from processing.core.parameters import getParameterFromString


def processCommand(alg):
    """ Handle data preparation for v.net.distance:
    * Integrate point layers into network vector map.
    * Make v.net.distance use those layers.
    * Delete the threshold parameter.
    * If where statement, connect to the db
    """
    paramsToDelete = []

    # Grab the threshold value for our v.net connect command
    threshold = alg.getParameterValue(u'threshold')
    if threshold:
        paramsToDelete.append(alg.getParameterFromName(u'threshold'))

    # Grab the network layer and tell to v.net.alloc to use the temp layer instead
    line_layer = alg.getParameterValue(u'input')
    if line_layer:
        line_layer = alg.exportedLayers[line_layer]

    # import the two point layers into the network
    for i, layer in enumerate([u'from', u'to']):
        # Get a temp layer name
        intLayer = alg.getTempFilename()

        # Grab the from point layer and delete this parameter (not used by v.net.distance)
        point_layer = alg.getParameterValue(layer + u'_points')
        if point_layer:
            point_layer = alg.exportedLayers[point_layer]
            paramsToDelete.append(alg.getParameterFromName(layer + u'_points'))

        # Create the v.net connect command for point layer integration
        command = u"v.net -s input={} points={} out={} op=connect threshold={} arc_layer=1 node_layer={}".format(line_layer, point_layer, intLayer, threshold, i + 2)
        alg.commands.append(command)
        line_layer = intLayer

        # Add the parameter to the algorithm
        parameter = alg.getParameterFromName(u'{}_layer'.format(layer))
        if not parameter:
            parameter = getParameterFromString(u'ParameterNumber|{0}_layer|{0} layer number|1|3|2|False'.format(layer))
            alg.addParameter(parameter)
        parameter.setValue(i + 2)

        # Make the connection with attribute table
        command = u"v.db.connect -o map={} table={} layer={}".format(line_layer, point_layer, i + 2)
        alg.commands.append(command)

    alg.setParameterValue(u'input', line_layer)

    # Delete some unnecessary parameters
    for param in paramsToDelete:
        alg.parameters.remove(param)

    alg.processCommand()

    # Bring back the parameters:
    for param in paramsToDelete:
        alg.parameters.append(param)

    # Delete from_layer and to_layer
    for word in [u'from', u'to']:
        alg.parameters.remove(alg.getParameterFromName(u'{}_layer'.format(word)))
