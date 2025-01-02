# The following has been generated automatically from src/core/proj/qgsellipsoidutils.h
try:
    QgsEllipsoidUtils.EllipsoidParameters.__attribute_docs__ = {'valid': 'Whether ellipsoid parameters are valid', 'semiMajor': 'Semi-major axis', 'semiMinor': 'Semi-minor axis', 'useCustomParameters': 'Whether custom parameters alone should be used (semiMajor/semiMinor only)', 'inverseFlattening': 'Inverse flattening', 'crs': 'Associated coordinate reference system'}
    QgsEllipsoidUtils.EllipsoidParameters.__doc__ = """Contains parameters for an ellipsoid."""
    QgsEllipsoidUtils.EllipsoidParameters.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsEllipsoidUtils.EllipsoidDefinition.__attribute_docs__ = {'acronym': 'authority:code for QGIS builds with proj version 6 or greater, or custom acronym for ellipsoid for earlier proj builds', 'description': 'Description of ellipsoid', 'parameters': 'Ellipsoid parameters', 'celestialBodyName': 'Name of the associated celestial body (e.g. "Earth").\n\n.. warning::\n\n   This method requires PROJ 8.1 or later. On earlier PROJ builds the string will always be empty.\n\n.. versionadded:: 3.20'}
    QgsEllipsoidUtils.EllipsoidDefinition.__doc__ = """Contains definition of an ellipsoid."""
    QgsEllipsoidUtils.EllipsoidDefinition.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsEllipsoidUtils.ellipsoidParameters = staticmethod(QgsEllipsoidUtils.ellipsoidParameters)
    QgsEllipsoidUtils.definitions = staticmethod(QgsEllipsoidUtils.definitions)
    QgsEllipsoidUtils.acronyms = staticmethod(QgsEllipsoidUtils.acronyms)
    QgsEllipsoidUtils.celestialBodies = staticmethod(QgsEllipsoidUtils.celestialBodies)
    QgsEllipsoidUtils.__group__ = ['proj']
except (NameError, AttributeError):
    pass
