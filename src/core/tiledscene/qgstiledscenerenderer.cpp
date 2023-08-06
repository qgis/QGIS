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
#include "qgsunittypes.h"
#include "qgsapplication.h"
#include "qgstiledscenerendererregistry.h"

#include <QThread>

//
// QgsTiledSceneRenderContext
//

QgsTiledSceneRenderContext::QgsTiledSceneRenderContext( QgsRenderContext &context, QgsFeedback *feedback )
  : mRenderContext( context )
  , mFeedback( feedback )
{

}

//
// QgsTiledSceneRenderer
//

QgsTiledSceneRenderer *QgsTiledSceneRenderer::load( QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.isNull() )
    return nullptr;

  // load renderer
  const QString rendererType = element.attribute( QStringLiteral( "type" ) );

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
}

void QgsTiledSceneRenderer::restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mMaximumScreenError = element.attribute( QStringLiteral( "maximumScreenError" ), QStringLiteral( "0.3" ) ).toDouble();
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "maximumScreenErrorUnit" ), QStringLiteral( "MM" ) ) );
}

void QgsTiledSceneRenderer::saveCommonProperties( QDomElement &element, const QgsReadWriteContext & ) const
{

  element.setAttribute( QStringLiteral( "maximumScreenError" ), qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( QStringLiteral( "maximumScreenErrorUnit" ), QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
}
