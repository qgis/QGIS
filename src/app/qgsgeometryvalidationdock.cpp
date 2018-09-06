/***************************************************************************
                      qgsgeometryvalidationdock.cpp
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryvalidationdock.h"
#include "qgsgeometryvalidationmodel.h"

QgsGeometryValidationDock::QgsGeometryValidationDock( const QString &title, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
{
  setupUi( this );
}

QgsGeometryValidationModel *QgsGeometryValidationDock::geometryValidationModel() const
{
  return mGeometryValidationModel;
}

void QgsGeometryValidationDock::setGeometryValidationModel( QgsGeometryValidationModel *geometryValidationModel )
{
  mGeometryValidationModel = geometryValidationModel;
}
