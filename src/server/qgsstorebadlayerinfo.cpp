/***************************************************************************
                              qgsstorebadlayerinfo.cpp
                              ------------------------
  begin                : Jan 2019
  copyright            : (C) 2019 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstorebadlayerinfo.h"
#include <QDomElement>

void QgsStoreBadLayerInfo::handleBadLayers( const QList<QDomNode> &layers )
{
  mBadLayerIds.clear();
  QList<QDomNode>::const_iterator it = layers.constBegin();
  for ( ; it != layers.constEnd(); ++it )
  {
    if ( !it->isNull() )
    {
      const QDomElement idElem = it->firstChildElement( "id" );
      if ( !idElem.isNull() )
      {
        const QString badLayerId = idElem.text();
        mBadLayerIds.append( badLayerId );
        const QDomElement nameElem = it->firstChildElement( "layername" );
        if ( !nameElem.isNull() )
        {
          mBadLayerNames.insert( badLayerId, nameElem.text() );
        }
      }
    }
  }
}
