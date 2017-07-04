/***************************************************************************
                             qgslayoutitemregistryguiutils.h
                             -------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslayoutitemregistryguiutils.h"
#include "qgslayoutviewrubberband.h"

void QgsLayoutItemRegistryGuiUtils::setItemRubberBandPrototype( int type, QgsLayoutViewRubberBand *prototype )
{
  auto create = [prototype]( QgsLayoutView * view )->QgsLayoutViewRubberBand *
  {
    return prototype->create( view );
  };
  QgsApplication::layoutItemRegistry()->mRubberBandFunctions.insert( type, create );
}
