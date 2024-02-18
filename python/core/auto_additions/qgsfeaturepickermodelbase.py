# The following has been generated automatically from src/core/qgsfeaturepickermodelbase.h
QgsFeaturePickerModelBase.Role = QgsFeaturePickerModelBase.CustomRole
# monkey patching scoped based enum
QgsFeaturePickerModelBase.IdentifierValueRole = QgsFeaturePickerModelBase.CustomRole.IdentifierValue
QgsFeaturePickerModelBase.Role.IdentifierValueRole = QgsFeaturePickerModelBase.CustomRole.IdentifierValue
QgsFeaturePickerModelBase.IdentifierValueRole.is_monkey_patched = True
QgsFeaturePickerModelBase.IdentifierValueRole.__doc__ = ".. deprecated:: Use IdentifierValuesRole instead"
QgsFeaturePickerModelBase.IdentifierValuesRole = QgsFeaturePickerModelBase.CustomRole.IdentifierValues
QgsFeaturePickerModelBase.Role.IdentifierValuesRole = QgsFeaturePickerModelBase.CustomRole.IdentifierValues
QgsFeaturePickerModelBase.IdentifierValuesRole.is_monkey_patched = True
QgsFeaturePickerModelBase.IdentifierValuesRole.__doc__ = "Used to retrieve the identifierValues (primary keys) of a feature."
QgsFeaturePickerModelBase.ValueRole = QgsFeaturePickerModelBase.CustomRole.Value
QgsFeaturePickerModelBase.Role.ValueRole = QgsFeaturePickerModelBase.CustomRole.Value
QgsFeaturePickerModelBase.ValueRole.is_monkey_patched = True
QgsFeaturePickerModelBase.ValueRole.__doc__ = "Used to retrieve the displayExpression of a feature."
QgsFeaturePickerModelBase.FeatureRole = QgsFeaturePickerModelBase.CustomRole.Feature
QgsFeaturePickerModelBase.Role.FeatureRole = QgsFeaturePickerModelBase.CustomRole.Feature
QgsFeaturePickerModelBase.FeatureRole.is_monkey_patched = True
QgsFeaturePickerModelBase.FeatureRole.__doc__ = "Used to retrieve the feature, it might be incomplete if the request doesn't fetch all attributes or geometry."
QgsFeaturePickerModelBase.FeatureIdRole = QgsFeaturePickerModelBase.CustomRole.FeatureId
QgsFeaturePickerModelBase.Role.FeatureIdRole = QgsFeaturePickerModelBase.CustomRole.FeatureId
QgsFeaturePickerModelBase.FeatureIdRole.is_monkey_patched = True
QgsFeaturePickerModelBase.FeatureIdRole.__doc__ = "Used to retrieve the id of a feature."
QgsFeaturePickerModelBase.CustomRole.__doc__ = "Extra roles that can be used to fetch data from this model.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsFeaturePickerModelBase.Role\n\n.. versionadded:: 3.36\n\n" + '* ``IdentifierValueRole``: ' + QgsFeaturePickerModelBase.CustomRole.IdentifierValue.__doc__ + '\n' + '* ``IdentifierValuesRole``: ' + QgsFeaturePickerModelBase.CustomRole.IdentifierValues.__doc__ + '\n' + '* ``ValueRole``: ' + QgsFeaturePickerModelBase.CustomRole.Value.__doc__ + '\n' + '* ``FeatureRole``: ' + QgsFeaturePickerModelBase.CustomRole.Feature.__doc__ + '\n' + '* ``FeatureIdRole``: ' + QgsFeaturePickerModelBase.CustomRole.FeatureId.__doc__
# --
QgsFeaturePickerModelBase.CustomRole.baseClass = QgsFeaturePickerModelBase
