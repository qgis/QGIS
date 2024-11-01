/***************************************************************************
                          qgswmsrequest.cpp

  Define request class for getting request contents for WMS service
  -------------------
  begin                : 2021-02-10
  copyright            : (C) 2021 by Paul Blottiere
  email                : blottiere.paul@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsrequest.h"
#include "moc_qgswmsrequest.cpp"

namespace QgsWms
{
  QgsWmsRequest::QgsWmsRequest( const QgsServerRequest &other )
    : QgsServerRequest( other )
  {
    init();
  }

  const QgsWmsParameters &QgsWmsRequest::wmsParameters() const
  {
    return mWmsParams;
  }

  void QgsWmsRequest::setParameter( const QString &key, const QString &value )
  {
    QgsServerRequest::setParameter( key, value );
    init();
  }

  void QgsWmsRequest::removeParameter( const QString &key )
  {
    QgsServerRequest::removeParameter( key );
    init();
  }

  void QgsWmsRequest::setUrl( const QUrl &url )
  {
    QgsServerRequest::setUrl( url );
    init();
  }

  void QgsWmsRequest::init()
  {
    mWmsParams = QgsWmsParameters( serverParameters() );
  }
} // namespace QgsWms
