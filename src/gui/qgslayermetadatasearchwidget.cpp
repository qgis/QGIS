/***************************************************************************
  qgslayermetadatasearchwidget.cpp - QgsLayerMetadataSearchWidget

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayermetadatasearchwidget.h"
#include "qgslayermetadataresultsmodel.h"

QgsLayerMetadataSearchWidget::QgsLayerMetadataSearchWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  QgsMetadataSearchContext searchContext;
  searchContext.transformContext = QgsCoordinateTransformContext();

  mModel = new QgsLayerMetadataResultsModel( searchContext, &mFeedback, this );
  mMetadataTableView->setModel( mModel );
}
