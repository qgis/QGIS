# The following has been generated automatically from src/core/providers/arcgis/qgsesriarcadeconverter.h
# monkey patching scoped based enum
QgsEsriArcadeConverter.Result.Success.__doc__ = "Script was successfully converted"
QgsEsriArcadeConverter.Result.ParserError.__doc__ = "Script could not be parsed"
QgsEsriArcadeConverter.Result.InvalidScriptMultiStatements.__doc__ = "Scripts containing multiple statements cannot be converted"
QgsEsriArcadeConverter.Result.InvalidScriptIncompatible.__doc__ = "Script is incompatible with conversion to a QGIS expression"
QgsEsriArcadeConverter.Result.UnknownError.__doc__ = "An unknown error occurred while converting the script"
QgsEsriArcadeConverter.Result.__doc__ = """Results of converting the Arcade script.

* ``Success``: Script was successfully converted
* ``ParserError``: Script could not be parsed
* ``InvalidScriptMultiStatements``: Scripts containing multiple statements cannot be converted
* ``InvalidScriptIncompatible``: Script is incompatible with conversion to a QGIS expression
* ``UnknownError``: An unknown error occurred while converting the script

"""
# --
try:
    QgsEsriArcadeConverter.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
