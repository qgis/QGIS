/***************************************************************************
    qgsattributeeditorcontext.cpp
     --------------------------------------
    Date                 : 30.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditorcontext.h"

QgsAttributeEditorContext::QgsAttributeEditorContext()
    : mVectorLayerTools( NULL )
{

}

QWidget*QgsAttributeEditorContext::proxyWidget( QgsVectorLayer* vl, int fieldIdx )
{
  return mProxyWidgets.value( vl ).value( fieldIdx );
}

void QgsAttributeEditorContext::addProxyWidgets( QgsVectorLayer* vl, QMap<int, QWidget*> proxyWidgets )
{
  mProxyWidgets[ vl ].unite( proxyWidgets );
}

void QgsAttributeEditorContext::addProxyWidget( QgsVectorLayer* vl, int idx, QWidget* widget )
{
  mProxyWidgets[ vl ].insert( idx, widget );
}


void QgsAttributeEditorContext::adjustForLayer( QgsVectorLayer* layer )
{
  mDistanceArea.setSourceCrs( layer->crs() );
}
