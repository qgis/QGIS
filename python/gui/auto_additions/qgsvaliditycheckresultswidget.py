# The following has been generated automatically from src/gui/qgsvaliditycheckresultswidget.h
QgsValidityCheckResultsModel.Roles = QgsValidityCheckResultsModel.CustomRole
# monkey patching scoped based enum
QgsValidityCheckResultsModel.DescriptionRole = QgsValidityCheckResultsModel.CustomRole.Description
QgsValidityCheckResultsModel.Roles.DescriptionRole = QgsValidityCheckResultsModel.CustomRole.Description
QgsValidityCheckResultsModel.DescriptionRole.is_monkey_patched = True
QgsValidityCheckResultsModel.DescriptionRole.__doc__ = "Result detailed description"
QgsValidityCheckResultsModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsValidityCheckResultsModel.Roles

.. versionadded:: 3.36

* ``DescriptionRole``: Result detailed description

"""
# --
QgsValidityCheckResultsModel.CustomRole.baseClass = QgsValidityCheckResultsModel
QgsValidityCheckResultsWidget.runChecks = staticmethod(QgsValidityCheckResultsWidget.runChecks)
