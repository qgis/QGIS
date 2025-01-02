# The following has been generated automatically from src/gui/qgsfeaturepickerwidget.h
try:
    QgsFeaturePickerWidget.__attribute_docs__ = {'modelUpdated': 'The underlying model has been updated.\n', 'layerChanged': 'The layer from which features should be listed.\n', 'displayExpressionChanged': 'The display expression will be used to display features as well as\nthe the value to match the typed text against.\n', 'filterExpressionChanged': 'An additional expression to further restrict the available features.\nThis can be used to integrate additional spatial or other constraints.\n', 'featureChanged': 'Sends the feature as soon as it is chosen\n', 'allowNullChanged': 'Determines if a NULL value should be available in the list.\n', 'fetchGeometryChanged': 'Emitted when the fetching of the geometry changes\n', 'fetchLimitChanged': 'Emitted when the fetching limit for the feature request changes\n', 'showBrowserButtonsChanged': 'Emitted when showing the browser buttons changes\n'}
    QgsFeaturePickerWidget.__signal_arguments__ = {'featureChanged': ['feature: QgsFeature']}
except (NameError, AttributeError):
    pass
