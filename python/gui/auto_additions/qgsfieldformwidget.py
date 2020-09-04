# The following has been generated automatically from src/gui/qgsfieldformwidget.h
# monkey patching scoped based enum
QgsFieldFormWidget.AdvancedField.Neither.__doc__ = ""
QgsFieldFormWidget.AdvancedField.Comment.__doc__ = ""
QgsFieldFormWidget.AdvancedField.Alias.__doc__ = ""
QgsFieldFormWidget.AdvancedField.IsNullable.__doc__ = ""
QgsFieldFormWidget.AdvancedField.All.__doc__ = ""
QgsFieldFormWidget.AdvancedField.__doc__ = 'List of advanced fields to be shown. If `Neither` is chosen, the whole advanced fields UI is hidden.\n\n' + '* ``Neither``: ' + QgsFieldFormWidget.AdvancedField.Neither.__doc__ + '\n' + '* ``Comment``: ' + QgsFieldFormWidget.AdvancedField.Comment.__doc__ + '\n' + '* ``Alias``: ' + QgsFieldFormWidget.AdvancedField.Alias.__doc__ + '\n' + '* ``IsNullable``: ' + QgsFieldFormWidget.AdvancedField.IsNullable.__doc__ + '\n' + '* ``All``: ' + QgsFieldFormWidget.AdvancedField.All.__doc__
# --
QgsFieldFormWidget.AdvancedFields.baseClass = QgsFieldFormWidget
AdvancedFields = QgsFieldFormWidget  # dirty hack since SIP seems to introduce the flags in module
