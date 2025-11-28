# The following has been generated automatically from src/analysis/vector/geometry_checker/qgsgeometrycheck.h
# monkey patching scoped based enum
QgsGeometryCheck.Result.Success.__doc__ = "Operation completed successfully"
QgsGeometryCheck.Result.Canceled.__doc__ = "User canceled calculation"
QgsGeometryCheck.Result.DuplicatedUniqueId.__doc__ = "Found duplicated unique ID value"
QgsGeometryCheck.Result.InvalidReferenceLayer.__doc__ = "Missed or invalid reference layer"
QgsGeometryCheck.Result.GeometryOverlayError.__doc__ = "Error performing geometry overlay operation"
QgsGeometryCheck.Result.__doc__ = """
.. versionadded:: 4.0

* ``Success``: Operation completed successfully
* ``Canceled``: User canceled calculation
* ``DuplicatedUniqueId``: Found duplicated unique ID value
* ``InvalidReferenceLayer``: Missed or invalid reference layer
* ``GeometryOverlayError``: Error performing geometry overlay operation

"""
# --
QgsGeometryCheck.Flags.baseClass = QgsGeometryCheck
Flags = QgsGeometryCheck  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsGeometryCheck.Change.__attribute_docs__ = {'what': 'What level this change affects.', 'type': 'What action this change performs.', 'vidx': 'The index of the part / ring / vertex, depending on :py:func:`what`.'}
    QgsGeometryCheck.Change.__annotations__ = {'what': 'QgsGeometryCheck.ChangeWhat', 'type': 'QgsGeometryCheck.ChangeType', 'vidx': 'QgsVertexId'}
    QgsGeometryCheck.Change.__doc__ = """Descripts a change to fix a geometry.

.. versionadded:: 3.4"""
    QgsGeometryCheck.Change.__group__ = ['vector', 'geometry_checker']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryCheck.__virtual_methods__ = ['prepare', 'isCompatible', 'flags', 'collectErrors', 'availableResolutionMethods', 'resolutionMethods']
    QgsGeometryCheck.__abstract_methods__ = ['compatibleGeometryTypes', 'description', 'id', 'checkType']
    QgsGeometryCheck.__group__ = ['vector', 'geometry_checker']
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
