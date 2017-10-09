/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrelationeditorwidget.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%ModuleHeaderCode
// fix to allow compilation with sip that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsrelationeditorwidget.h>
%End

class QgsRelationEditorWidget : QgsCollapsibleGroupBox
{

%TypeHeaderCode
#include "qgsrelationeditorwidget.h"
%End
%ConvertToSubClassCode
    if ( qobject_cast<QgsRelationEditorWidget *>( sipCpp ) )
      sipType = sipType_QgsRelationEditorWidget;
    else
      sipType = NULL;
%End
  public:

    QgsRelationEditorWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 \param parent parent widget
%End

    void setViewMode( QgsDualView::ViewMode mode );
%Docstring
Define the view mode for the dual view
%End

    QgsDualView::ViewMode viewMode();
%Docstring
Get the view mode for the dual view
 :rtype: QgsDualView.ViewMode
%End

    void setRelationFeature( const QgsRelation &relation, const QgsFeature &feature );

    void setRelations( const QgsRelation &relation, const QgsRelation &nmrelation );
%Docstring
 Set the relation(s) for this widget
 If only one relation is set, it will act as a simple 1:N relation widget
 If both relations are set, it will act as an N:M relation widget
 inserting and deleting entries on the intermediate table as required.

 \param relation    Relation referencing the edited table
 \param nmrelation  Optional reference from the referencing table to a 3rd N:M table
%End

    void setFeature( const QgsFeature &feature );

    void setEditorContext( const QgsAttributeEditorContext &context );

    QgsIFeatureSelectionManager *featureSelectionManager();
%Docstring
 The feature selection manager is responsible for the selected features
 which are currently being edited.
 :rtype: QgsIFeatureSelectionManager
%End

    bool showLabel() const;
%Docstring
 Defines if a title label should be shown for this widget.

.. versionadded:: 2.18
 :rtype: bool
%End

    void setShowLabel( bool showLabel );
%Docstring
 Defines if a title label should be shown for this widget.

.. versionadded:: 2.18
%End

    bool showLinkButton() const;
%Docstring
 Determines if the "link feature" button should be shown

.. versionadded:: 2.18
 :rtype: bool
%End

    void setShowLinkButton( bool showLinkButton );
%Docstring
 Determines if the "link feature" button should be shown

.. versionadded:: 2.18
%End

    bool showUnlinkButton() const;
%Docstring
 Determines if the "unlink feature" button should be shown

.. versionadded:: 2.18
 :rtype: bool
%End

    void setShowUnlinkButton( bool showUnlinkButton );
%Docstring
 Determines if the "unlink feature" button should be shown

.. versionadded:: 2.18
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrelationeditorwidget.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
