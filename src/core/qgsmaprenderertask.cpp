/***************************************************************************
                          qgsmaprenderertask.h
                          -------------------------
    begin                : Apr 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotation.h"
#include "qgsannotationmanager.h"
#include "qgsmaprenderertask.h"
#include "qgsmaprenderercustompainterjob.h"


QgsMapRendererTask::QgsMapRendererTask( const QgsMapSettings &ms, const QString &fileName, const QString &fileFormat )
  : QgsTask( tr( "Saving as image" ) )
  , mMapSettings( ms )
  , mFileName( fileName )
  , mFileFormat( fileFormat )
{
}

QgsMapRendererTask::QgsMapRendererTask( const QgsMapSettings &ms, QPainter *p )
  : QgsTask( tr( "Saving as image" ) )
  , mMapSettings( ms )
  , mPainter( p )
{
}

void QgsMapRendererTask::addAnnotations( QList< QgsAnnotation * > annotations )
{
  qDeleteAll( mAnnotations );
  mAnnotations.clear();

  Q_FOREACH ( const QgsAnnotation *a, annotations )
  {
    mAnnotations << a->clone();
  }
}

bool QgsMapRendererTask::run()
{
  QImage img;
  std::unique_ptr< QPainter > tempPainter;
  QPainter *destPainter = mPainter;

  if ( !mPainter )
  {
    // save rendered map to an image file
    img = QImage( mMapSettings.outputSize(), QImage::Format_ARGB32 );
    if ( img.isNull() )
    {
      mError = ImageAllocationFail;
      return false;
    }

    img.setDotsPerMeterX( 1000 * mMapSettings.outputDpi() / 25.4 );
    img.setDotsPerMeterY( 1000 * mMapSettings.outputDpi() / 25.4 );

    tempPainter.reset( new QPainter( &img ) );
    destPainter = tempPainter.get();
  }

  if ( !destPainter )
    return false;

  QgsMapRendererCustomPainterJob r( mMapSettings, destPainter );
  r.renderSynchronously();

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mMapSettings );
  context.setPainter( destPainter );

  Q_FOREACH ( QgsAnnotation *annotation, mAnnotations )
  {
    if ( !annotation || !annotation->isVisible() )
    {
      continue;
    }
    if ( annotation->mapLayer() && !mMapSettings.layers().contains( annotation->mapLayer() ) )
    {
      continue;
    }

    context.painter()->save();
    context.painter()->setRenderHint( QPainter::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );

    double itemX, itemY;
    if ( annotation->hasFixedMapPosition() )
    {
      itemX = mMapSettings.outputSize().width() * ( annotation->mapPosition().x() - mMapSettings.extent().xMinimum() ) / mMapSettings.extent().width();
      itemY = mMapSettings.outputSize().height() * ( 1 - ( annotation->mapPosition().y() - mMapSettings.extent().yMinimum() ) / mMapSettings.extent().height() );
    }
    else
    {
      itemX = annotation->relativePosition().x() * mMapSettings.outputSize().width();
      itemY = annotation->relativePosition().y() * mMapSettings.outputSize().height();
    }

    context.painter()->translate( itemX, itemY );

    annotation->render( context );
    context.painter()->restore();
  }
  qDeleteAll( mAnnotations );
  mAnnotations.clear();

  if ( !mFileName.isEmpty() )
  {
    destPainter->end();
    bool success = img.save( mFileName, mFileFormat.toLocal8Bit().data() );
    if ( !success )
    {
      mError = ImageSaveFail;
      return false;
    }
  }

  return true;
}

void QgsMapRendererTask::finished( bool result )
{
  if ( result )
    emit renderingComplete();
  else
    emit errorOccurred( mError );
}
