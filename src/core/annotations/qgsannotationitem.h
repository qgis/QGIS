/***************************************************************************
    qgsannotationitem.h
    ----------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONITEM_H
#define QGSANNOTATIONITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrendercontext.h"

class QgsFeedback;

class CORE_EXPORT QgsAnnotationItem
{
  public:

    QgsAnnotationItem *clone() { return nullptr; }

    QgsCoordinateReferenceSystem crs() const { return QgsCoordinateReferenceSystem(); }

    void render( QgsRenderContext &context, QgsFeedback *feedback ) {}

    int zIndex() const { return 0; }

};

#endif // QGSANNOTATIONITEM_H
