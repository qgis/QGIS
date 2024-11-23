/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsapplication.h"
#include "qgsgroupwmsdatadialog.h"
#include "moc_qgsgroupwmsdatadialog.cpp"

#include <QRegularExpressionValidator>

QgsGroupWmsDataDialog::QgsGroupWmsDataDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegularExpressionValidator( QgsApplication::shortNameRegularExpression(), this );
  mShortNameLineEdit->setValidator( shortNameValidator );
}

QString QgsGroupWmsDataDialog::groupShortName()
{
  return mShortNameLineEdit->text();
}

void QgsGroupWmsDataDialog::setGroupShortName( const QString &shortName )
{
  mShortNameLineEdit->setText( shortName );
}

QString QgsGroupWmsDataDialog::groupTitle()
{
  return mTitleLineEdit->text();
}

void QgsGroupWmsDataDialog::setGroupTitle( const QString &title )
{
  mTitleLineEdit->setText( title );
}

QString QgsGroupWmsDataDialog::groupAbstract()
{
  return mAbstractTextEdit->toPlainText();
}

void QgsGroupWmsDataDialog::setGroupAbstract( const QString &abstract )
{
  mAbstractTextEdit->setPlainText( abstract );
}
