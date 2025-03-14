/***************************************************************************
  qgsabstractrenderview.cpp
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

#include "qgsabstractrenderview.h"
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/qsubtreeenabler.h>

QgsAbstractRenderView::QgsAbstractRenderView( const QString &viewName )
{
  mViewName = viewName;
  // in order to avoid a render pass on the render view, we add a NoDraw node
  // which is disabled when the enabler is enabled, and vice versa
  using namespace Qt3DRender;
  mRoot = new QNoDraw;
  mRoot->setEnabled( false );
  mRoot->setObjectName( viewName + "::NoDraw" );
  mRendererEnabler = new QSubtreeEnabler( mRoot );
  mRendererEnabler->setEnablement( QSubtreeEnabler::Persistent );
  mRendererEnabler->setObjectName( viewName + "::SubtreeEnabler" );
}

QgsAbstractRenderView::~QgsAbstractRenderView()
{
  // if mRoot is still defined and not attached to a parent we have the responsibility to delete it
  if ( !mRoot.isNull() && mRoot->parent() == nullptr )
  {
    delete mRoot.data();
    mRoot.clear();
  }
}

void QgsAbstractRenderView::updateWindowResize( int, int )
{
  // noop
}

QPointer<Qt3DRender::QFrameGraphNode> QgsAbstractRenderView::topGraphNode() const
{
  return mRoot;
}

void QgsAbstractRenderView::setEnabled( bool enable )
{
  mRoot->setEnabled( !enable );
  mRendererEnabler->setEnabled( enable );
}

bool QgsAbstractRenderView::isEnabled() const
{
  return !mRoot->isEnabled() && mRendererEnabler->isEnabled();
}
