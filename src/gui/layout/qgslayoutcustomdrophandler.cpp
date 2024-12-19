/***************************************************************************
    qgslayoutcustomdrophandler.cpp
    ------------------------------
    begin                : December 2017
    copyright            : (C) 2017 by nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutcustomdrophandler.h"
#include "moc_qgslayoutcustomdrophandler.cpp"
#include <QPointF>

QgsLayoutCustomDropHandler::QgsLayoutCustomDropHandler( QObject *parent )
  : QObject( parent )
{
}

bool QgsLayoutCustomDropHandler::handleFileDrop( QgsLayoutDesignerInterface *, const QString & )
{
  return false;
}

bool QgsLayoutCustomDropHandler::handleFileDrop( QgsLayoutDesignerInterface *, QPointF, const QString & )
{
  return false;
}

bool QgsLayoutCustomDropHandler::handlePaste( QgsLayoutDesignerInterface *, QPointF, const QMimeData *, QList<QgsLayoutItem *> & )
{
  return false;
}
