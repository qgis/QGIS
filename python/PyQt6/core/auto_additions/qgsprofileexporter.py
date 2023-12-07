# The following has been generated automatically from src/core/elevation/qgsprofileexporter.h
# monkey patching scoped based enum
QgsProfileExporterTask.ExportResult.Success.__doc__ = "Successful export"
QgsProfileExporterTask.Success = QgsProfileExporterTask.ExportResult.Success
QgsProfileExporterTask.ExportResult.Empty.__doc__ = "Results were empty"
QgsProfileExporterTask.Empty = QgsProfileExporterTask.ExportResult.Empty
QgsProfileExporterTask.ExportResult.DeviceError.__doc__ = "Could not open output file device"
QgsProfileExporterTask.DeviceError = QgsProfileExporterTask.ExportResult.DeviceError
QgsProfileExporterTask.ExportResult.DxfExportFailed.__doc__ = "Generic error when outputting to DXF"
QgsProfileExporterTask.DxfExportFailed = QgsProfileExporterTask.ExportResult.DxfExportFailed
QgsProfileExporterTask.ExportResult.LayerExportFailed.__doc__ = "Generic error when outputting to files"
QgsProfileExporterTask.LayerExportFailed = QgsProfileExporterTask.ExportResult.LayerExportFailed
QgsProfileExporterTask.ExportResult.Canceled.__doc__ = "Export was canceled"
QgsProfileExporterTask.Canceled = QgsProfileExporterTask.ExportResult.Canceled
QgsProfileExporterTask.ExportResult.__doc__ = "Results of exporting the profile.\n\n" + '* ``Success``: ' + QgsProfileExporterTask.ExportResult.Success.__doc__ + '\n' + '* ``Empty``: ' + QgsProfileExporterTask.ExportResult.Empty.__doc__ + '\n' + '* ``DeviceError``: ' + QgsProfileExporterTask.ExportResult.DeviceError.__doc__ + '\n' + '* ``DxfExportFailed``: ' + QgsProfileExporterTask.ExportResult.DxfExportFailed.__doc__ + '\n' + '* ``LayerExportFailed``: ' + QgsProfileExporterTask.ExportResult.LayerExportFailed.__doc__ + '\n' + '* ``Canceled``: ' + QgsProfileExporterTask.ExportResult.Canceled.__doc__
# --
QgsProfileExporterTask.ExportResult.baseClass = QgsProfileExporterTask
