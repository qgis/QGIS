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


QgsGroupWMSDataDialog::QgsGroupWMSDataDialog( QWidget *parent, const Qt::WindowFlags& fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegExpValidator( QgsApplication::shortNameRegExp(), this );
  mShortNameLineEdit->setValidator( shortNameValidator );
}

QString QgsGroupWMSDataDialog::groupShortName()
{
  return mShortNameLineEdit->text();
}

void QgsGroupWMSDataDialog::setGroupShortName( const QString& shortName )
{
  mShortNameLineEdit->setText( shortName );
}

QString QgsGroupWMSDataDialog::groupTitle()
{
  return mTitleLineEdit->text();
}

void QgsGroupWMSDataDialog::setGroupTitle( const QString& title )
{
  mTitleLineEdit->setText( title );
}

QString QgsGroupWMSDataDialog::groupAbstract()
{
  return mAbstractTextEdit->toPlainText();
}

void QgsGroupWMSDataDialog::setGroupAbstract( const QString& abstract )
{
  mAbstractTextEdit->setPlainText( abstract );
}
