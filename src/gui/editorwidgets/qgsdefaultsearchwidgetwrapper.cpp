/***************************************************************************
    qgsdefaultsearchwidgettwrapper.cpp
     --------------------------------------
    Date                 : 31.5.2015
    Copyright            : (C) 2015 Karolina Alexiou (carolinux)
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdefaultsearchwidgetwrapper.h"

#include "qgsfield.h"
#include "qgsfieldvalidator.h"

#include <QSettings>
#include <QHBoxLayout>

QgsDefaultSearchWidgetWrapper::QgsDefaultSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mLineEdit( NULL )
    , mCheckbox( NULL )
    , mContainer( NULL )
    , mCaseString( QString( "LIKE" ) )
{
}


QString QgsDefaultSearchWidgetWrapper::expression()
{
  return mExpression;
}

void QgsDefaultSearchWidgetWrapper::setCaseString( int caseSensitiveCheckState )
{
  if ( caseSensitiveCheckState == Qt::Checked )
  {
    mCaseString = "LIKE";
  }
  else
  {
    mCaseString = "ILIKE";
  }
  // need to update also the line edit
  setExpression( mLineEdit->text() );
}

void QgsDefaultSearchWidgetWrapper::setExpression( QString exp )
{
  QVariant::Type fldType = layer()->fields()[mFieldIdx].type();
  bool numeric = ( fldType == QVariant::Int || fldType == QVariant::Double || fldType == QVariant::LongLong );

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();
  QString fieldName = layer()->fields()[mFieldIdx].name();
  QString str;
  if ( exp == nullValue )
  {
    str = QString( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QString( "%1 %2 '%3'" )
          .arg( QgsExpression::quotedColumnRef( fieldName ) )
          .arg( numeric ? "=" : mCaseString )
          .arg( numeric
                ? exp.replace( "'", "''" )
                :
                "%" + exp.replace( "'", "''" ) + "%" ); // escape quotes
  }
  mExpression = str;
  emit expressionChanged( mExpression );
}

QWidget* QgsDefaultSearchWidgetWrapper::createWidget( QWidget* parent )
{
  return new QWidget( parent );
}

bool QgsDefaultSearchWidgetWrapper::applyDirectly()
{
  return false;
}

void QgsDefaultSearchWidgetWrapper::initWidget( QWidget* widget )
{
  mContainer = widget;
  mContainer->setLayout( new QHBoxLayout() );
  mLineEdit = new QgsFilterLineEdit();
  mCheckbox = new QCheckBox( "Case sensitive" );
  mContainer->layout()->addWidget( mLineEdit );
  mContainer->layout()->addWidget( mCheckbox );
  connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( setExpression( QString ) ) );
  connect( mCheckbox, SIGNAL( stateChanged( int ) ), this, SLOT( setCaseString( int ) ) );
  mCheckbox->setChecked( Qt::Unchecked );
  mCaseString = "ILIKE";
}

bool QgsDefaultSearchWidgetWrapper::valid()
{
  return true;
}
