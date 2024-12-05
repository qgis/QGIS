# The following has been generated automatically from src/core/pointcloud/qgspointcloudlayer.h
# monkey patching scoped based enum
QgsPointCloudLayer.PointCloudStatisticsCalculationState.NotStarted.__doc__ = "The statistics calculation task has not been started"
QgsPointCloudLayer.PointCloudStatisticsCalculationState.Calculating.__doc__ = "The statistics calculation task is running"
QgsPointCloudLayer.PointCloudStatisticsCalculationState.Calculated.__doc__ = "The statistics calculation task is done and statistics are available"
QgsPointCloudLayer.PointCloudStatisticsCalculationState.__doc__ = """Point cloud statistics calculation task

.. versionadded:: 3.26

* ``NotStarted``: The statistics calculation task has not been started
* ``Calculating``: The statistics calculation task is running
* ``Calculated``: The statistics calculation task is done and statistics are available

"""
# --
QgsPointCloudLayer.PointCloudStatisticsCalculationState.baseClass = QgsPointCloudLayer
try:
    QgsPointCloudLayer.LayerOptions.__attribute_docs__ = {'transformContext': 'Coordinate transform context', 'loadDefaultStyle': 'Set to ``True`` if the default layer style should be loaded', 'skipCrsValidation': "Controls whether the layer is allowed to have an invalid/unknown CRS.\n\nIf ``True``, then no validation will be performed on the layer's CRS and the layer\nlayer's :py:func:`~QgsPointCloudLayer.crs` may be :py:func:`~QgsPointCloudLayer.invalid` (i.e. the layer will have no georeferencing available\nand will be treated as having purely numerical coordinates).\n\nIf ``False`` (the default), the layer's CRS will be validated using :py:func:`QgsCoordinateReferenceSystem.validate()`,\nwhich may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the\nlayer.", 'skipIndexGeneration': 'Set to ``True`` if point cloud index generation should be skipped.', 'skipStatisticsCalculation': 'Set to true if the statistics calculation for this point cloud is disabled\n\n.. versionadded:: 3.26'}
    QgsPointCloudLayer.LayerOptions.__doc__ = """Setting options for loading point cloud layers."""
    QgsPointCloudLayer.LayerOptions.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudLayer.__attribute_docs__ = {'subsetStringChanged': "Emitted when the layer's subset string has changed.\n\n.. versionadded:: 3.26\n", 'raiseError': 'Signals an error related to this point cloud layer.\n\n.. versionadded:: 3.26\n', 'statisticsCalculationStateChanged': 'Emitted when statistics calculation state has changed\n\n.. versionadded:: 3.26\n'}
    QgsPointCloudLayer.__signal_arguments__ = {'raiseError': ['msg: str'], 'statisticsCalculationStateChanged': ['state: QgsPointCloudLayer.PointCloudStatisticsCalculationState']}
    QgsPointCloudLayer.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
