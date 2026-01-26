/***************************************************************************
  qgsfgutils.cpp
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Mike Krus / Benoit De Mezzo
  Email                : mike dot krus at kdab dot com / benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsframegraphutils.h"

#include <QMetaEnum>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QPointSize>
#include <Qt3DRender/QSeamlessCubemap>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/QTechnique>

QStringList QgsFrameGraphUtils::dumpSceneGraph( const Qt3DCore::QNode *node, FgDumpContext context )
{
  return dumpSG( context, node );
}

QStringList QgsFrameGraphUtils::dumpFrameGraph( const Qt3DCore::QNode *node, FgDumpContext context )
{
  return dumpFG( context, node );
}

QString QgsFrameGraphUtils::formatIdName( FgDumpContext context, quint64 id, const QString &name )
{
  QString fixedName = name.isEmpty() ? "<no_name>"_L1 : name;
  return "{%1/%2}"_L1.arg( QString::number( id - context.lowestId ) ).arg( fixedName );
}

QString QgsFrameGraphUtils::formatIdName( FgDumpContext context, const Qt3DRender::QAbstractTexture *texture )
{
  QString fixedName = texture->objectName().isEmpty() ? "<no_name>"_L1 : texture->objectName();
  return "{%1[%2]/%3"_L1
    .arg( QString::number( texture->id().id() - context.lowestId ), QString( QMetaEnum::fromType<Qt3DRender::QAbstractTexture::TextureFormat>().valueToKey( texture->format() ) ), fixedName );
}

QString QgsFrameGraphUtils::formatNode( FgDumpContext context, const Qt3DCore::QNode *node )
{
  QString res = "(%1%2)"_L1
                  .arg( QLatin1String( node->metaObject()->className() ) )
                  .arg( formatIdName( context, node->id().id(), node->objectName() ) );
  if ( !node->isEnabled() )
    res += " [D]"_L1;
  return res;
}

QString QgsFrameGraphUtils::formatList( const QStringList &lst )
{
  return QString( "[ %1 ]"_L1 ).arg( lst.join( ", "_L1 ) );
}

QString QgsFrameGraphUtils::formatLongList( const QStringList &lst, int level )
{
  QString out = formatList( lst );
  if ( out.size() < 200 )
    return out;

  out = QString( "[\n"_L1 );
  for ( QString item : lst )
  {
    item = QString( "-> %1\n" ).arg( item );
    out += item.rightJustified( item.length() + ( 1 + level ) * 2, ' ' );
  }
  QString end( "]"_L1 );
  return out + end.rightJustified( end.length() + ( 1 + level ) * 2, ' ' );
}

QString QgsFrameGraphUtils::formatField( const QString &name, const QString &value )
{
  if ( value == "<no_value>" )
    return QString( "(%1)"_L1 ).arg( name );
  return QString( "(%1:%2)"_L1 ).arg( name, value );
}

QString QgsFrameGraphUtils::dumpSGEntity( FgDumpContext context, const Qt3DCore::QEntity *node, int level )
{
  auto extractTextureParam = []( FgDumpContext context, const QVector<Qt3DRender::QParameter *> &params, QStringList &fl ) {
    for ( const auto *param : params )
    {
      if ( strstr( param->value().typeName(), "QAbstractTexture*" ) )
      {
        const Qt3DRender::QAbstractTexture *tex = param->value().value<Qt3DRender::QAbstractTexture *>();
        fl += formatField( param->name(), formatIdName( context, tex ) );
      }
    }
  };


  QString res = formatNode( context, node );
  const auto &components = node->components();
  if ( !components.isEmpty() )
  {
    QStringList componentNames;
    for ( const auto &comp : components )
    {
      QString res = formatNode( context, comp );
      QStringList fl;
      if ( const auto *textMat = qobject_cast<const Qt3DExtras::QTextureMaterial *>( comp ) )
      {
        if ( textMat->texture() )
        {
          const auto texImages = textMat->texture()->textureImages();
          for ( const auto *texImg : texImages )
          {
            fl += formatField(
              texImg->metaObject()->className(),
              formatIdName( context, texImg->id().id(), texImg->objectName() )
            );
          }
        }
      }
      if ( const auto *material = qobject_cast<const Qt3DRender::QMaterial *>( comp ) )
      {
        if ( material->effect() )
        {
          const auto techniques = material->effect()->techniques();
          for ( const auto *tech : techniques )
          {
            extractTextureParam( context, tech->parameters(), fl );
            const auto passes = tech->renderPasses();
            for ( const auto *pass : passes )
            {
              extractTextureParam( context, pass->parameters(), fl );
            }
          }
          extractTextureParam( context, material->effect()->parameters(), fl );
        }
        extractTextureParam( context, material->parameters(), fl );
        if ( !fl.empty() )
          res += formatList( fl );
      }

      componentNames << res;
    }
    res += formatLongList( componentNames, level );
  }

  return res;
}

QStringList QgsFrameGraphUtils::dumpSG( FgDumpContext context, const Qt3DCore::QNode *node, int level )
{
  QStringList reply;
  const auto *entity = qobject_cast<const Qt3DCore::QEntity *>( node );
  if ( entity )
  {
    QString res = dumpSGEntity( context, entity, level );
    reply += res.rightJustified( res.length() + level * 2, ' ' );
    level++;
  }

  const auto children = node->childNodes();
  for ( auto *child : children )
    reply += dumpSG( context, child, level );

  return reply;
}

QString QgsFrameGraphUtils::dumpFGNode( FgDumpContext context, const Qt3DRender::QFrameGraphNode *node )
{
  QString res = formatNode( context, node );

  if ( const auto *lf = qobject_cast<const Qt3DRender::QLayerFilter *>( node ) )
  {
    QStringList sl;
    const auto layers = lf->layers();
    for ( auto layer : layers )
    {
      sl += formatIdName( context, layer->id().id(), layer->objectName() );
    }

    QStringList fl;
    fl += formatField( QMetaEnum::fromType<Qt3DRender::QLayerFilter::FilterMode>().valueToKey( lf->filterMode() ), formatList( sl ) );
    res += QString( " %1" ).arg( formatList( fl ) );
  }

  else if ( const auto *cs = qobject_cast<const Qt3DRender::QCameraSelector *>( node ) )
  {
    QStringList fl;
    fl += formatField( cs->camera()->metaObject()->className(), formatIdName( context, cs->camera()->id().id(), cs->camera()->objectName() ) );
    res += QString( " %1" ).arg( formatList( fl ) );
  }

  else if ( const auto *rss = qobject_cast<const Qt3DRender::QRenderStateSet *>( node ) )
  {
    QStringList sl;
    const auto renderStates = rss->renderStates();
    for ( auto rs : renderStates )
    {
      if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QCullFace *>( rs ) )
      {
        sl += formatField( "QCullFace", QMetaEnum::fromType<Qt3DRender::QCullFace::CullingMode>().valueToKey( rs_cast->mode() ) );
      }
      else if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QBlendEquation *>( rs ) )
      {
        sl += formatField( "QBlendEquation", QMetaEnum::fromType<Qt3DRender::QBlendEquation::BlendFunction>().valueToKey( rs_cast->blendFunction() ) );
      }
      else if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QBlendEquationArguments *>( rs ) )
      {
        QStringList fl;
        fl += formatField( "sourceRgb", QMetaEnum::fromType<Qt3DRender::QBlendEquationArguments::Blending>().valueToKey( rs_cast->sourceRgb() ) );
        fl += formatField( "destinationRgb", QMetaEnum::fromType<Qt3DRender::QBlendEquationArguments::Blending>().valueToKey( rs_cast->destinationRgb() ) );
        fl += formatField( "sourceAlpha", QMetaEnum::fromType<Qt3DRender::QBlendEquationArguments::Blending>().valueToKey( rs_cast->sourceAlpha() ) );
        fl += formatField( "destinationAlpha", QMetaEnum::fromType<Qt3DRender::QBlendEquationArguments::Blending>().valueToKey( rs_cast->destinationAlpha() ) );
        fl += formatField( "bufferIndex", QString::number( rs_cast->bufferIndex() ) );

        sl += formatField( "QBlendEquationArguments", formatList( fl ) );
      }
      else if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QColorMask *>( rs ) )
      {
        QStringList fl;
        fl += formatField( "red", ( rs_cast->isRedMasked() ? "true"_L1 : "false"_L1 ) );
        fl += formatField( "green", ( rs_cast->isGreenMasked() ? "true"_L1 : "false"_L1 ) );
        fl += formatField( "blue", ( rs_cast->isBlueMasked() ? "true"_L1 : "false"_L1 ) );
        fl += formatField( "alpha", ( rs_cast->isAlphaMasked() ? "true"_L1 : "false"_L1 ) );
        sl += formatField( "QColorMask", formatList( fl ) );
      }
      else if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QDepthTest *>( rs ) )
      {
        sl += formatField( "QDepthTest", QMetaEnum::fromType<Qt3DRender::QDepthTest::DepthFunction>().valueToKey( rs_cast->depthFunction() ) );
      }
      else if ( qobject_cast<const Qt3DRender::QNoDepthMask *>( rs ) )
      {
        sl += formatField( "QNoDepthMask", "<no_value>" );
      }
      else if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QPointSize *>( rs ) )
      {
        QStringList fl;
        fl += formatField( "sizeMode", QMetaEnum::fromType<Qt3DRender::QPointSize::SizeMode>().valueToKey( rs_cast->sizeMode() ) );
        fl += formatField( "value", QString::number( rs_cast->value() ) );
        sl += formatField( "QPointSize", formatList( fl ) );
      }
      else if ( const auto *rs_cast = qobject_cast<const Qt3DRender::QPolygonOffset *>( rs ) )
      {
        QStringList fl;
        fl += formatField( "scaleFactor", QString::number( rs_cast->scaleFactor() ) );
        fl += formatField( "depthSteps", QString::number( rs_cast->depthSteps() ) );
        sl += formatField( "QPolygonOffset", formatList( fl ) );
      }
      else if ( qobject_cast<const Qt3DRender::QSeamlessCubemap *>( rs ) )
      {
        sl += formatField( "QSeamlessCubemap", "<no_value>" );
      }
    }
    res += QString( " %1" ).arg( formatList( sl ) );
  }

  else if ( const auto *rs = qobject_cast<const Qt3DRender::QRenderTargetSelector *>( node ) )
  {
    if ( rs->target() )
    {
      QStringList sl;
      const auto outputs = rs->target()->outputs();
      for ( auto output : outputs )
      {
        sl += formatField( QMetaEnum::fromType<Qt3DRender::QRenderTargetOutput::AttachmentPoint>().valueToKey( output->attachmentPoint() ), formatIdName( context, output->texture() ) );
      }
      QStringList fl;
      fl += formatField( "outputs"_L1, formatList( sl ) );
      res += QString( " %1" ).arg( formatList( fl ) );
    }
  }
  //    if (!n->isEnabled())
  //        res += " [D]"_L1;
  return res;
}

QStringList QgsFrameGraphUtils::dumpFG( FgDumpContext context, const Qt3DCore::QNode *node, int level )
{
  QStringList reply;

  const Qt3DRender::QFrameGraphNode *fgNode = qobject_cast<const Qt3DRender::QFrameGraphNode *>( node );
  if ( fgNode )
  {
    QString res = dumpFGNode( context, fgNode );
    reply += res.rightJustified( res.length() + level * 2, ' ' );
  }

  const auto children = node->childNodes();
  const int inc = fgNode ? 1 : 0;
  for ( auto *child : children )
  {
    auto *childFGNode = qobject_cast<Qt3DCore::QNode *>( child );
    if ( childFGNode )
      reply += dumpFG( context, childFGNode, level + inc );
  }

  return reply;
}
