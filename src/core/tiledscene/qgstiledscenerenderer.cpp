/***************************************************************************
                         qgstiledscenerenderer.cpp
                         --------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenerenderer.h"

#include "qgsapplication.h"
#include "qgstiledscenerendererregistry.h"
#include "qgsunittypes.h"

#include <QThread>

//
// QgsTiledSceneRenderContext
//

QgsTiledSceneRenderContext::QgsTiledSceneRenderContext( QgsRenderContext &context, QgsFeedback *feedback )
  : mRenderContext( context )
  , mFeedback( feedback )
{

}

QImage QgsTiledSceneRenderContext::textureImage() const
{
  return mTextureImage;
}

void QgsTiledSceneRenderContext::setTextureImage( const QImage &image )
{
  mTextureImage = image;
}

void QgsTiledSceneRenderContext::setTextureCoordinates( float textureX1, float textureY1, float textureX2, float textureY2, float textureX3, float textureY3 )
{
  mTextureCoordinates[0] = textureX1;
  mTextureCoordinates[1] = textureY1;
  mTextureCoordinates[2] = textureX2;
  mTextureCoordinates[3] = textureY2;
  mTextureCoordinates[4] = textureX3;
  mTextureCoordinates[5] = textureY3;
}

void QgsTiledSceneRenderContext::textureCoordinates( float &textureX1, float &textureY1, float &textureX2, float &textureY2, float &textureX3, float &textureY3 ) const
{
  textureX1 = mTextureCoordinates[0];
  textureY1 = mTextureCoordinates[1];
  textureX2 = mTextureCoordinates[2];
  textureY2 = mTextureCoordinates[3];
  textureX3 = mTextureCoordinates[4];
  textureY3 = mTextureCoordinates[5];
}

//
// QgsTiledSceneRenderer
//

Qgis::TiledSceneRendererFlags QgsTiledSceneRenderer::flags() const
{
  return Qgis::TiledSceneRendererFlag::RendersLines | Qgis::TiledSceneRendererFlag::RendersTriangles;
}

QgsTiledSceneRenderer *QgsTiledSceneRenderer::load( QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.isNull() )
    return nullptr;

  // load renderer
  const QString rendererType = element.attribute( u"type"_s );

  QgsTiledSceneRendererAbstractMetadata *m = QgsApplication::tiledSceneRendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  std::unique_ptr< QgsTiledSceneRenderer > r( m->createRenderer( element, context ) );
  return r.release();
}

void QgsTiledSceneRenderer::startRender( QgsTiledSceneRenderContext & )
{
#ifdef QGISDEBUG
  if ( !mThread )
  {
    mThread = QThread::currentThread();
  }
  else
  {
    Q_ASSERT_X( mThread == QThread::currentThread(), "QgsTiledSceneRenderer::startRender", "startRender called in a different thread - use a cloned renderer instead" );
  }
#endif
}

void QgsTiledSceneRenderer::stopRender( QgsTiledSceneRenderContext & )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( mThread == QThread::currentThread(), "QgsTiledSceneRenderer::stopRender", "stopRender called in a different thread - use a cloned renderer instead" );
#endif
}

double QgsTiledSceneRenderer::maximumScreenError() const
{
  return mMaximumScreenError;
}

void QgsTiledSceneRenderer::setMaximumScreenError( double error )
{
  mMaximumScreenError = error;
}

Qgis::RenderUnit QgsTiledSceneRenderer::maximumScreenErrorUnit() const
{
  return mMaximumScreenErrorUnit;
}

void QgsTiledSceneRenderer::setMaximumScreenErrorUnit( Qgis::RenderUnit unit )
{
  mMaximumScreenErrorUnit = unit;
}

QList<QgsLayerTreeModelLegendNode *> QgsTiledSceneRenderer::createLegendNodes( QgsLayerTreeLayer * )
{
  return QList<QgsLayerTreeModelLegendNode *>();
}

QStringList QgsTiledSceneRenderer::legendRuleKeys() const
{
  return QStringList();
}

void QgsTiledSceneRenderer::copyCommonProperties( QgsTiledSceneRenderer *destination ) const
{
  destination->setMaximumScreenError( mMaximumScreenError );
  destination->setMaximumScreenErrorUnit( mMaximumScreenErrorUnit );
  destination->setTileBorderRenderingEnabled( mTileBorderRendering );
}

void QgsTiledSceneRenderer::restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mMaximumScreenError = element.attribute( u"maximumScreenError"_s, u"3"_s ).toDouble();
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( u"maximumScreenErrorUnit"_s, u"MM"_s ) );
  mTileBorderRendering = element.attribute( u"tileBorderRendering"_s, u"0"_s ).toInt();
}

void QgsTiledSceneRenderer::saveCommonProperties( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( u"maximumScreenError"_s, qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( u"maximumScreenErrorUnit"_s, QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
  element.setAttribute( u"tileBorderRendering"_s, mTileBorderRendering ? 1 : 0 );
}
