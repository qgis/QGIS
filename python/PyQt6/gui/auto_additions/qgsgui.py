# The following has been generated automatically from src/gui/qgsgui.h
QgsGui.UseCrsOfFirstLayerAdded = QgsGui.ProjectCrsBehavior.UseCrsOfFirstLayerAdded
QgsGui.UsePresetCrs = QgsGui.ProjectCrsBehavior.UsePresetCrs
QgsGui.ProjectCrsBehavior.baseClass = QgsGui
QgsGui.HigMenuTextIsTitleCase = QgsGui.HigFlag.HigMenuTextIsTitleCase
QgsGui.HigDialogTitleIsTitleCase = QgsGui.HigFlag.HigDialogTitleIsTitleCase
QgsGui.HigFlags = lambda flags=0: QgsGui.HigFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsGui.HigFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsGui.HigFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsGui.HigFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsGui.HigFlag.__or__ = lambda flag1, flag2: QgsGui.HigFlag(_force_int(flag1) | _force_int(flag2))
try:
    QgsGui.__attribute_docs__ = {'optionsChanged': 'This signal is emitted whenever the application options have been changed.\n\nThis signal is a "blanket" signal, and will be emitted whenever the options dialog\nhas been accepted regardless of whether or not individual settings are changed.\nIt is designed as a "last resort" fallback only, allowing widgets to respond\nto possible settings changes.\n\n.. versionadded:: 3.16\n'}
    QgsGui.instance = staticmethod(QgsGui.instance)
    QgsGui.settingsRegistryGui = staticmethod(QgsGui.settingsRegistryGui)
    QgsGui.editorWidgetRegistry = staticmethod(QgsGui.editorWidgetRegistry)
    QgsGui.sourceSelectProviderRegistry = staticmethod(QgsGui.sourceSelectProviderRegistry)
    QgsGui.shortcutsManager = staticmethod(QgsGui.shortcutsManager)
    QgsGui.layerTreeEmbeddedWidgetRegistry = staticmethod(QgsGui.layerTreeEmbeddedWidgetRegistry)
    QgsGui.mapLayerActionRegistry = staticmethod(QgsGui.mapLayerActionRegistry)
    QgsGui.layoutItemGuiRegistry = staticmethod(QgsGui.layoutItemGuiRegistry)
    QgsGui.annotationItemGuiRegistry = staticmethod(QgsGui.annotationItemGuiRegistry)
    QgsGui.advancedDigitizingToolsRegistry = staticmethod(QgsGui.advancedDigitizingToolsRegistry)
    QgsGui.processingGuiRegistry = staticmethod(QgsGui.processingGuiRegistry)
    QgsGui.numericFormatGuiRegistry = staticmethod(QgsGui.numericFormatGuiRegistry)
    QgsGui.codeEditorColorSchemeRegistry = staticmethod(QgsGui.codeEditorColorSchemeRegistry)
    QgsGui.processingRecentAlgorithmLog = staticmethod(QgsGui.processingRecentAlgorithmLog)
    QgsGui.processingFavoriteAlgorithmManager = staticmethod(QgsGui.processingFavoriteAlgorithmManager)
    QgsGui.dataItemGuiProviderRegistry = staticmethod(QgsGui.dataItemGuiProviderRegistry)
    QgsGui.projectStorageGuiRegistry = staticmethod(QgsGui.projectStorageGuiRegistry)
    QgsGui.providerGuiRegistry = staticmethod(QgsGui.providerGuiRegistry)
    QgsGui.sensorGuiRegistry = staticmethod(QgsGui.sensorGuiRegistry)
    QgsGui.subsetStringEditorProviderRegistry = staticmethod(QgsGui.subsetStringEditorProviderRegistry)
    QgsGui.sourceWidgetProviderRegistry = staticmethod(QgsGui.sourceWidgetProviderRegistry)
    QgsGui.relationWidgetRegistry = staticmethod(QgsGui.relationWidgetRegistry)
    QgsGui.historyProviderRegistry = staticmethod(QgsGui.historyProviderRegistry)
    QgsGui.settingsEditorWidgetRegistry = staticmethod(QgsGui.settingsEditorWidgetRegistry)
    QgsGui.enableAutoGeometryRestore = staticmethod(QgsGui.enableAutoGeometryRestore)
    QgsGui.windowManager = staticmethod(QgsGui.windowManager)
    QgsGui.setWindowManager = staticmethod(QgsGui.setWindowManager)
    QgsGui.inputControllerManager = staticmethod(QgsGui.inputControllerManager)
    QgsGui.higFlags = staticmethod(QgsGui.higFlags)
    QgsGui.sampleColor = staticmethod(QgsGui.sampleColor)
    QgsGui.findScreenAt = staticmethod(QgsGui.findScreenAt)
    QgsGui.hasWebEngine = staticmethod(QgsGui.hasWebEngine)
except (NameError, AttributeError):
    pass
