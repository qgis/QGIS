/***************************************************************************
  qgsmeshadvancedediting.h - QgsMeshAdvancedEditing

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
#ifndef QGSMESHADVANCEDEDITING_H
#define QGSMESHADVANCEDEDITING_H


#include "qgis_core.h"
#include "qgstopologicalmesh.h"
#include "qgstriangularmesh.h"

class QgsMeshEditor;


class CORE_EXPORT QgsMeshAdvancedEditing// : protected QgsTopologicalMesh::Changes
{
  public:
    QgsMeshAdvancedEditing();
    virtual ~QgsMeshAdvancedEditing();
    virtual QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ); SIP_SKIP

    void setInputVertices( const QList<int> verticesIndexes );

    void setInputFaces( const QList<int> faceIndexes );

    bool isFinished() const {return mIsFinished;}

  protected:
    bool mIsFinished = false;
    QList<int> mInputVertices;
    QList<int> mInputFaces;

};


#endif // QGSMESHADVANCEDEDITING_H
