/***************************************************************************
                         qgsludialog.cpp  -  description
                             -------------------
    begin                : September 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsludialog.h"


QgsLUDialog::QgsLUDialog( QWidget *parent, const Qt::WindowFlags& fl )
    : QDialog( parent, fl )
{
  setupUi( this );
}

QgsLUDialog::~QgsLUDialog()
{

}

QString QgsLUDialog::lowerValue() const
{
  return mLowerEdit->text();
}

QString QgsLUDialog::upperValue() const
{
  return mUpperEdit->text();
}

void QgsLUDialog::setLowerValue( const QString& val )
{
  mLowerEdit->setText( val );
}

void QgsLUDialog::setUpperValue( const QString& val )
{
  mUpperEdit->setText( val );
}
