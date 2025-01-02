# The following has been generated automatically from src/core/qgsfeaturepickermodelbase.h
QgsFeaturePickerModelBase.Role = QgsFeaturePickerModelBase.CustomRole
# monkey patching scoped based enum
QgsFeaturePickerModelBase.IdentifierValueRole = QgsFeaturePickerModelBase.CustomRole.IdentifierValue
QgsFeaturePickerModelBase.Role.IdentifierValueRole = QgsFeaturePickerModelBase.CustomRole.IdentifierValue
QgsFeaturePickerModelBase.IdentifierValueRole.is_monkey_patched = True
QgsFeaturePickerModelBase.IdentifierValueRole.__doc__ = "Used to retrieve the identifier value (primary key) of a feature. \n.. deprecated:: 3.40. Use IdentifierValuesRole instead."
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
QgsFeaturePickerModelBase.CustomRole.__doc__ = """Extra roles that can be used to fetch data from this model.

.. note::

   Prior to QGIS 3.36 this was available as QgsFeaturePickerModelBase.Role

.. versionadded:: 3.36

* ``IdentifierValue``: Used to retrieve the identifier value (primary key) of a feature.

  .. deprecated:: 3.40. Use IdentifierValuesRole instead.


  Available as ``QgsFeaturePickerModelBase.IdentifierValueRole`` in older QGIS releases.

* ``IdentifierValues``: Used to retrieve the identifierValues (primary keys) of a feature.

  Available as ``QgsFeaturePickerModelBase.IdentifierValuesRole`` in older QGIS releases.

* ``Value``: Used to retrieve the displayExpression of a feature.

  Available as ``QgsFeaturePickerModelBase.ValueRole`` in older QGIS releases.

* ``Feature``: Used to retrieve the feature, it might be incomplete if the request doesn't fetch all attributes or geometry.

  Available as ``QgsFeaturePickerModelBase.FeatureRole`` in older QGIS releases.

* ``FeatureId``: Used to retrieve the id of a feature.

  Available as ``QgsFeaturePickerModelBase.FeatureIdRole`` in older QGIS releases.


"""
# --
QgsFeaturePickerModelBase.CustomRole.baseClass = QgsFeaturePickerModelBase
try:
    QgsFeaturePickerModelBase.__attribute_docs__ = {'currentFeatureChanged': 'Emitted when the current feature in the model has changed\nThis emitted both when the extra value changes and when the extra value status changes.\nIt allows being notified when the feature is fetched after the extra value has been set.\n\n.. versionadded:: 3.16.5\n', 'sourceLayerChanged': 'The source layer from which features will be fetched.\n', 'displayExpressionChanged': 'The display expression will be used for\n\n- displaying values in the combobox\n- filtering based on filterValue\n', 'filterValueChanged': 'This value will be used to filter the features available from\nthis model. Whenever a substring of the displayExpression of a feature\nmatches the filter value, it will be accessible by this model.\n', 'filterExpressionChanged': 'An additional filter expression to apply, next to the filterValue.\nCan be used for spatial filtering etc.\n', 'isLoadingChanged': 'Indicator if the model is currently performing any feature iteration in the background.\n', 'filterJobCompleted': 'Indicates that a filter job has been completed and new data may be available.\n', 'extraIdentifierValueChanged': 'Allows specifying one value that does not need to match the filter criteria but will\nstill be available in the model.\n', 'extraIdentifierValueIndexChanged': 'The index at which the extra identifier value is available within the model.\n', 'extraValueDoesNotExistChanged': 'Notification whether the model has ``found`` a feature tied to the extraIdentifierValue or not.\n', 'beginUpdate': 'Notification that the model is about to be changed because a job was completed.\n', 'endUpdate': 'Notification that the model change is finished. Will always be emitted in sync with beginUpdate.\n', 'allowNullChanged': 'Add a NULL entry to the list.\n', 'fetchGeometryChanged': 'Emitted when the fetching of the geometry changes\n', 'fetchLimitChanged': 'Emitted when the fetching limit for the feature request changes\n'}
    QgsFeaturePickerModelBase.__signal_arguments__ = {'extraIdentifierValueIndexChanged': ['index: int'], 'extraValueDoesNotExistChanged': ['found: bool']}
except (NameError, AttributeError):
    pass
