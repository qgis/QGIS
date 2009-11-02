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
#include "qgsattributeeditor.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature )
    : QDialog(),
    mSettingsPath( "/Windows/AttributeDialog/" ),
    mLayer( vl ),
    mpFeature( thepFeature )
{
  setupUi( this );
  if ( mpFeature == NULL || vl->dataProvider() == NULL )
    return;

  const QgsFieldMap &theFieldMap = vl->pendingFields();

  if ( theFieldMap.isEmpty() ) return;

  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  //
  //Set up dynamic inside a scroll box
  //
  QVBoxLayout * mypOuterLayout = new QVBoxLayout();
  mypOuterLayout->setContentsMargins( 0, 0, 0, 0 );
  //transfers layout ownership so no need to call delete
  mFrame->setLayout( mypOuterLayout );
  QScrollArea * mypScrollArea = new QScrollArea();
  //transfers scroll area ownership so no need to call delete
  mypOuterLayout->addWidget( mypScrollArea );
  QFrame * mypInnerFrame = new QFrame();
  mypInnerFrame->setFrameShape( QFrame::NoFrame );
  mypInnerFrame->setFrameShadow( QFrame::Plain );
  //transfers frame ownership so no need to call delete
  mypScrollArea->setWidget( mypInnerFrame );
  mypScrollArea->setWidgetResizable( true );
  QGridLayout * mypInnerLayout = new QGridLayout( mypInnerFrame );



  int index = 0;
  for ( QgsAttributeMap::const_iterator it = myAttributes.begin();
        it != myAttributes.end();
        ++it )
  {
    const QgsField &field = theFieldMap[it.key()];

    //show attribute alias if available
    QString myFieldName = vl->attributeDisplayName( it.key() );
    int myFieldType = field.type();

    QLabel * mypLabel = new QLabel();
    mypInnerLayout->addWidget( mypLabel, index, 0 );
    if ( myFieldType == QVariant::Int )
    {
      mypLabel->setText( myFieldName + tr( " (int)" ) );
    }
    else if ( myFieldType == QVariant::Double )
    {
      mypLabel->setText( myFieldName + tr( " (dbl)" ) );
    }
    else //string
    {
      //any special behaviour for string goes here
      mypLabel->setText( myFieldName + tr( " (txt)" ) );
    }

    QWidget *myWidget = QgsAttributeEditor::createAttributeEditor( 0, vl, it.key(), it.value() );

    mypInnerLayout->addWidget( myWidget, index, 1 );
    mpWidgets << myWidget;
    ++index;
  }
  // Set focus to first widget in list, to help entering data without moving the mouse.
  if ( mpWidgets.size() > 0 )
  {
    mpWidgets.first()->setFocus( Qt::OtherFocusReason );
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
  int myIndex = 0;
  for ( QgsAttributeMap::const_iterator it = myAttributes.begin();
        it != myAttributes.end();
        ++it )
  {
    QVariant value;

    if ( QgsAttributeEditor::retrieveValue( mpWidgets.value( myIndex ), mLayer, it.key(), value ) )
      mpFeature->changeAttribute( it.key(), value );

    ++myIndex;
  }
  QDialog::accept();
}

void QgsAttributeDialog::saveGeometry()
{
  QSettings settings;
  settings.setValue( mSettingsPath + "geometry", QDialog::saveGeometry() );
}

void QgsAttributeDialog::restoreGeometry()
{
  QSettings settings;
  QDialog::restoreGeometry( settings.value( mSettingsPath + "geometry" ).toByteArray() );
}
