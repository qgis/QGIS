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
QgsDetailedItemWidget::QgsDetailedItemWidget( QWidget * parent )
    : QWidget( parent )

{
  setupUi( this );
}

QgsDetailedItemWidget::~QgsDetailedItemWidget()
{
}

void QgsDetailedItemWidget::setData( const QgsDetailedItemData& theData )
{
  lblTitle->setText( theData.title() );
  lblDetail->setText( theData.detail() );
  lblCategory->setText( theData.category() );
  cbx->setVisible( theData.isCheckable() );
  cbx->setChecked( theData.isChecked() );
  lblIcon->setPixmap( theData.icon() );
}

void QgsDetailedItemWidget::setChecked( bool theFlag )
{
  cbx->setChecked( theFlag );
}
