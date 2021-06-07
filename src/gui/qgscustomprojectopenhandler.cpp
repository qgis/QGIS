/***************************************************************************
    qgscustomprojectopenhandler.h
    ---------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomprojectopenhandler.h"
#include <QIcon>

bool QgsCustomProjectOpenHandler::createDocumentThumbnailAfterOpen() const
{
  return false;
}

QIcon QgsCustomProjectOpenHandler::icon() const
{
  return QIcon();
}
