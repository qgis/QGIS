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
#include <QPointer>

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QFrameGraphNode;
  class QSubtreeEnabler;
} // namespace Qt3DRender

/**
 * \ingroup qgis_3d
 * \brief Base class for 3D render view
 *
 * An instance of QgsAbstractRenderView is a branch (ie. a render pass, ie. a render view) of the framegraph.
 * It may contains multiple render passes.
 *
 * Will be used with QgsFrameGraph::registerRenderView.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.44
 */
class _3D_EXPORT QgsAbstractRenderView
{
  public:
    /**
     * Constructor for QgsAbstractRenderView with the specified \a parent object.
     */
    QgsAbstractRenderView( const QString &viewName );

    //! Disabled copy constructor
    QgsAbstractRenderView( const QgsAbstractRenderView &other ) = delete;
    //! Disabled operator=
    QgsAbstractRenderView &operator=( const QgsAbstractRenderView &other ) = delete;

    //! Default destructor
    virtual ~QgsAbstractRenderView();

    /**
     * Called when 3D window is resized. Render views may update their textures accordingly.
     * Default implementation does nothing.
     */
    virtual void updateWindowResize( int width, int height );

    //! Returns the top node of this render view branch. Will be used to register the render view.
    QPointer<Qt3DRender::QFrameGraphNode> topGraphNode() const;

    //! Enable or disable via \a enable the render view sub tree
    virtual void setEnabled( bool enable );

    //! Returns true if render view is enabled
    virtual bool isEnabled() const;

  protected:
    QString mViewName;
    QPointer<Qt3DRender::QFrameGraphNode> mRoot;
    Qt3DRender::QSubtreeEnabler *mRendererEnabler = nullptr;
};

#endif // QGSABSTRACTRENDERVIEW_H
