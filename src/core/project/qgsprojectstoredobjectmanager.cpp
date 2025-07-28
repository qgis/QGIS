/***************************************************************************
    qgsprojectstoredobjectmanager.cpp
    --------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstoredobjectmanager.h"
#include "moc_qgsprojectstoredobjectmanager.cpp"
#include "qgsproject.h"

QgsProjectStoredObjectManagerBase::QgsProjectStoredObjectManagerBase( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

void QgsProjectStoredObjectManagerBase::markProjectDirty()
{
  mProject->setDirty( true );
}
