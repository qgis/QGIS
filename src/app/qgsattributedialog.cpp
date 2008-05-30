/***************************************************************************
                         qgsattributedialog.cpp  -  description
                             -------------------
    begin                : October 2004
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
/* $Id$ */
#include "qgsattributedialog.h"
#include "qgsfield.h"
#include "qgslogger.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QScrollArea>

QgsAttributeDialog::QgsAttributeDialog(
    QgsFieldMap theFieldMap, QgsFeature * thepFeature)
  : QDialog(),
    mSettingsPath("/Windows/AttributeDialog/"),
    mFieldMap(theFieldMap),
    mpFeature(thepFeature)
{

  setupUi(this);
  if (mpFeature==NULL) return;
  if (mFieldMap.isEmpty()) return;
  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  //
  //Set up dynamic inside a scroll box
  //
  QVBoxLayout * mypOuterLayout = new QVBoxLayout();
  mypOuterLayout->setContentsMargins(0,0,0,0);
  //transfers layout ownership so no need to call delete
  mFrame->setLayout(mypOuterLayout);
  QScrollArea * mypScrollArea = new QScrollArea();
  //transfers scroll area ownership so no need to call delete
  mypOuterLayout->addWidget(mypScrollArea);
  QFrame * mypInnerFrame = new QFrame();
  mypInnerFrame->setFrameShape(QFrame::NoFrame);
  mypInnerFrame->setFrameShadow(QFrame::Plain);
  //transfers frame ownership so no need to call delete
  mypScrollArea->setWidget(mypInnerFrame);
  mypScrollArea->setWidgetResizable( true );
  QGridLayout * mypInnerLayout = new QGridLayout(mypInnerFrame);

  int index=0;
  for (QgsAttributeMap::const_iterator it = myAttributes.begin(); 
      it != myAttributes.end(); 
      ++it)
  {
    QString myFieldName = mFieldMap[it.key()].name();
    QLabel * mypLabel = new QLabel();
    mypInnerLayout->addWidget(mypLabel,index,0);
    QVariant myFieldValue = it.value();
    QLineEdit * mypLineEdit = new QLineEdit();
    //the provider may have provided a default value so use it
    mypLineEdit->setText(myFieldValue.toString());
    if( mFieldMap[it.key()].type()==QVariant::Int )
    {
      mypLineEdit->setValidator( new QIntValidator(mypLineEdit) );
      mypLabel->setText(myFieldName + tr(" (int)"));
    }
    else if( mFieldMap[it.key()].type()==QVariant::Double )
    {
      mypLineEdit->setValidator( new QDoubleValidator(mypLineEdit) );
      mypLabel->setText(myFieldName + tr(" (dbl)"));
    }
    else //string
    {
      //any special behaviour for string goes here
      mypLabel->setText(myFieldName + tr(" (txt)"));
    }
    mypInnerLayout->addWidget(mypLineEdit,index,1);
    mpWidgets << mypLineEdit;
    ++index;
  }
  restoreGeometry();
}


QgsAttributeDialog::~QgsAttributeDialog()
{
  saveGeometry();
}
  
void QgsAttributeDialog::accept()
{
  //write the new values back to the feature
  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  int myIndex=0;
  for (QgsAttributeMap::const_iterator it = myAttributes.begin(); 
      it != myAttributes.end(); 
      ++it)
  {
    //Q_ASSERT(myIndex <= mpWidgets.size());
    QString myFieldName = mFieldMap[it.key()].name();
    bool myFlag=false;
    QString myFieldValue = 
      dynamic_cast<QLineEdit *>(mpWidgets.value(myIndex))->text();
    switch( mFieldMap[it.key()].type() )
    {
      case QVariant::Int:
        {
          int myIntValue = myFieldValue.toInt(&myFlag);
          if (myFlag && ! myFieldValue.isEmpty())
          {
            mpFeature->changeAttribute( it.key(), QVariant(myIntValue) );
          }
          else
          {
            mpFeature->changeAttribute( it.key(), QVariant(QString::null) );
          }
        }
        break;
      case QVariant::Double:
        {
          double myDblValue = myFieldValue.toDouble(&myFlag);
          if (myFlag && ! myFieldValue.isEmpty())
          {
            mpFeature->changeAttribute( it.key(), QVariant(myDblValue) );
          }
          else
          {
            mpFeature->changeAttribute( it.key(), QVariant(QString::null) );
          }
        }
        break;
      default: //string
        mpFeature->changeAttribute(it.key(),QVariant( myFieldValue ) );
        break;
    }
    ++myIndex;
  }
  QDialog::accept();
}

void QgsAttributeDialog::saveGeometry()
{
  QSettings settings;
  settings.setValue(mSettingsPath+"geometry", QDialog::saveGeometry());
}

void QgsAttributeDialog::restoreGeometry()
{
  QSettings settings;
  QDialog::restoreGeometry(settings.value(mSettingsPath+"geometry").toByteArray());
}

