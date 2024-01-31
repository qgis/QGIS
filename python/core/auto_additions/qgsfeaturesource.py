# The following has been generated automatically from src/core/qgsfeaturesource.h
# monkey patching scoped based enum
QgsFeatureSource.NoFeaturesAvailable = QgsFeatureSource.FeatureAvailability.NoFeaturesAvailable
QgsFeatureSource.NoFeaturesAvailable.is_monkey_patched = True
QgsFeatureSource.NoFeaturesAvailable.__doc__ = "There are certainly no features available in this source"
QgsFeatureSource.FeaturesAvailable = QgsFeatureSource.FeatureAvailability.FeaturesAvailable
QgsFeatureSource.FeaturesAvailable.is_monkey_patched = True
QgsFeatureSource.FeaturesAvailable.__doc__ = "There is at least one feature available in this source"
QgsFeatureSource.FeaturesMaybeAvailable = QgsFeatureSource.FeatureAvailability.FeaturesMaybeAvailable
QgsFeatureSource.FeaturesMaybeAvailable.is_monkey_patched = True
QgsFeatureSource.FeaturesMaybeAvailable.__doc__ = "There may be features available in this source"
QgsFeatureSource.FeatureAvailability.__doc__ = "Possible return value for :py:func:`~QgsFeatureSource.hasFeatures` to determine if a source is empty.\nIt is implemented as a three-value logic, so it can return if\nthere are features available for sure, if there are no features\navailable for sure or if there might be features available but\nthere is no guarantee for this.\n\n.. versionadded:: 3.4\n\n" + '* ``NoFeaturesAvailable``: ' + QgsFeatureSource.FeatureAvailability.NoFeaturesAvailable.__doc__ + '\n' + '* ``FeaturesAvailable``: ' + QgsFeatureSource.FeatureAvailability.FeaturesAvailable.__doc__ + '\n' + '* ``FeaturesMaybeAvailable``: ' + QgsFeatureSource.FeatureAvailability.FeaturesMaybeAvailable.__doc__
# --
