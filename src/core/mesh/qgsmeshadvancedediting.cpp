/***************************************************************************
  qgsmeshadvancedediting.cpp - QgsMeshAdvancedEditing

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
#include "qgsmeshadvancedediting.h"


QgsMeshAdvancedEditing::QgsMeshAdvancedEditing() {}

QgsMeshAdvancedEditing::~QgsMeshAdvancedEditing() {}

QgsTopologicalMesh::Changes QgsMeshAdvancedEditing::apply( QgsMeshEditor *meshEditor ) {}

void QgsMeshAdvancedEditing::setInputVertices( const QList<int> verticesIndexes )
{
  mInputVertices = verticesIndexes;
}

void QgsMeshAdvancedEditing::setInputFaces( const QList<int> faceIndexes )
{
  mInputFaces = faceIndexes;
}
