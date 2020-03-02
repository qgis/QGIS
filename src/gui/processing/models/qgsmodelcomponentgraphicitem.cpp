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
#include "qgsapplication.h"
#include "qgsmodelgraphicitem.h"
#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>

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

  QSvgRenderer svg( QgsApplication::iconPath( QStringLiteral( "mActionEditModelComponent.svg" ) ) );
  QPicture editPicture;
  QPainter painter( &editPicture );
  svg.render( &painter );
  painter.end();
  mEditButton = new QgsModelDesignerFlatButtonGraphicItem( this, editPicture,
      QPointF( component->size().width() / 2.0 - mButtonSize.width() / 2.0,
               component->size().height() / 2.0 - mButtonSize.height() / 2.0 ) );
  connect( mEditButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::editComponent );

  QSvgRenderer svg2( QgsApplication::iconPath( QStringLiteral( "mActionDeleteModelComponent.svg" ) ) );
  QPicture deletePicture;
  painter.begin( &deletePicture );
  svg2.render( &painter );
  painter.end();
  mDeleteButton = new QgsModelDesignerFlatButtonGraphicItem( this, deletePicture,
      QPointF( component->size().width() / 2.0 - mButtonSize.width() / 2.0,
               mButtonSize.height() / 2.0 - component->size().height() / 2.0 ) );
  connect( mDeleteButton, &QgsModelDesignerFlatButtonGraphicItem::clicked, this, &QgsModelComponentGraphicItem::deleteComponent );
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
