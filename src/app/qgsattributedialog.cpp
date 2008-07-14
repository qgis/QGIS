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

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsuniquevaluerenderer.h"
#include "qgssymbol.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>

QgsAttributeDialog::QgsAttributeDialog(QgsVectorLayer *vl, QgsFeature * thepFeature)
  : QDialog(),
    mSettingsPath("/Windows/AttributeDialog/"),
    mpFeature(thepFeature),
    mLayer(vl)
{
  setupUi(this);
  if (mpFeature==NULL || vl->getDataProvider()==NULL )
    return;

  const QgsFieldMap &theFieldMap = vl->getDataProvider()->fields();

  if (theFieldMap.isEmpty()) return;

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


  int classificationField = -1;
  QStringList values;

  const QgsUniqueValueRenderer *uvr = dynamic_cast<const QgsUniqueValueRenderer *>( mLayer->renderer() );
  if( uvr )
  {
    classificationField = uvr->classificationField();

    const QList<QgsSymbol *> symbols = uvr->symbols();

    for(int i=0; i<symbols.size(); i++)
    {
      values.append( symbols[i]->lowerValue() );
    }
  }

  int index=0;
  for (QgsAttributeMap::const_iterator it = myAttributes.begin(); 
      it != myAttributes.end(); 
      ++it)
  {
    QString myFieldName = theFieldMap[it.key()].name();
    int myFieldType = theFieldMap[it.key()].type();
    QLabel * mypLabel = new QLabel();
    mypInnerLayout->addWidget(mypLabel,index,0);
    QVariant myFieldValue = it.value();

    QWidget *myWidget;
    if(classificationField!=it.key())
    {
      QLineEdit *le = new QLineEdit();

      //the provider may have provided a default value so use it
      le->setText(myFieldValue.toString());

      if( myFieldType==QVariant::Int )
      {
        le->setValidator( new QIntValidator(le) );
      }
      else if( myFieldType==QVariant::Double )
      {
        le->setValidator( new QIntValidator(le) );
      }

      myWidget = le;
    }
    else
    {
      QComboBox *cb = new QComboBox();
      cb->addItems(values);
      cb->setEditable(true);

      //the provider may have provided a default value so use it
      cb->setEditText(myFieldValue.toString());

      if( myFieldType==QVariant::Int ) {
        cb->setValidator( new QIntValidator(cb) );
      }
      else if( myFieldType==QVariant::Double )
      {
        cb->setValidator( new QIntValidator(cb) );
      }

      myWidget = cb;
    }

    if( myFieldType==QVariant::Int )
    {
      mypLabel->setText(myFieldName + tr(" (int)"));
    }
    else if( myFieldType==QVariant::Double )
    {
      mypLabel->setText(myFieldName + tr(" (dbl)"));
    }
    else //string
    {
      //any special behaviour for string goes here
      mypLabel->setText(myFieldName + tr(" (txt)"));
    }

    mypInnerLayout->addWidget(myWidget, index, 1);
    mpWidgets << myWidget;
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
    const QgsFieldMap &theFieldMap = mLayer->getDataProvider()->fields();

    //Q_ASSERT(myIndex <= mpWidgets.size());
    QString myFieldName = theFieldMap[it.key()].name();
    bool myFlag=false;
    QString myFieldValue;

    QLineEdit *le = dynamic_cast<QLineEdit *>(mpWidgets.value(myIndex));
    if(le)
    {
      myFieldValue = le->text();
    }

    QComboBox *cb = dynamic_cast<QComboBox *>(mpWidgets.value(myIndex));
    if(cb)
    {
      myFieldValue = cb->currentText();
    }

    switch( theFieldMap[it.key()].type() )
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

