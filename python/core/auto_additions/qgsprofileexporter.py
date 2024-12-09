# The following has been generated automatically from src/core/elevation/qgsprofileexporter.h
# monkey patching scoped based enum
QgsProfileExporterTask.ExportResult.Success.__doc__ = "Successful export"
QgsProfileExporterTask.ExportResult.Empty.__doc__ = "Results were empty"
QgsProfileExporterTask.ExportResult.DeviceError.__doc__ = "Could not open output file device"
QgsProfileExporterTask.ExportResult.DxfExportFailed.__doc__ = "Generic error when outputting to DXF"
QgsProfileExporterTask.ExportResult.LayerExportFailed.__doc__ = "Generic error when outputting to files"
QgsProfileExporterTask.ExportResult.Canceled.__doc__ = "Export was canceled"
QgsProfileExporterTask.ExportResult.__doc__ = """Results of exporting the profile.

* ``Success``: Successful export
* ``Empty``: Results were empty
* ``DeviceError``: Could not open output file device
* ``DxfExportFailed``: Generic error when outputting to DXF
* ``LayerExportFailed``: Generic error when outputting to files
* ``Canceled``: Export was canceled

"""
# --
QgsProfileExporterTask.ExportResult.baseClass = QgsProfileExporterTask
try:
    QgsProfileExporter.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsProfileExporterTask.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
