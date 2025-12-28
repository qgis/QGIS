/***************************************************************************
    qgshtmlutils.cpp
    ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne.trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgshtmlutils.h"

#include <QStringList>

QString QgsHtmlUtils::buildBulletList( const QStringList &values )
{
  QString s( u"<ul>"_s );

  for ( const QString &value : values )
  {
    s += u"<li>%1</li>"_s.arg( value );
  }
  s += "</ul>"_L1;

  return s;
}
