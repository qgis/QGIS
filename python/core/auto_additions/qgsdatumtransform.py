# The following has been generated automatically from src/core/proj/qgsdatumtransform.h
try:
    QgsDatumTransform.TransformPair.__attribute_docs__ = {'sourceTransformId': 'ID for the datum transform to use when projecting from the source CRS.\n\n.. seealso:: :py:func:`QgsCoordinateTransform.datumTransformCrsInfo`', 'destinationTransformId': 'ID for the datum transform to use when projecting to the destination CRS.\n\n.. seealso:: :py:func:`QgsCoordinateTransform.datumTransformCrsInfo`'}
    QgsDatumTransform.TransformPair.__doc__ = """Contains datum transform information.

.. deprecated:: 3.40

   Not used for builds based on Proj >= 6.0."""
    QgsDatumTransform.TransformPair.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsDatumTransform.TransformInfo.__attribute_docs__ = {'datumTransformId': 'Datum transform ID', 'epsgCode': 'EPSG code for the transform, or 0 if not found in EPSG database', 'sourceCrsAuthId': 'Source CRS auth ID', 'destinationCrsAuthId': 'Destination CRS auth ID', 'sourceCrsDescription': 'Source CRS description', 'destinationCrsDescription': 'Destination CRS description', 'remarks': 'Transform remarks', 'scope': 'Scope of transform', 'preferred': 'True if transform is the preferred transform to use for the source/destination CRS combination', 'deprecated': 'True if transform is deprecated'}
    QgsDatumTransform.TransformInfo.__doc__ = """Contains datum transform information.

.. deprecated:: 3.40

   Not used on builds based on Proj >= 6.0."""
    QgsDatumTransform.TransformInfo.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsDatumTransform.GridDetails.__attribute_docs__ = {'shortName': 'Short name of transform grid', 'fullName': 'Full name of transform grid', 'packageName': 'Name of package the grid is included within', 'url': 'Url to download grid from', 'directDownload': '``True`` if direct download of grid is possible', 'openLicense': '``True`` if grid is available under an open license', 'isAvailable': '``True`` if grid is currently available for use'}
    QgsDatumTransform.GridDetails.__doc__ = """Contains information about a projection transformation grid file.

.. versionadded:: 3.8"""
    QgsDatumTransform.GridDetails.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsDatumTransform.SingleOperationDetails.__attribute_docs__ = {'scope': 'Scope of operation, from EPSG registry database', 'remarks': 'Remarks for operation, from EPSG registry database', 'areaOfUse': 'Area of use, from EPSG registry database', 'authority': 'Authority name, e.g. EPSG.', 'code': 'Authority code, e.g. "8447" (for EPSG:8447).'}
    QgsDatumTransform.SingleOperationDetails.__doc__ = """Contains information about a single coordinate operation.

.. note::

   Only used in builds based on on Proj >= 6.2

.. versionadded:: 3.10"""
    QgsDatumTransform.SingleOperationDetails.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsDatumTransform.TransformDetails.__attribute_docs__ = {'proj': 'Proj representation of transform operation', 'name': 'Display name of transform operation', 'accuracy': 'Transformation accuracy (in meters)', 'authority': 'Authority name, e.g. EPSG.\n\nThis is only available for single step coordinate operations. For multi-step operations, check\n``operationDetails`` instead.', 'code': 'Identification code, e.g. "8447" (For EPSG:8447).\n\nThis is only available for single step coordinate operations. For multi-step operations, check\n``operationDetails`` instead.', 'scope': 'Scope of operation, from EPSG registry database.\n\nThis is only available for single step coordinate operations. For multi-step operations, check\n``operationDetails`` instead.', 'remarks': 'Remarks for operation, from EPSG registry database.\n\nThis is only available for single step coordinate operations. For multi-step operations, check\n``operationDetails`` instead.', 'isAvailable': '``True`` if operation is available.\n\nIf ``False``, it likely means a transform grid is required which is not\navailable.', 'areaOfUse': 'Area of use string.\n\nThis is only available for single step coordinate operations. For multi-step operations, check\n``operationDetails`` instead.\n\n.. seealso:: :py:func:`bounds`', 'bounds': 'Valid bounds for the coordinate operation.\n\n.. seealso:: :py:func:`areaOfUse`', 'grids': 'Contains a list of transform grids used by the operation.', 'operationDetails': 'Contains information about the single operation steps used in the transform operation.\n\n.. note::\n\n   Only used in builds based on on Proj >= 6.2\n\n.. versionadded:: 3.10'}
    QgsDatumTransform.TransformDetails.__doc__ = """Contains information about a coordinate transformation operation.

.. note::

   Only used in builds based on on Proj >= 6.0

.. versionadded:: 3.8"""
    QgsDatumTransform.TransformDetails.__group__ = ['proj']
except (NameError, AttributeError):
    pass
try:
    QgsDatumTransform.operations = staticmethod(QgsDatumTransform.operations)
    QgsDatumTransform.datumTransformations = staticmethod(QgsDatumTransform.datumTransformations)
    QgsDatumTransform.datumTransformToProj = staticmethod(QgsDatumTransform.datumTransformToProj)
    QgsDatumTransform.projStringToDatumTransformId = staticmethod(QgsDatumTransform.projStringToDatumTransformId)
    QgsDatumTransform.datumTransformInfo = staticmethod(QgsDatumTransform.datumTransformInfo)
    QgsDatumTransform.__group__ = ['proj']
except (NameError, AttributeError):
    pass
