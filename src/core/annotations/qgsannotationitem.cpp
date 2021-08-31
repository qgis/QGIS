/***************************************************************************
                             qgsannotationitem.cpp
                             -----------------
    begin                : August 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsannotationitem.h"
#include "qgsannotationitemnode.h"

Qgis::AnnotationItemFlags QgsAnnotationItem::flags() const
{
  return Qgis::AnnotationItemFlags();
}

QList<QgsAnnotationItemNode> QgsAnnotationItem::nodes() const
{
  return {};
}
