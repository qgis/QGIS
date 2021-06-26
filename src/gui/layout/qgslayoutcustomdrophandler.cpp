/***************************************************************************
    qgslayoutcustomdrophandler.cpp
    ------------------------------
    begin                : December 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutcustomdrophandler.h"
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
