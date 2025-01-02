# The following has been generated automatically from src/analysis/vector/geometry_checker/qgsgeometrycheck.h
QgsGeometryCheck.ChangeFeature = QgsGeometryCheck.ChangeWhat.ChangeFeature
QgsGeometryCheck.ChangePart = QgsGeometryCheck.ChangeWhat.ChangePart
QgsGeometryCheck.ChangeRing = QgsGeometryCheck.ChangeWhat.ChangeRing
QgsGeometryCheck.ChangeNode = QgsGeometryCheck.ChangeWhat.ChangeNode
QgsGeometryCheck.ChangeAdded = QgsGeometryCheck.ChangeType.ChangeAdded
QgsGeometryCheck.ChangeRemoved = QgsGeometryCheck.ChangeType.ChangeRemoved
QgsGeometryCheck.ChangeChanged = QgsGeometryCheck.ChangeType.ChangeChanged
QgsGeometryCheck.FeatureNodeCheck = QgsGeometryCheck.CheckType.FeatureNodeCheck
QgsGeometryCheck.FeatureCheck = QgsGeometryCheck.CheckType.FeatureCheck
QgsGeometryCheck.LayerCheck = QgsGeometryCheck.CheckType.LayerCheck
QgsGeometryCheck.AvailableInValidation = QgsGeometryCheck.Flag.AvailableInValidation
QgsGeometryCheck.Flags = lambda flags=0: QgsGeometryCheck.Flag(flags)
QgsGeometryCheck.Flags.baseClass = QgsGeometryCheck
Flags = QgsGeometryCheck  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsGeometryCheck.Change.__attribute_docs__ = {'what': 'What level this change affects.', 'type': 'What action this change performs.', 'vidx': 'The index of the part / ring / vertex, depending on :py:func:`what`.'}
    QgsGeometryCheck.Change.__doc__ = """Descripts a change to fix a geometry.

.. versionadded:: 3.4"""
    QgsGeometryCheck.Change.__group__ = ['vector', 'geometry_checker']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryCheck.LayerFeatureIds.__doc__ = """A list of layers and feature ids for each of these layers.
In C++, the member `ids` can be accessed directly.
In Python some accessor methods will need to be written.

.. versionadded:: 3.4"""
    QgsGeometryCheck.LayerFeatureIds.__group__ = ['vector', 'geometry_checker']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryCheck.__group__ = ['vector', 'geometry_checker']
except (NameError, AttributeError):
    pass
