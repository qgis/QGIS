# The following has been generated automatically from src/core/qgsattributeeditorelement.h
# monkey patching scoped based enum
QgsAttributeEditorRelation.Button.Link.__doc__ = "Show link button"
QgsAttributeEditorRelation.Button.Unlink.__doc__ = "Show unlink button"
QgsAttributeEditorRelation.Button.SaveChildEdits.__doc__ = "Show save child edits button"
QgsAttributeEditorRelation.Button.AddChildFeature.__doc__ = "Show add child feature (as in some projects we only want to allow to link/unlink existing features)"
QgsAttributeEditorRelation.Button.DuplicateChildFeature.__doc__ = "Show duplicate child feature"
QgsAttributeEditorRelation.Button.DeleteChildFeature.__doc__ = "Show delete child feature button"
QgsAttributeEditorRelation.Button.ZoomToChildFeature.__doc__ = "Show zoom to child feature"
QgsAttributeEditorRelation.Button.AllButtons.__doc__ = "Defaults set of buttons shown"
QgsAttributeEditorRelation.Button.__doc__ = 'Possible buttons shown in the relation editor\n\n.. versionadded:: 3.16\n\n' + '* ``Link``: ' + QgsAttributeEditorRelation.Button.Link.__doc__ + '\n' + '* ``Unlink``: ' + QgsAttributeEditorRelation.Button.Unlink.__doc__ + '\n' + '* ``SaveChildEdits``: ' + QgsAttributeEditorRelation.Button.SaveChildEdits.__doc__ + '\n' + '* ``AddChildFeature``: ' + QgsAttributeEditorRelation.Button.AddChildFeature.__doc__ + '\n' + '* ``DuplicateChildFeature``: ' + QgsAttributeEditorRelation.Button.DuplicateChildFeature.__doc__ + '\n' + '* ``DeleteChildFeature``: ' + QgsAttributeEditorRelation.Button.DeleteChildFeature.__doc__ + '\n' + '* ``ZoomToChildFeature``: ' + QgsAttributeEditorRelation.Button.ZoomToChildFeature.__doc__ + '\n' + '* ``AllButtons``: ' + QgsAttributeEditorRelation.Button.AllButtons.__doc__
# --
QgsAttributeEditorRelation.Button.baseClass = QgsAttributeEditorRelation
QgsAttributeEditorRelation.Buttons.baseClass = QgsAttributeEditorRelation
Buttons = QgsAttributeEditorRelation  # dirty hack since SIP seems to introduce the flags in module
