# The following has been generated automatically from src/core/elevation/qgsprofileexporter.h
# monkey patching scoped based enum
QgsProfileExporterTask.ExportResult.Success.__doc__ = "Successful export"
QgsProfileExporterTask.ExportResult.Empty.__doc__ = "Results were empty"
QgsProfileExporterTask.ExportResult.DeviceError.__doc__ = "Could not open output file device"
QgsProfileExporterTask.ExportResult.DxfExportFailed.__doc__ = "Generic error when outputting to DXF"
QgsProfileExporterTask.ExportResult.LayerExportFailed.__doc__ = "Generic error when outputting to files"
QgsProfileExporterTask.ExportResult.Canceled.__doc__ = "Export was canceled"
QgsProfileExporterTask.ExportResult.__doc__ = 'Results of exporting the profile.\n\n' + '* ``Success``: ' + QgsProfileExporterTask.ExportResult.Success.__doc__ + '\n' + '* ``Empty``: ' + QgsProfileExporterTask.ExportResult.Empty.__doc__ + '\n' + '* ``DeviceError``: ' + QgsProfileExporterTask.ExportResult.DeviceError.__doc__ + '\n' + '* ``DxfExportFailed``: ' + QgsProfileExporterTask.ExportResult.DxfExportFailed.__doc__ + '\n' + '* ``LayerExportFailed``: ' + QgsProfileExporterTask.ExportResult.LayerExportFailed.__doc__ + '\n' + '* ``Canceled``: ' + QgsProfileExporterTask.ExportResult.Canceled.__doc__
# --
QgsProfileExporterTask.ExportResult.baseClass = QgsProfileExporterTask
