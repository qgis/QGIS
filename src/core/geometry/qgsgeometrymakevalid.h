/***************************************************************************
  qgsgeometrymakevalid.h
  --------------------------------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYMAKEVALID_H
#define QGSGEOMETRYMAKEVALID_H

#define SIP_NO_FILE

#include <memory>

class QString;
class QgsAbstractGeometry;

//! Implementation of QgsGeometry::makeValid(). Not a public API.
std::unique_ptr< QgsAbstractGeometry > _qgis_lwgeom_make_valid( const QgsAbstractGeometry *lwgeom_in, QString &errorMessage );

#endif // QGSGEOMETRYMAKEVALID_H
