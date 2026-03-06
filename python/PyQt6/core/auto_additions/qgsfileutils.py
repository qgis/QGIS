# The following has been generated automatically from src/core/qgsfileutils.h
# monkey patching scoped based enum
QgsFileUtils.CopyFlag.NoSymLinks.__doc__ = "If present, indicates that symbolic links should be skipped during the copy"
QgsFileUtils.CopyFlag.__doc__ = """Flags controlling behavior of file copy operations.

.. versionadded:: 4.0.1

* ``NoSymLinks``: If present, indicates that symbolic links should be skipped during the copy

"""
# --
QgsFileUtils.CopyFlag.baseClass = QgsFileUtils
QgsFileUtils.CopyFlags = lambda flags=0: QgsFileUtils.CopyFlag(flags)
QgsFileUtils.CopyFlags.baseClass = QgsFileUtils
CopyFlags = QgsFileUtils  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsFileUtils.representFileSize = staticmethod(QgsFileUtils.representFileSize)
    QgsFileUtils.extensionsFromFilter = staticmethod(QgsFileUtils.extensionsFromFilter)
    QgsFileUtils.wildcardsFromFilter = staticmethod(QgsFileUtils.wildcardsFromFilter)
    QgsFileUtils.fileMatchesFilter = staticmethod(QgsFileUtils.fileMatchesFilter)
    QgsFileUtils.ensureFileNameHasExtension = staticmethod(QgsFileUtils.ensureFileNameHasExtension)
    QgsFileUtils.addExtensionFromFilter = staticmethod(QgsFileUtils.addExtensionFromFilter)
    QgsFileUtils.stringToSafeFilename = staticmethod(QgsFileUtils.stringToSafeFilename)
    QgsFileUtils.findClosestExistingPath = staticmethod(QgsFileUtils.findClosestExistingPath)
    QgsFileUtils.findFile = staticmethod(QgsFileUtils.findFile)
    QgsFileUtils.driveType = staticmethod(QgsFileUtils.driveType)
    QgsFileUtils.pathIsSlowDevice = staticmethod(QgsFileUtils.pathIsSlowDevice)
    QgsFileUtils.sidecarFilesForPath = staticmethod(QgsFileUtils.sidecarFilesForPath)
    QgsFileUtils.renameDataset = staticmethod(QgsFileUtils.renameDataset)
    QgsFileUtils.splitPathToComponents = staticmethod(QgsFileUtils.splitPathToComponents)
    QgsFileUtils.uniquePath = staticmethod(QgsFileUtils.uniquePath)
    QgsFileUtils.copyDirectory = staticmethod(QgsFileUtils.copyDirectory)
    QgsFileUtils.replaceTextInFile = staticmethod(QgsFileUtils.replaceTextInFile)
except (NameError, AttributeError):
    pass
