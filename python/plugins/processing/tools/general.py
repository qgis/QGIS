"""
***************************************************************************
    general.py
    ---------------------
    Date                 : April 2013
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

__author__ = "Victor Olaya"
__date__ = "April 2013"
__copyright__ = "(C) 2013, Victor Olaya"

from qgis.core import (
    QgsApplication,
    QgsProcessingAlgorithm,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterEnum,
    QgsProcessingParameterFeatureSink,
    QgsProcessingParameterVectorDestination,
    QgsProcessingParameterRasterDestination,
    QgsProcessingOutputLayerDefinition,
    QgsProject,
)
from processing.core.Processing import Processing
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.gui.AlgorithmDialog import AlgorithmDialog
from qgis.utils import iface


# changing this signature? make sure you update the signature in
# python/processing/__init__.py too!
# Docstring for this function is in python/processing/__init__.py
def algorithmHelp(id: str) -> None:
    alg = QgsApplication.processingRegistry().algorithmById(id)
    if alg is not None:
        print(f"{alg.displayName()} ({alg.id()})\n")
        if alg.shortDescription():
            print(alg.shortDescription() + "\n")
        if alg.shortHelpString():
            print(alg.shortHelpString() + "\n")
        print("\n----------------")
        print("Input parameters")
        print("----------------")
        for p in alg.parameterDefinitions():
            if p.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden:
                continue
            print(f"\n{p.name()}: {p.description()}")
            if p.help():
                print(f"\n\t{p.help()}")

            print(f"\n\tParameter type:\t{p.__class__.__name__}")

            if isinstance(p, QgsProcessingParameterEnum):
                opts = []
                for i, o in enumerate(p.options()):
                    opts.append(f"\t\t- {i}: {o}")
                print("\n\tAvailable values:\n{}".format("\n".join(opts)))

            parameter_type = QgsApplication.processingRegistry().parameterType(p.type())
            accepted_types = (
                parameter_type.acceptedPythonTypes()
                if parameter_type is not None
                else []
            )
            if accepted_types:
                opts = []
                for t in accepted_types:
                    opts.append(f"\t\t- {t}")
                print("\n\tAccepted data types:")
                print("\n".join(opts))

        print("\n----------------")
        print("Outputs")
        print("----------------")

        for o in alg.outputDefinitions():
            print(f"\n{o.name()}:  <{o.__class__.__name__}>")
            if o.description():
                print("\t" + o.description())

    else:
        print(f'Algorithm "{id}" not found.')


# changing this signature? make sure you update the signature in
# python/processing/__init__.py too!
# Docstring for this function is in python/processing/__init__.py
def run(
    algOrName,
    parameters,
    onFinish=None,
    feedback=None,
    context=None,
    is_child_algorithm=False,
):
    if onFinish or not is_child_algorithm:
        return Processing.runAlgorithm(
            algOrName, parameters, onFinish, feedback, context
        )
    else:
        # for child algorithms, we disable to default post-processing step where layer ownership
        # is transferred from the context to the caller. In this case, we NEED the ownership to remain
        # with the context, so that further steps in the algorithm have guaranteed access to the layer.
        def post_process(_alg, _context, _feedback):
            return

        return Processing.runAlgorithm(
            algOrName,
            parameters,
            onFinish=post_process,
            feedback=feedback,
            context=context,
        )


# changing this signature? make sure you update the signature in
# python/processing/__init__.py too!
# Docstring for this function is in python/processing/__init__.py
def runAndLoadResults(algOrName, parameters, feedback=None, context=None):
    if isinstance(algOrName, QgsProcessingAlgorithm):
        alg = algOrName
    else:
        alg = QgsApplication.processingRegistry().createAlgorithmById(algOrName)

    # output destination parameters to point to current project
    for param in alg.parameterDefinitions():
        if not param.name() in parameters:
            continue

        if isinstance(
            param,
            (
                QgsProcessingParameterFeatureSink,
                QgsProcessingParameterVectorDestination,
                QgsProcessingParameterRasterDestination,
            ),
        ):
            p = parameters[param.name()]
            if not isinstance(p, QgsProcessingOutputLayerDefinition):
                parameters[param.name()] = QgsProcessingOutputLayerDefinition(
                    p, QgsProject.instance()
                )
            else:
                p.destinationProject = QgsProject.instance()
                parameters[param.name()] = p

    return Processing.runAlgorithm(
        alg,
        parameters=parameters,
        onFinish=handleAlgorithmResults,
        feedback=feedback,
        context=context,
    )


# changing this signature? make sure you update the signature in
# python/processing/__init__.py too!
# Docstring for this function is in python/processing/__init__.py
def createAlgorithmDialog(algOrName, parameters={}):
    if isinstance(algOrName, QgsProcessingAlgorithm):
        alg = algOrName.create()
    else:
        alg = QgsApplication.processingRegistry().createAlgorithmById(algOrName)

    if alg is None:
        return None

    dlg = alg.createCustomParametersWidget(iface.mainWindow())

    if not dlg:
        dlg = AlgorithmDialog(alg, parent=iface.mainWindow())

    dlg.setParameters(parameters)

    return dlg


# changing this signature? make sure you update the signature in
# python/processing/__init__.py too!
# Docstring for this function is in python/processing/__init__.py
def execAlgorithmDialog(algOrName, parameters={}):
    dlg = createAlgorithmDialog(algOrName, parameters)
    if dlg is None:
        return {}

    canvas = iface.mapCanvas()
    prevMapTool = canvas.mapTool()
    dlg.show()
    dlg.exec()
    if canvas.mapTool() != prevMapTool:
        try:
            canvas.mapTool().reset()
        except:
            pass
        canvas.setMapTool(prevMapTool)

    results = dlg.results()
    # make sure the dialog is destroyed and not only hidden on pressing Esc
    dlg.close()
    return results
