/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   *
*  
*        *
*                                     *
*                                                                         *
***************************************************************************/

#include "qgsapplication.h"
#include "qgsgroupwmsdatadialog.h"

#include <QRegExpValidator>

QgsGroupWmsDataDialog::QgsGroupWmsDataDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegExpValidator( QgsApplication::shortNameRegExp(), this );
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
