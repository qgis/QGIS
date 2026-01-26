/***************************************************************************
                             qgsprojectitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectitem.h"

#include <QDir>

#include "moc_qgsprojectitem.cpp"

QgsProjectItem::QgsProjectItem( QgsDataItem *parent, const QString &name,
                                const QString &path, const QString &providerKey )
  : QgsDataItem( Qgis::BrowserItemType::Project, parent, name, path, providerKey )
{
  mIconName = u":/images/icons/qgis_icon.svg"_s;
  setToolTip( QDir::toNativeSeparators( path ) );
  setState( Qgis::BrowserItemState::Populated ); // no more children
}

QgsMimeDataUtils::UriList QgsProjectItem::mimeUris() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = u"project"_s;
  u.name = mName;
  u.uri = mPath;

  if ( capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {
    u.filePath = path();
  }

  return { u };
}

