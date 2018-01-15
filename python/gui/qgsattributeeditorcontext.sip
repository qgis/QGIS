/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeeditorcontext.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAttributeEditorContext
{
%Docstring
This class contains context information for attribute editor widgets.
It will be passed to embedded widgets whenever this occurs (e.g. when
showing an embedded form due to relations)
%End

%TypeHeaderCode
#include "qgsattributeeditorcontext.h"
%End
  public:

    enum RelationMode
    {
      Undefined,
      Multiple,
      Single
    };

    enum FormMode
    {
      Embed,
      StandaloneDialog,
      Popup
    };

    QgsAttributeEditorContext();
%Docstring
Constructor for QgsAttributeEditorContext
%End

    QgsAttributeEditorContext( const QgsAttributeEditorContext &parentContext, FormMode formMode );

    QgsAttributeEditorContext( const QgsAttributeEditorContext &parentContext, const QgsRelation &relation, RelationMode relationMode, FormMode widgetMode );

    void setDistanceArea( const QgsDistanceArea &distanceArea );

    const QgsDistanceArea &distanceArea() const;

    void setVectorLayerTools( QgsVectorLayerTools *vlTools );
    const QgsVectorLayerTools *vectorLayerTools() const;

    void setRelation( const QgsRelation &relation, RelationMode mode );
    const QgsRelation &relation() const;
    RelationMode relationMode() const;

    FormMode formMode() const;
%Docstring
Returns the form mode.

.. seealso:: :py:func:`setFormMode`
%End

    void setFormMode( FormMode mode );
%Docstring
Sets the form mode.

:param mode: form mode

.. seealso:: :py:func:`formMode`

.. versionadded:: 2.16
%End

    bool allowCustomUi() const;
%Docstring
Returns true if the attribute editor should permit use of custom UI forms.

.. seealso:: :py:func:`setAllowCustomUi`

.. versionadded:: 2.16
%End

    void setAllowCustomUi( bool allow );
%Docstring
Sets whether the attribute editor should permit use of custom UI forms.

:param allow: set to true to allow custom UI forms, or false to disable them and use default generated
QGIS forms

.. seealso:: :py:func:`allowCustomUi`

.. versionadded:: 2.16
%End

    const QgsAttributeEditorContext *parentContext() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeeditorcontext.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
