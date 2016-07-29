/***************************************************************************
                         qgscomposerlegenditemdialog.cpp
                         -------------------------------
    begin                : July 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegenditemdialog.h"
#include <QStandardItem>

QgsComposerLegendItemDialog::QgsComposerLegendItemDialog( const QStandardItem* item, QWidget* parent ): QDialog( parent )
{
  setupUi( this );

  if ( item )
  {
    mItemTextLineEdit->setText( item->text() );
  }
}

QgsComposerLegendItemDialog::QgsComposerLegendItemDialog(): QDialog( nullptr )
{

}

QgsComposerLegendItemDialog::~QgsComposerLegendItemDialog()
{

}

QString QgsComposerLegendItemDialog::itemText() const
{
  return mItemTextLineEdit->text();
}
