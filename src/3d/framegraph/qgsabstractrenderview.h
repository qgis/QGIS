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
  class QSubtreeEnabler;
}

class QgsFrameGraph;

/**
 * \ingroup 3d
 * \brief Base class for 3D render view
 *
 * Will be used with QgsShadowRenderingFrameGraph::registerRenderView
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
    QgsAbstractRenderView( QObject *parent = nullptr );

    //! Returns the layer to be used by entities to be included in this renderview
    virtual Qt3DRender::QLayer *layerToFilter() = 0;

    //! Returns the viewport associated to this renderview
    virtual Qt3DRender::QViewport *viewport() = 0;

    //! Returns the top node of this renderview branch. Will be used to register the renderview.
    virtual Qt3DRender::QFrameGraphNode *topGraphNode() = 0;

    //! Enable or disable via \a enable the renderview sub tree
    virtual void enableSubTree( bool enable ) = 0;

    //! Returns true if renderview is enabled
    virtual bool isSubTreeEnabled() = 0;

  protected:
    std::pair<Qt3DRender::QFrameGraphNode *, Qt3DRender::QSubtreeEnabler *> createSubtreeEnabler( Qt3DRender::QFrameGraphNode *parent = nullptr );
};

#endif // QGSABSTRACTRENDERVIEW_H
