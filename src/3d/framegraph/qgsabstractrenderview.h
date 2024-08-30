/***************************************************************************
  qgsabstractrenderview.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTRENDERVIEW_H
#define QGSABSTRACTRENDERVIEW_H

#include "qgis_3d.h"

#include <QObject>
#include <Qt3DRender/QRenderTargetOutput>

#define SIP_NO_FILE

class QColor;
class QRect;
class QSurface;

namespace Qt3DCore
{
  class QEntity;
}

namespace Qt3DRender
{
  class QRenderSettings;
  class QCamera;
  class QFrameGraphNode;
  class QLayer;
  class QViewport;
  class QTexture2D;
  class QSubtreeEnabler;
  class QRenderTargetSelector;
}

class QgsFrameGraph;

/**
 * \ingroup 3d
 * \brief Base class for 3D render view
 *
 * An instance of QgsAbstractRenderView is a branch (ie. a render pass, ie. a render view) of in the framegraph
 *
 * Will be used with QgsFrameGraph::registerRenderView.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsAbstractRenderView : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsAbstractRenderView with the specified \a parent object.
     */
    QgsAbstractRenderView( QObject *parent, const QString &viewName );

    //! Set output to screen (ie. nullptr) or to a render target output
    virtual void setTargetOutputs( const QList<Qt3DRender::QRenderTargetOutput *> &targetOutputList );

    //! Returns the list of all target outputs
    virtual QList<Qt3DRender::QRenderTargetOutput *> targetOutputs() const;

    //! Updates texture sizes for all target outputs
    virtual void updateTargetOutputSize( int width, int height );

    //! Returns the 2D texture attached at the \a attachment point, if any
    virtual Qt3DRender::QTexture2D *outputTexture( Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment ) const;

    //! Returns the layer to be used by entities to be included in this render view
    Qt3DRender::QLayer *layerToFilter() const;

    //! Returns the viewport associated to this render view
    virtual Qt3DRender::QViewport *viewport() const;

    //! Returns the top node of this render view branch. Will be used to register the render view.
    Qt3DRender::QFrameGraphNode *topGraphNode() const;

    //! Enable or disable via \a enable the render view sub tree
    virtual void setEnabled( bool enable );

    //! Returns true if render view is enabled
    virtual bool isEnabled() const;

  protected:
    //! Handles target outputs changes
    virtual void onTargetOutputUpdate();

    //! Stores target outputs
    QList<Qt3DRender::QRenderTargetOutput *> mTargetOutputs;

    Qt3DRender::QFrameGraphNode *mRoot = nullptr;
    Qt3DRender::QSubtreeEnabler *mRendererEnabler = nullptr;
    Qt3DRender::QLayer *mLayer = nullptr;
    Qt3DRender::QRenderTargetSelector *mRenderTargetSelector = nullptr;
};

#endif // QGSABSTRACTRENDERVIEW_H
