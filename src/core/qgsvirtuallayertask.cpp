/***************************************************************************
                          qgsvirtuallayertask.cpp  -  description
                             -------------------
    begin                : Jan 19, 2018
    copyright            : (C) 2017 by Paul Blottiere
    email                : blottiere.paul@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayertask.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

QgsVirtualLayerTask::QgsVirtualLayerTask( const QgsVirtualLayerDefinition &definition )
  : mDefinition( definition )
{
  mDefinition.setLazy( true );
  const QgsVectorLayer::LayerOptions options { QgsCoordinateTransformContext() };
  mLayer = std::make_unique<QgsVectorLayer>( mDefinition.toString(), QStringLiteral( "layer" ), QLatin1String( "virtual" ), options );
}

bool QgsVirtualLayerTask::run()
{
  bool rc = false;
  try
  {
    mLayer->reload(); // blocking call because the loading is postponed
    rc = mLayer->isValid();
  }
  catch ( std::exception &e )
  {
    QgsDebugMsg( QStringLiteral( "Reload error: %1" ).arg( e.what() ) );
    setExceptionText( e.what() );
    rc = false;
  }
  return rc;
}

QgsVirtualLayerDefinition QgsVirtualLayerTask::definition() const
{
  return mDefinition;
}

QgsVectorLayer *QgsVirtualLayerTask::layer()
{
  return mLayer.get();
}

QgsVectorLayer *QgsVirtualLayerTask::takeLayer()
{
  return mLayer.release();
}

void QgsVirtualLayerTask::cancel()
{
  mLayer->dataProvider()->cancelReload();
  QgsTask::cancel();
}

QString QgsVirtualLayerTask::exceptionText() const
{
  return mExceptionText;
}

void QgsVirtualLayerTask::setExceptionText( const QString &exceptionText )
{
  mExceptionText = exceptionText;
}
