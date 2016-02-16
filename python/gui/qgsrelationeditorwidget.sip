/***************************************************************************
    qgsrelationeditorwidget.sip
     --------------------------------------
    Date                 : 28.11.2015
    Copyright            : (C) 2015 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%ModuleCode
#include "qgsrelationeditorwidget.h"
%End

class QgsRelationEditorWidget : QgsCollapsibleGroupBox
{
%TypeHeaderCode
#include <qgsrelationeditorwidget.h>
%End


%ConvertToSubClassCode
  if ( qobject_cast<QgsRelationEditorWidget*>( sipCpp ) )
    sipType = sipType_QgsRelationEditorWidget;
  else
    sipType = 0;
%End

  public:
    /**
     * @param parent parent widget
     */
    QgsRelationEditorWidget( QWidget* parent /TransferThis/= 0 );

    //! Define the view mode for the dual view
    void setViewMode( QgsDualView::ViewMode mode );

    //! Get the view mode for the dual view
    QgsDualView::ViewMode viewMode();

    void setRelationFeature( const QgsRelation& relation, const QgsFeature& feature );

    /**
     * Set the relation(s) for this widget
     * If only one relation is set, it will act as a simple 1:N relation widget
     * If both relations are set, it will act as an N:M relation widget
     * inserting and deleting entries on the intermediate table as required.
     *
     * @param relation    Relation referencing the edited table
     * @param nmrelation  Optional reference from the referencing table to a 3rd N:M table
     */
    void setRelations( const QgsRelation& relation, const QgsRelation& nmrelation );

    void setFeature( const QgsFeature& feature );

    void setEditorContext( const QgsAttributeEditorContext& context );

    /**
     * The feature selection manager is responsible for the selected features
     * which are currently being edited.
     */
    QgsIFeatureSelectionManager* featureSelectionManager();
};
