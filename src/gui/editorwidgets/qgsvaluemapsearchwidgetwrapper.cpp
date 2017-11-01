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

#include "qgsvaluemapsearchwidgetwrapper.h"
#include "qgstexteditconfigdlg.h"
#include "qgsvaluemapconfigdlg.h"
#include "qgsvaluemapfieldformatter.h"

#include "qgsfields.h"
#include "qgsfieldvalidator.h"

#include <QSettings>
#include <QSizePolicy>

QgsValueMapSearchWidgetWrapper::QgsValueMapSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, fieldIdx, parent )

{
}

QWidget *QgsValueMapSearchWidgetWrapper::createWidget( QWidget *parent )
{
  return new QComboBox( parent );
}

void QgsValueMapSearchWidgetWrapper::comboBoxIndexChanged( int idx )
{
  if ( mComboBox )
  {
    if ( idx == 0 )
    {
      clearExpression();
      emit valueCleared();
    }
    else
    {
      setExpression( mComboBox->itemData( idx ).toString() );
      emit valueChanged();
    }
    emit expressionChanged( mExpression );
  }
}

bool QgsValueMapSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsValueMapSearchWidgetWrapper::expression() const
{
  return mExpression;
}

bool QgsValueMapSearchWidgetWrapper::valid() const
{
  return true;
}

QgsSearchWidgetWrapper::FilterFlags QgsValueMapSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo | NotEqualTo | IsNull | IsNotNull;
}

QgsSearchWidgetWrapper::FilterFlags QgsValueMapSearchWidgetWrapper::defaultFlags() const
{
  return EqualTo;
}

QString QgsValueMapSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  //if deselect value, always pass
  if ( mComboBox->currentIndex() == 0 )
    return QString();

  //clear any unsupported flags
  flags &= supportedFlags();

  QVariant::Type fldType = layer()->fields().at( mFieldIdx ).type();
  QString fieldName = QgsExpression::quotedColumnRef( layer()->fields().at( mFieldIdx ).name() );

  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  QString currentKey = mComboBox->currentData().toString();

  switch ( fldType )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    {
      if ( flags & EqualTo )
        return fieldName + '=' + currentKey;
      else if ( flags & NotEqualTo )
        return fieldName + "<>" + currentKey;
      break;
    }

    default:
    {
      if ( flags & EqualTo )
        return fieldName + "='" + currentKey + '\'';
      else if ( flags & NotEqualTo )
        return fieldName + "<>'" + currentKey + '\'';
      break;
    }
  }

  return QString();
}

void QgsValueMapSearchWidgetWrapper::clearWidget()
{
  mComboBox->setCurrentIndex( 0 );
}

void QgsValueMapSearchWidgetWrapper::setEnabled( bool enabled )
{
  mComboBox->setEnabled( enabled );
}

void QgsValueMapSearchWidgetWrapper::initWidget( QWidget *editor )
{
  mComboBox = qobject_cast<QComboBox *>( editor );

  if ( mComboBox )
  {
    const QVariantMap cfg = config();
    QVariantMap::ConstIterator it = cfg.constBegin();
    mComboBox->addItem( tr( "Please select" ), QString() );

    while ( it != cfg.constEnd() )
    {
      if ( it.value() != QgsValueMapFieldFormatter::NULL_VALUE )
        mComboBox->addItem( it.key(), it.value() );
      ++it;
    }
    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsValueMapSearchWidgetWrapper::comboBoxIndexChanged );
  }
}

void QgsValueMapSearchWidgetWrapper::setExpression( const QString &expression )
{
  QString exp = expression;
  QString fieldName = layer()->fields().at( mFieldIdx ).name();
  QString str;

  str = QStringLiteral( "%1 = '%2'" )
        .arg( QgsExpression::quotedColumnRef( fieldName ),
              exp.replace( '\'', QLatin1String( "''" ) ) );

  mExpression = str;
}

