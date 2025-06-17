/***************************************************************************
    qgsattributesformtreeviewindicator.cpp
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributesformtreeviewindicator.h"
#include "moc_qgsattributesformtreeviewindicator.cpp"

QgsAttributesFormTreeViewIndicator::QgsAttributesFormTreeViewIndicator( QObject *parent )
  : QObject { parent }
{
}

QIcon QgsAttributesFormTreeViewIndicator::icon() const
{
  return mIcon;
}

void QgsAttributesFormTreeViewIndicator::setIcon( const QIcon &icon )
{
  mIcon = icon;
  emit changed();
}

QString QgsAttributesFormTreeViewIndicator::toolTip() const
{
  return mToolTip;
}

void QgsAttributesFormTreeViewIndicator::setToolTip( const QString &tip )
{
  mToolTip = tip;
}
