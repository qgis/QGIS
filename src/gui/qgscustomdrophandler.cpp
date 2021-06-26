/***************************************************************************
    qgscustomdrophandler.cpp
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomdrophandler.h"

QString QgsCustomDropHandler::customUriProviderKey() const
{
  return QString();
}

void QgsCustomDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  Q_UNUSED( uri )
}

bool QgsCustomDropHandler::canHandleMimeData( const QMimeData * )
{
  return false;
}

void QgsCustomDropHandler::handleMimeData( const QMimeData *data )
{
  Q_UNUSED( data )
}

bool QgsCustomDropHandler::handleMimeDataV2( const QMimeData * )
{
  return false;
}

bool QgsCustomDropHandler::handleFileDrop( const QString &file )
{
  Q_UNUSED( file )
  return false;
}

bool QgsCustomDropHandler::canHandleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &, QgsMapCanvas * )
{
  return false;
}

bool QgsCustomDropHandler::handleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &, QgsMapCanvas * ) const
{
  return false;
}
