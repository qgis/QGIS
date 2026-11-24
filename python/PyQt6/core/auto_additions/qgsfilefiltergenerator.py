# The following has been generated automatically from src/core/qgsfilefiltergenerator.h
# monkey patching scoped based enum
QgsFileFilterGenerator.StandardFileFormat.QML.__doc__ = "QML Files"
QgsFileFilterGenerator.StandardFileFormat.HTML.__doc__ = "HTML Files"
QgsFileFilterGenerator.StandardFileFormat.TXT.__doc__ = "Text files"
QgsFileFilterGenerator.StandardFileFormat.SLD.__doc__ = "SLD files"
QgsFileFilterGenerator.StandardFileFormat.AllFiles.__doc__ = "All files (*.*)"
QgsFileFilterGenerator.StandardFileFormat.__doc__ = """Standard file filters.

.. versionadded:: 4.6

* ``QML``: QML Files
* ``HTML``: HTML Files
* ``TXT``: Text files
* ``SLD``: SLD files
* ``AllFiles``: All files (*.*)

"""
# --
try:
    QgsFileFilterGenerator.fileFilter = staticmethod(QgsFileFilterGenerator.fileFilter)
    QgsFileFilterGenerator.filterString = staticmethod(QgsFileFilterGenerator.filterString)
    QgsFileFilterGenerator.__abstract_methods__ = ['createFileFilter']
except (NameError, AttributeError):
    pass
