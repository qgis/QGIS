/***************************************************************************
                             qgsmodelcomponentgraphicitem.cpp
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelcomponentgraphicitem.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingmodelparameter.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsprocessingmodeloutput.h"
#include "qgsmodelgraphicsscene.h"

///@cond NOT_STABLE

QgsModelComponentGraphicItem::QgsModelComponentGraphicItem( QgsProcessingModelComponent *component, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QGraphicsObject( parent )
  , mComponent( component )
  , mModel( model )
{
  setAcceptHoverEvents( true );
  setFlag( QGraphicsItem::ItemIsMovable, true );
  setFlag( QGraphicsItem::ItemIsSelectable, true );
  setFlag( QGraphicsItem::ItemSendsGeometryChanges, true );
  setZValue( QgsModelGraphicsScene::ZValues::ModelComponent );
}

QgsProcessingModelComponent *QgsModelComponentGraphicItem::component()
{
  return mComponent.get();
}

QgsProcessingModelAlgorithm *QgsModelComponentGraphicItem::model()
{
  return mModel;
}

QgsModelParameterGraphicItem::QgsModelParameterGraphicItem( QgsProcessingModelParameter *parameter, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( parameter, model, parent )
{

}



QgsModelChildAlgorithmGraphicItem::QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( child, model, parent )
{

}


QgsModelOutputGraphicItem::QgsModelOutputGraphicItem( QgsProcessingModelOutput *output, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent )
  : QgsModelComponentGraphicItem( output, model, parent )
{
}

///@endcond
