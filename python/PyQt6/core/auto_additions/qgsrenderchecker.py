# The following has been generated automatically from src/core/qgsrenderchecker.h
# monkey patching scoped based enum
QgsRenderChecker.Flag.AvoidExportingRenderedImage.__doc__ = "Avoids exporting rendered images to reports"
QgsRenderChecker.Flag.Silent.__doc__ = "Don't output non-critical messages to console \n.. versionadded:: 3.40"
QgsRenderChecker.Flag.__doc__ = """Render checker flags.

.. versionadded:: 3.28

* ``AvoidExportingRenderedImage``: Avoids exporting rendered images to reports
* ``Silent``: Don't output non-critical messages to console

  .. versionadded:: 3.40


"""
# --
QgsRenderChecker.Flag.baseClass = QgsRenderChecker
QgsRenderChecker.Flags = lambda flags=0: QgsRenderChecker.Flag(flags)
QgsRenderChecker.Flags.baseClass = QgsRenderChecker
Flags = QgsRenderChecker  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsRenderChecker.testReportDir = staticmethod(QgsRenderChecker.testReportDir)
    QgsRenderChecker.shouldGenerateReport = staticmethod(QgsRenderChecker.shouldGenerateReport)
    QgsRenderChecker.drawBackground = staticmethod(QgsRenderChecker.drawBackground)
    QgsRenderChecker.sourcePath = staticmethod(QgsRenderChecker.sourcePath)
except (NameError, AttributeError):
    pass
