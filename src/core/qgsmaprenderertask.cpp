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
#include "qgsmapsettingsutils.h"

#include <QFile>
#include <QTextStream>
#include <QPrinter>

QgsMapRendererTask::QgsMapRendererTask( const QgsMapSettings &ms, const QString &fileName, const QString &fileFormat, const bool forceRaster )
  : QgsTask( tr( "Saving as image" ) )
  , mMapSettings( ms )
  , mFileName( fileName )
  , mFileFormat( fileFormat )
  , mForceRaster( forceRaster )
{
}

QgsMapRendererTask::QgsMapRendererTask( const QgsMapSettings &ms, QPainter *p )
  : QgsTask( tr( "Rendering to painter" ) )
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

void QgsMapRendererTask::addDecorations( const QList< QgsMapDecoration * > &decorations )
{
  mDecorations = decorations;
}


void QgsMapRendererTask::cancel()
{
  mJobMutex.lock();
  if ( mJob )
    mJob->cancelWithoutBlocking();
  mJobMutex.unlock();

  QgsTask::cancel();
}

bool QgsMapRendererTask::run()
{
  QImage img;
  std::unique_ptr< QPainter > tempPainter;
  QPainter *destPainter = mPainter;
  std::unique_ptr< QPrinter > printer;

  if ( mFileFormat == QStringLiteral( "PDF" ) )
  {
    printer.reset( new QPrinter() );
    printer->setOutputFileName( mFileName );
    printer->setOutputFormat( QPrinter::PdfFormat );
    printer->setOrientation( QPrinter::Portrait );
    // paper size needs to be given in millimeters in order to be able to set a resolution to pass onto the map renderer
    printer->setPaperSize( mMapSettings.outputSize()  * 25.4 / mMapSettings.outputDpi(), QPrinter::Millimeter );
    printer->setPageMargins( 0, 0, 0, 0, QPrinter::Millimeter );
    printer->setResolution( mMapSettings.outputDpi() );

    if ( !mForceRaster )
    {
      tempPainter.reset( new QPainter( printer.get() ) );
      destPainter = tempPainter.get();
    }
  }

  if ( !destPainter )
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

  mJobMutex.lock();
  mJob.reset( new QgsMapRendererCustomPainterJob( mMapSettings, destPainter ) );
  mJobMutex.unlock();
  mJob->renderSynchronously();

  mJobMutex.lock();
  mJob.reset( nullptr );
  mJobMutex.unlock();

  if ( isCanceled() )
    return false;

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mMapSettings );
  context.setPainter( destPainter );

  Q_FOREACH ( QgsMapDecoration *decoration, mDecorations )
  {
    decoration->render( mMapSettings, context );
  }

  Q_FOREACH ( QgsAnnotation *annotation, mAnnotations )
  {
    if ( isCanceled() )
      return false;

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

  if ( !mFileName.isEmpty() )
  {
    destPainter->end();

    if ( mForceRaster && mFileFormat == QStringLiteral( "PDF" ) )
    {
      QPainter pp;
      pp.begin( printer.get() );
      QRectF rect( 0, 0, img.width(), img.height() );
      pp.drawImage( rect, img, rect );
      pp.end();
    }
    else if ( mFileFormat != QStringLiteral( "PDF" ) )
    {
      bool success = img.save( mFileName, mFileFormat.toLocal8Bit().data() );
      if ( !success )
      {
        mError = ImageSaveFail;
        return false;
      }

      if ( mSaveWorldFile )
      {
        QFileInfo info  = QFileInfo( mFileName );

        // build the world file name
        QString outputSuffix = info.suffix();
        QString worldFileName = info.absolutePath() + '/' + info.baseName() + '.'
                                + outputSuffix.at( 0 ) + outputSuffix.at( info.suffix().size() - 1 ) + 'w';
        QFile worldFile( worldFileName );

        if ( worldFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) //don't use QIODevice::Text
        {
          QTextStream stream( &worldFile );
          stream << QgsMapSettingsUtils::worldFileContent( mMapSettings );
        }
      }
    }
  }

  return true;
}

void QgsMapRendererTask::finished( bool result )
{
  qDeleteAll( mAnnotations );
  mAnnotations.clear();

  if ( result )
    emit renderingComplete();
  else
    emit errorOccurred( mError );
}
