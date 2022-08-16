# The following has been generated automatically from src/core/qgsrenderchecker.h
# monkey patching scoped based enum
QgsRenderChecker.Flag.AvoidExportingRenderedImage.__doc__ = "Avoids exporting rendered images to reports"
QgsRenderChecker.Flag.__doc__ = 'Render checker flags.\n\n.. versionadded:: 3.28\n\n' + '* ``AvoidExportingRenderedImage``: ' + QgsRenderChecker.Flag.AvoidExportingRenderedImage.__doc__
# --
QgsRenderChecker.Flag.baseClass = QgsRenderChecker
QgsRenderChecker.Flags.baseClass = QgsRenderChecker
Flags = QgsRenderChecker  # dirty hack since SIP seems to introduce the flags in module
