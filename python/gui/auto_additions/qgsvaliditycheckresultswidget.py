# The following has been generated automatically from src/gui/qgsvaliditycheckresultswidget.h
QgsValidityCheckResultsModel.Roles = QgsValidityCheckResultsModel.CustomRole
# monkey patching scoped based enum
QgsValidityCheckResultsModel.DescriptionRole = QgsValidityCheckResultsModel.CustomRole.Description
QgsValidityCheckResultsModel.Roles.DescriptionRole = QgsValidityCheckResultsModel.CustomRole.Description
QgsValidityCheckResultsModel.DescriptionRole.is_monkey_patched = True
QgsValidityCheckResultsModel.DescriptionRole.__doc__ = "Result detailed description"
QgsValidityCheckResultsModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsValidityCheckResultsModel.Roles\n\n.. versionadded:: 3.36\n\n" + '* ``DescriptionRole``: ' + QgsValidityCheckResultsModel.CustomRole.Description.__doc__
# --
QgsValidityCheckResultsModel.CustomRole.baseClass = QgsValidityCheckResultsModel
QgsValidityCheckResultsWidget.runChecks = staticmethod(QgsValidityCheckResultsWidget.runChecks)
