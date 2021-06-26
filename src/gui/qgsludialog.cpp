/***************************************************************************
                         qgsludialog.cpp  -  description
                             -------------------
    begin                : September 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsludialog.h"


QgsLUDialog::QgsLUDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
}

QString QgsLUDialog::lowerValue() const
{
  return mLowerEdit->text();
}

QString QgsLUDialog::upperValue() const
{
  return mUpperEdit->text();
}

void QgsLUDialog::setLowerValue( const QString &val )
{
  mLowerEdit->setText( val );
}

void QgsLUDialog::setUpperValue( const QString &val )
{
  mUpperEdit->setText( val );
}
