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


QgsLUDialog::QgsLUDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );

  connect( mLowerEdit, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ this ]( double value ) { setDecimalPlaces( mLowerEdit, value ); } );
  connect( mUpperEdit, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, [ this ]( double value ) { setDecimalPlaces( mUpperEdit, value ); } );
}

QString QgsLUDialog::lowerValue() const
{
  return mLowerEdit->text();
}

double QgsLUDialog::lowerValueDouble() const
{
  return mLowerEdit->value();
}

QString QgsLUDialog::upperValue() const
{
  return mUpperEdit->text();
}

double QgsLUDialog::upperValueDouble() const
{
  return mUpperEdit->value();
}

void QgsLUDialog::setLowerValue( const QString &val )
{
  bool ok;
  const double value { QLocale().toDouble( val, &ok )};
  mLowerEdit->setValue( value );
  if ( ok )
  {
    setDecimalPlaces( mLowerEdit, value );
  }
}

void QgsLUDialog::setUpperValue( const QString &val )
{
  bool ok;
  const double value { QLocale().toDouble( val, &ok )};
  mUpperEdit->setValue( value );
  if ( ok )
  {
    setDecimalPlaces( mUpperEdit, value );
  }
}

void QgsLUDialog::setDecimalPlaces( QgsDoubleSpinBox *widget, double value ) const
{
  const QString strVal { QVariant( value ).toString() };
  const int dotPosition( strVal.indexOf( '.' ) );
  int decimals {2};
  if ( dotPosition >= 0 )
  {
    decimals = std::max<int>( 2, strVal.length() - dotPosition - 1 );
    widget->setDecimals( decimals );
  }
}
