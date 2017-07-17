/***************************************************************************
                             qgslayoutnewitempropertiesdialog.cpp
                             ------------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutnewitempropertiesdialog.h"
#include "qgssettings.h"

QgsLayoutNewItemPropertiesDialog::QgsLayoutNewItemPropertiesDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );
  QgsSettings settings;
  double lastWidth = settings.value( QStringLiteral( "LayoutDesigner/lastItemWidth" ), QStringLiteral( "50" ) ).toDouble();
  double lastHeight = settings.value( QStringLiteral( "LayoutDesigner/lastItemHeight" ), QStringLiteral( "50" ) ).toDouble();
  mWidthSpin->setValue( lastWidth );
  mHeightSpin->setValue( lastHeight );
}

void QgsLayoutNewItemPropertiesDialog::setInitialItemPosition( QPointF position )
{
  mXPosSpin->setValue( position.x() );
  mYPosSpin->setValue( position.y() );
}

QgsLayoutPoint QgsLayoutNewItemPropertiesDialog::itemPosition() const
{
  return QgsLayoutPoint( mXPosSpin->value(), mYPosSpin->value() );
}

QgsLayoutSize QgsLayoutNewItemPropertiesDialog::itemSize() const
{
  return QgsLayoutSize( mWidthSpin->value(), mHeightSpin->value() );
}
