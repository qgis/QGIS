# The following has been generated automatically from src/gui/processing/models/qgsmodelgraphicsscene.h
QgsModelGraphicsScene.GroupBox = QgsModelGraphicsScene.ZValues.GroupBox
QgsModelGraphicsScene.ArrowLink = QgsModelGraphicsScene.ZValues.ArrowLink
QgsModelGraphicsScene.ModelComponent = QgsModelGraphicsScene.ZValues.ModelComponent
QgsModelGraphicsScene.MouseHandles = QgsModelGraphicsScene.ZValues.MouseHandles
QgsModelGraphicsScene.RubberBand = QgsModelGraphicsScene.ZValues.RubberBand
QgsModelGraphicsScene.ZSnapIndicator = QgsModelGraphicsScene.ZValues.ZSnapIndicator
QgsModelGraphicsScene.FlagHideControls = QgsModelGraphicsScene.Flag.FlagHideControls
QgsModelGraphicsScene.FlagHideComments = QgsModelGraphicsScene.Flag.FlagHideComments
QgsModelGraphicsScene.Flags = lambda flags=0: QgsModelGraphicsScene.Flag(flags)
try:
    QgsModelGraphicsScene.__attribute_docs__ = {'rebuildRequired': 'Emitted when a change in the model requires a full rebuild of the scene.\n', 'componentAboutToChange': 'Emitted whenever a component of the model is about to be changed.\n\nThe ``text`` argument gives the translated text describing the change about to occur, and the\noptional ``id`` can be used to group the associated undo commands.\n', 'componentChanged': 'Emitted whenever a component of the model is changed.\n', 'selectedItemChanged': 'Emitted whenever the selected item changes.\nIf ``None``, no item is selected.\n', 'runSelected': 'Emitted when the user opts to run selected steps from the model.\n\n.. versionadded:: 3.38\n', 'runFromChild': 'Emitted when the user opts to run the part of the model starting from the specified child algorithm.\n\n.. versionadded:: 3.38\n', 'showChildAlgorithmOutputs': 'Emitted when the user opts to view previous results from the child algorithm with matching ID.\n\n.. versionadded:: 3.38\n', 'showChildAlgorithmLog': 'Emitted when the user opts to view the previous log from the child algorithm with matching ID.\n\n.. versionadded:: 3.38\n'}
    QgsModelGraphicsScene.__signal_arguments__ = {'componentAboutToChange': ['text: str', 'id: int = 0'], 'selectedItemChanged': ['selected: QgsModelComponentGraphicItem'], 'runFromChild': ['childId: str'], 'showChildAlgorithmOutputs': ['childId: str'], 'showChildAlgorithmLog': ['childId: str']}
    QgsModelGraphicsScene.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
