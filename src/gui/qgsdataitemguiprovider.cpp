/***************************************************************************
  qgsdataitemguiprovider.cpp
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiprovider.h"

//
// QgsDataItemGuiContext
//

QgsMessageBar *QgsDataItemGuiContext::messageBar()
{
  return mMessageBar;
}

void QgsDataItemGuiContext::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}


//
// QgsDataItemGuiProvider
//

void QgsDataItemGuiProvider::populateContextMenu( const QList<QgsDataItem *> &, QMenu *, QgsDataItemGuiContext )
{

}
