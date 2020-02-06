# The following has been generated automatically from src/gui/qgsmaplayeractionregistry.h
# monkey patching scoped based enum
QgsMapLayerAction.Layer = QgsMapLayerAction.Target.Layer
QgsMapLayerAction.Target.Layer.__doc__ = ""
QgsMapLayerAction.SingleFeature = QgsMapLayerAction.Target.SingleFeature
QgsMapLayerAction.Target.SingleFeature.__doc__ = ""
QgsMapLayerAction.MultipleFeatures = QgsMapLayerAction.Target.MultipleFeatures
QgsMapLayerAction.Target.MultipleFeatures.__doc__ = ""
QgsMapLayerAction.AllActions = QgsMapLayerAction.Target.AllActions
QgsMapLayerAction.Target.AllActions.__doc__ = ""
QgsMapLayerAction.Target.__doc__ = '\n\n' + '* ``Layer``: ' + QgsMapLayerAction.Target.Layer.__doc__ + '\n' + '* ``SingleFeature``: ' + QgsMapLayerAction.Target.SingleFeature.__doc__ + '\n' + '* ``MultipleFeatures``: ' + QgsMapLayerAction.Target.MultipleFeatures.__doc__ + '\n' + '* ``AllActions``: ' + QgsMapLayerAction.Target.AllActions.__doc__
# --
QgsMapLayerAction.Target.baseClass = QgsMapLayerAction
QgsMapLayerAction.Targets.baseClass = QgsMapLayerAction
Targets = QgsMapLayerAction  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
QgsMapLayerAction.EnabledOnlyWhenEditable = QgsMapLayerAction.Flag.EnabledOnlyWhenEditable
QgsMapLayerAction.Flag.EnabledOnlyWhenEditable.__doc__ = "Action should be shown only for editable layers"
QgsMapLayerAction.Flag.__doc__ = 'Flags which control action behavior\n\n.. versionadded:: 3.0\n\n' + '* ``EnabledOnlyWhenEditable``: ' + QgsMapLayerAction.Flag.EnabledOnlyWhenEditable.__doc__
# --
QgsMapLayerAction.Flag.baseClass = QgsMapLayerAction
QgsMapLayerAction.Flags.baseClass = QgsMapLayerAction
Flags = QgsMapLayerAction  # dirty hack since SIP seems to introduce the flags in module
