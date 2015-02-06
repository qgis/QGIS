/***************************************************************************
    qgsattributeeditorcontext.h
     --------------------------------------
    Date                 : 30.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEEDITORCONTEXT_H
#define QGSATTRIBUTEEDITORCONTEXT_H

#include <QMap>
#include <QWidget>

#include <qgsdistancearea.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayertools.h>


/**
 * This class contains context information for attribute editor widgets.
 * It will be passed to embedded widgets whenever this occurs (e.g. when
 * showing an embedded form due to relations)
 */

class GUI_EXPORT QgsAttributeEditorContext
{
  public:
    /**
     * Determines in which direction a relation was resolved.
     */
    enum RelationMode
    {
      Undefined,  //!< This context is not defined by a relation
      Multiple,   //!< When showing a list of features (e.g. houses as an embedded form in a district form)
      Single      //!< When showing a single feature (e.g. district information when looking at the form of a house)
    };

    enum FormMode
    {
      Embed,            //!< A form was embedded as a widget on another form
      StandaloneDialog, //!< A form was opened as a new dialog
      Popup             //!< A widget was opened as a popup (e.g. attribute table editor widget)
    };

    QgsAttributeEditorContext()
        : mParentContext( 0 )
        , mLayer( 0 )
        , mVectorLayerTools( 0 )
        , mRelationMode( Undefined )
        , mFormMode( Embed )
    {}

    QgsAttributeEditorContext( const QgsAttributeEditorContext& parentContext, FormMode formMode )
        : mParentContext( &parentContext )
        , mLayer( 0 )
        , mVectorLayerTools( parentContext.mVectorLayerTools )
        , mDistanceArea( parentContext.mDistanceArea )
        , mRelationMode( Undefined )
        , mFormMode( formMode )
    {
      Q_ASSERT( parentContext.vectorLayerTools() );
    }

    QgsAttributeEditorContext( const QgsAttributeEditorContext& parentContext, const QgsRelation& relation, RelationMode relationMode, FormMode widgetMode )
        : mParentContext( &parentContext )
        , mLayer( 0 )
        , mVectorLayerTools( parentContext.mVectorLayerTools )
        , mDistanceArea( parentContext.mDistanceArea )
        , mRelation( relation )
        , mRelationMode( relationMode )
        , mFormMode( widgetMode )
    {
      Q_ASSERT( parentContext.vectorLayerTools() );
    }

    inline void setDistanceArea( const QgsDistanceArea& distanceArea )
    {
      if ( mLayer )
      {
        mDistanceArea = distanceArea;
        mDistanceArea.setSourceCrs( mLayer->crs() );
      }
    }

    inline const QgsDistanceArea& distanceArea() const { return mDistanceArea; }

    inline void setVectorLayerTools( QgsVectorLayerTools* vlTools ) { mVectorLayerTools = vlTools; }
    inline const QgsVectorLayerTools* vectorLayerTools() const { return mVectorLayerTools; }

    inline void setRelation( const QgsRelation& relation, RelationMode mode ) { mRelation = relation; mRelationMode = mode; }
    inline const QgsRelation& relation() const { return mRelation; }
    inline RelationMode relationMode() const { return mRelationMode; }

    inline FormMode formMode() const { return mFormMode; }

    inline const QgsAttributeEditorContext* parentContext() const { return mParentContext; }

  private:
    const QgsAttributeEditorContext* mParentContext;
    QgsVectorLayer* mLayer;
    QgsVectorLayerTools* mVectorLayerTools;
    QgsDistanceArea mDistanceArea;
    QgsRelation mRelation;
    RelationMode mRelationMode;
    FormMode mFormMode;
};

#endif // QGSATTRIBUTEEDITORCONTEXT_H
