/***************************************************************************
  qgsmesheditingdelaunaytriangulation.h - QgsMeshEditingDelaunayTriangulation

 ---------------------
 begin                : 9.7.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESHEDITINGDELAUNAYTRIANGULATION_H
#define QGSMESHEDITINGDELAUNAYTRIANGULATION_H


#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsmeshadvancedediting.h"

class ANALYSIS_EXPORT QgsMeshEditingDelaunayTriangulation : public QgsMeshAdvancedEditing
{
  public:
    QgsMeshEditingDelaunayTriangulation();
    ~QgsMeshEditingDelaunayTriangulation();

    QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) override; SIP_SKIP
};

#endif // QGSMESHEDITINGDELAUNAYTRIANGULATION_H
