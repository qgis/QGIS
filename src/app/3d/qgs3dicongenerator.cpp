/***************************************************************************
    qgs3dicongenerator.cpp
    ---------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dicongenerator.h"

#include "qgs3d.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsambientocclusionrenderview.h"
#include "qgsapplication.h"
#include "qgsdepthrenderview.h"
#include "qgsfileutils.h"
#include "qgsforwardrenderview.h"
#include "qgsframegraph.h"
#include "qgshighlightsrenderview.h"
#include "qgsimageoperation.h"
#include "qgslinematerial_p.h"
#include "qgsmaterial3dhandler.h"
#include "qgsmaterialregistry.h"
#include "qgsoffscreen3dengine.h"
#include "qgssettingsentryenumflag.h"
#include "qgsshadowrenderview.h"

#include <QCryptographicHash>
#include <QDir>
#include <QPalette>
#include <QString>
#include <QTimer>
#include <Qt3DCore/QEntity>
#include <Qt3DLogic/QFrameAction>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderSettings>

#include "moc_qgs3dicongenerator.cpp"

using namespace Qt::StringLiterals;

Qgs3DIconGenerator::Qgs3DIconGenerator( QObject *parent )
  : QgsAbstractStyleEntityIconGenerator( parent )
{}

void Qgs3DIconGenerator::generateIcon( QgsStyle *style, QgsStyle::StyleEntity type, const QString &name )
{
  QIcon icon;
  const QList<QSize> sizes = iconSizes();

  switch ( type )
  {
    case QgsStyle::SymbolEntity:
    case QgsStyle::TagEntity:
    case QgsStyle::ColorrampEntity:
    case QgsStyle::SmartgroupEntity:
    case QgsStyle::TextFormatEntity:
    case QgsStyle::LabelSettingsEntity:
    case QgsStyle::LegendPatchShapeEntity:
      return;

    case QgsStyle::Symbol3DEntity:
    {
      if ( sizes.isEmpty() )
        icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + u"3d.svg"_s, QSize( 24, 24 ) );
      for ( const QSize &s : sizes )
      {
        icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + u"3d.svg"_s, s );
      }

      emit iconGenerated( type, name, icon );
      break;
    }

    case QgsStyle::MaterialSettingsEntity:
    {
      // capturing 3d thumbnails involves event looping -- we don't want to trigger an event loop immediately here,
      // as this method may have been called while painting a widget (eg a list widget showing the style model), and
      // processing the event loop while drawing a widget is a VERY BAD THING
      QMetaObject::invokeMethod( this, &Qgs3DIconGenerator::generateThumbnailForMaterial, Qt::QueuedConnection, style, name );
      break;
    }
  }
}

QImage Qgs3DIconGenerator::scaleAndCenterImage( const QImage &source, const QSize &targetSize )
{
  if ( source.isNull() || targetSize.isEmpty() )
  {
    return QImage();
  }

  const QImage scaledImage = source.scaled( targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation );
  if ( scaledImage.size() == targetSize )
  {
    return scaledImage;
  }

  QImage result( targetSize, QImage::Format_ARGB32_Premultiplied );
  result.fill( Qt::transparent );

  const int xOffset = ( targetSize.width() - scaledImage.width() ) / 2;
  const int yOffset = ( targetSize.height() - scaledImage.height() ) / 2;

  QPainter painter( &result );
  painter.setCompositionMode( QPainter::CompositionMode_Source );
  painter.drawImage( xOffset, yOffset, scaledImage );
  painter.end();

  return result;
}

void Qgs3DIconGenerator::generateThumbnailForMaterial( QgsStyle *style, const QString &name )
{
  std::unique_ptr< QgsAbstractMaterialSettings > settings( style->materialSettings( name ) );
  if ( !settings )
    return;

  const QString cacheDirPath = QgsApplication::qgisSettingsDirPath() + u"style_thumbnails"_s;
  QDir cacheDir( cacheDirPath );
  if ( !cacheDir.exists() )
  {
    cacheDir.mkpath( u"."_s );
  }

  // hash the setting's xml so we can detect if it's changed since the thumbnail was last made
  QDomDocument doc;
  QDomElement rootElement = doc.createElement( u"material"_s );
  doc.appendChild( rootElement );
  settings->writeXml( rootElement, QgsReadWriteContext() );
  const QByteArray xmlData = doc.toString().toUtf8();
  const QString hashString = QString( QCryptographicHash::hash( xmlData, QCryptographicHash::Md5 ).toHex() );

  const QString safeName = QgsFileUtils::stringToSafeFilename( name );
  const QString targetFileName = cacheDir.filePath( u"%1_%2.webp"_s.arg( safeName, hashString ) );

  // check for existing thumbnails for this material
  const QStringList filters { u"%1_*.webp"_s.arg( safeName ) };
  const QStringList existingFiles = cacheDir.entryList( filters, QDir::Files );

  QImage thumbnail;
  for ( const QString &fileName : existingFiles )
  {
    const QString filePath = cacheDir.filePath( fileName );
    if ( filePath == targetFileName )
    {
      // found previously generated thumbnail with the same hash
      thumbnail.load( filePath );
    }
    else
    {
      // cache file for this material, but the hash is out of date => remove
      QFile::remove( filePath );
    }
  }

  if ( thumbnail.isNull() )
  {
    thumbnail = renderMaterial( settings.get() );
    thumbnail.save( targetFileName, "WEBP", 97 );
  }

  const QList<QSize> sizes = iconSizes();
  QIcon icon;
  if ( sizes.isEmpty() )
  {
    const QImage scaled = scaleAndCenterImage( thumbnail, QSize( 24, 24 ) );
    icon.addPixmap( QPixmap::fromImage( scaled ) );
  }
  for ( const QSize &s : sizes )
  {
    const QImage scaled = scaleAndCenterImage( thumbnail, s );
    icon.addPixmap( QPixmap::fromImage( scaled ) );
  }

  emit iconGenerated( QgsStyle::MaterialSettingsEntity, name, icon );
}

QImage Qgs3DIconGenerator::renderMaterial( const QgsAbstractMaterialSettings *settings )
{
  QgsOffscreen3DEngine engine;
  const QSize size( 600, 600 );
  engine.setSize( size );

  // clear color -- use black
  constexpr QColor CLEAR_COLOR = QColor( 0, 0, 0 );
  engine.setClearColor( CLEAR_COLOR );
  engine.frameGraph()->setRenderCaptureEnabled( true );

  QgsMaterialContext context;
  context.setTextureFilterQuality( Qgs3D::settingTextureFilterQuality->value() );
  const QgsAbstractMaterial3DHandler *handler = Qgs3D::handlerForMaterialSettings( settings );
  if ( !handler )
    return QImage();

  const QList< QgsAbstractMaterial3DHandler::PreviewMeshType > meshTypes = handler->previewMeshTypes();

  auto root = new Qt3DCore::QEntity();
  Qt3DCore::QEntity *scene = handler->createPreviewScene( settings, meshTypes.at( 0 ).type, context, nullptr, root );
  scene->addComponent( engine.frameGraph()->forwardRenderView().renderLayer() );

  // bit of a hack to get simple line materials to show -- since we don't have a window for the material
  // to retrieve the viewport from, we just manually set one
  const QList<QgsLineMaterial *> lineMaterials = scene->findChildren<QgsLineMaterial *>();
  for ( QgsLineMaterial *lineMaterial : lineMaterials )
  {
    lineMaterial->setViewportSize( QSizeF( size.width(), size.height() ) );
    lineMaterial->setLineWidth( 6 );
  }

  // use same camera angle as interactive preview
  Qt3DRender::QCamera *camera = engine.camera();
  camera->lens()->setPerspectiveProjection( 45.0f, 1.0f, 0.1f, 100.0f );
  camera->setPosition( { 0, 0, 4 } );
  camera->setViewCenter( { 0, 0, 0 } );

  engine.setRootEntity( root );
  engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::Always );

  // same trick as we use in the 3d rendering tests to ensure that we get a non-empty image
  for ( int i = 0; i < 2; i++ )
  {
    Qt3DLogic::QFrameAction *frameAction = new Qt3DLogic::QFrameAction();
    root->addComponent( frameAction );
    QEventLoop waitLoop;
    QObject::connect( frameAction, &Qt3DLogic::QFrameAction::triggered, &waitLoop, &QEventLoop::quit );
    waitLoop.exec();
    root->removeComponent( frameAction );
    frameAction->deleteLater();
  }

  // following similar approach to Qgs3DUtils::captureSceneImage() (we can't reuse that here,
  // as it's tightly tied in with QGIS 3D classes we aren't using)
  QImage thumbnail;
  {
    QEventLoop captureLoop;
    QObject::connect( &engine, &QgsAbstract3DEngine::imageCaptured, &captureLoop, [&thumbnail, &captureLoop]( const QImage &img ) {
      thumbnail = img.convertToFormat( QImage::Format_ARGB32_Premultiplied );
      captureLoop.quit();
    } );

    engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::OnDemand );
    engine.requestCaptureImage();
    captureLoop.exec();
  }

  QImage depthThumbnail;
  {
    QEventLoop captureLoop;
    QObject::connect( &engine, &QgsAbstract3DEngine::depthBufferCaptured, &captureLoop, [&depthThumbnail, &captureLoop]( const QImage &img ) {
      depthThumbnail = img;
      captureLoop.quit();
    } );

    engine.renderSettings()->setRenderPolicy( Qt3DRender::QRenderSettings::RenderPolicy::OnDemand );
    engine.requestDepthBufferCapture();
    captureLoop.exec();
  }

  setMaximumDepthAsTransparent( depthThumbnail, thumbnail );
  return thumbnail;
}

void Qgs3DIconGenerator::setMaximumDepthAsTransparent( const QImage &depthImage, QImage &renderImage )
{
  const int width = renderImage.width();
  const int height = renderImage.height();

  constexpr QRgb maxDepthColor = qRgb( 0, 0, 255 );
  constexpr QRgb transparentPixel = qRgba( 0, 0, 0, 0 );

  for ( int y = 0; y < height; ++y )
  {
    const QRgb *maskPixels = reinterpret_cast<const QRgb *>( depthImage.constScanLine( y ) );
    QRgb *renderPixels = reinterpret_cast<QRgb *>( renderImage.scanLine( y ) );
    for ( int x = 0; x < width; ++x )
    {
      if ( maskPixels[x] == maxDepthColor )
      {
        renderPixels[x] = transparentPixel;
      }
    }
  }
}
