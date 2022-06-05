# The following has been generated automatically from src/core/pointcloud/qgspointcloudlayer.h
# monkey patching scoped based enum
QgsPointCloudLayer.PointCloudStatisticsCalculationState.NotStarted.__doc__ = "The statistics calculation task has not been started"
QgsPointCloudLayer.PointCloudStatisticsCalculationState.Calculating.__doc__ = "The statistics calculation task is running"
QgsPointCloudLayer.PointCloudStatisticsCalculationState.Calculated.__doc__ = "The statistics calculation task is done and statistics are available"
QgsPointCloudLayer.PointCloudStatisticsCalculationState.__doc__ = 'Point cloud statistics calculation task\n\n.. versionadded:: 3.26\n\n' + '* ``NotStarted``: ' + QgsPointCloudLayer.PointCloudStatisticsCalculationState.NotStarted.__doc__ + '\n' + '* ``Calculating``: ' + QgsPointCloudLayer.PointCloudStatisticsCalculationState.Calculating.__doc__ + '\n' + '* ``Calculated``: ' + QgsPointCloudLayer.PointCloudStatisticsCalculationState.Calculated.__doc__
# --
QgsPointCloudLayer.PointCloudStatisticsCalculationState.baseClass = QgsPointCloudLayer
