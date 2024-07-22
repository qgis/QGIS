/***************************************************************************
  qgsfgutils.h
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

#ifndef QGSFG_UTILS_H
#define QGSFG_UTILS_H

#include <QWindow>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QDebugOverlay>


#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Util class to dump Qt3D framegraph or scenegraph.
 * \since QGIS 3.40
 */
class QgsFgUtils
{
  public:
    struct FgDumpContext
    {
        quint64 lowestId = 0;
    };

    //! Returns a tree view of the scene graph starting from \a node. The object ids will be given relatively to the \a context lowestId.
    static QStringList dumpSceneGraph( const Qt3DCore::QNode *node, FgDumpContext context );

    //! Returns a tree view of the frame graph starting from \a node. The object ids will be given relatively to the \a context lowestId.
    static QStringList dumpFrameGraph( const Qt3DCore::QNode *node, FgDumpContext context );

  private:
    static QString formatIdName( FgDumpContext context, quint64 id, const QString &name );

    static QString formatIdName( FgDumpContext context, const Qt3DRender::QAbstractTexture *texture );

    static QString formatNode( FgDumpContext context, const Qt3DCore::QNode *n );

    static QString formatList( const QStringList &lst );

    static QString formatLongList( const QStringList &lst, int level );

    static QString formatField( const QString &name, const QString &value );

    static QString dumpSGEntity( FgDumpContext context, const Qt3DCore::QEntity *n, int level );

    static QStringList dumpSG( FgDumpContext context, const Qt3DCore::QNode *n, int level = 0 );

    static QString dumpFGNode( FgDumpContext context, const Qt3DRender::QFrameGraphNode *n );

    static QStringList dumpFG( FgDumpContext context, const Qt3DCore::QNode *n, int level = 0 );
};

#endif // QGSFG_UTILS_H
