/***************************************************************************
     qgsdetailedlistwidget.cpp  -  A rich QItemWidget subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdetaileditemwidget.h"
#include "moc_qgsdetaileditemwidget.cpp"
QgsDetailedItemWidget::QgsDetailedItemWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );
}

void QgsDetailedItemWidget::setData( const QgsDetailedItemData &data )
{
  lblTitle->setText( data.title() );
  lblDetail->setText( data.detail() );
  lblCategory->setText( data.category() );
  cbx->setVisible( data.isCheckable() );
  cbx->setChecked( data.isChecked() );
  lblIcon->setPixmap( data.icon() );
}

void QgsDetailedItemWidget::setChecked( bool flag )
{
  cbx->setChecked( flag );
}
