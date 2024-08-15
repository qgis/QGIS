# The following has been generated automatically from src/core/elevation/qgsabstractprofilegenerator.h
try:
    QgsProfileIdentifyContext.__attribute_docs__ = {'maximumSurfaceDistanceDelta': 'Maximum allowed snapping delta for the distance values when identifying a continuous elevation surface', 'maximumSurfaceElevationDelta': 'Maximum allowed snapping delta for the elevation values when identifying a continuous elevation surface', 'maximumPointDistanceDelta': 'Maximum allowed snapping delta for the distance values when identifying a point', 'maximumPointElevationDelta': 'Maximum allowed snapping delta for the elevation values when identifying a point', 'displayRatioElevationVsDistance': 'Display ratio of elevation vs distance units', 'project': 'Associated project.'}
except NameError:
    pass
try:
    QgsAbstractProfileResults.__attribute_docs__ = {'layerIdentifier': 'Identifier for grouping output features', 'geometry': 'Exported geometry', 'attributes': 'Exported attributes'}
except NameError:
    pass
QgsAbstractProfileResults.Feature.__doc__ = """Encapsulates information about a feature exported from the profile results.

.. versionadded:: 3.32"""
try:
    QgsProfileRenderContext.__group__ = ['elevation']
except NameError:
    pass
try:
    QgsProfileIdentifyContext.__group__ = ['elevation']
except NameError:
    pass
try:
    QgsProfileIdentifyResults.__group__ = ['elevation']
except NameError:
    pass
try:
    QgsAbstractProfileResults.__group__ = ['elevation']
except NameError:
    pass
try:
    QgsAbstractProfileResults.Feature.__group__ = ['elevation']
except NameError:
    pass
try:
    QgsProfileGenerationContext.__group__ = ['elevation']
except NameError:
    pass
try:
    QgsAbstractProfileGenerator.__group__ = ['elevation']
except NameError:
    pass
