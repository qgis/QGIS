/***************************************************************************
                            qgshistoryprovider.cpp
                            -------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgshistoryprovider.h"
#include "moc_qgshistoryprovider.cpp"

QgsAbstractHistoryProvider::~QgsAbstractHistoryProvider() = default;

QgsHistoryEntryNode *QgsAbstractHistoryProvider::createNodeForEntry( const QgsHistoryEntry &, const QgsHistoryWidgetContext & )
{
  return nullptr;
}

void QgsAbstractHistoryProvider::updateNodeForEntry( QgsHistoryEntryNode *, const QgsHistoryEntry &, const QgsHistoryWidgetContext & )
{
}
