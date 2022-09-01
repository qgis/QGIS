/***************************************************************************
    qgsvaluerelationsearchwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationsearchwidgetwrapper.h"

#include "qgsfields.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsfilterlineedit.h"
#include "qgsvaluerelationwidgetwrapper.h"
#include "qgssettings.h"
#include "qgsapplication.h"

#include <QStringListModel>
#include <QCompleter>

QgsValueRelationSearchWidgetWrapper::QgsValueRelationSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, fieldIdx, parent )

{
}

bool QgsValueRelationSearchWidgetWrapper::applyDirectly()
{
  return !mLineEdit;
}

QString QgsValueRelationSearchWidgetWrapper::expression() const
{
  return mExpression;
}

QVariant QgsValueRelationSearchWidgetWrapper::value() const
{
  QVariant v;

  if ( mComboBox )
  {
    int cbxIdx = mComboBox->currentIndex();
    if ( cbxIdx > -1 )
    {
      v = mComboBox->currentData();
    }
  }

  if ( mLineEdit )
  {
    const auto constMCache = mCache;
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : constMCache )
    {
      if ( i.value == mLineEdit->text() )
      {
        v = i.key;
        break;
      }
    }
  }

  return v;
}

QgsSearchWidgetWrapper::FilterFlags QgsValueRelationSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo | NotEqualTo | IsNull | IsNotNull;
}

QgsSearchWidgetWrapper::FilterFlags QgsValueRelationSearchWidgetWrapper::defaultFlags() const
{
  return EqualTo;
}

QString QgsValueRelationSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  QString fieldName = createFieldIdentifier();

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  QVariant v = value();
  if ( !v.isValid() )
    return QString();

  switch ( v.type() )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    {
      if ( flags & EqualTo )
        return fieldName + '=' + v.toString();
      else if ( flags & NotEqualTo )
        return fieldName + "<>" + v.toString();
      break;
    }

    default:
    {
      if ( flags & EqualTo )
        return fieldName + "='" + v.toString() + '\'';
      else if ( flags & NotEqualTo )
        return fieldName + "<>'" + v.toString() + '\'';
      break;
    }
  }

  return QString();
}

void QgsValueRelationSearchWidgetWrapper::clearWidget()
{
  if ( mComboBox )
  {
    mComboBox->setCurrentIndex( 0 );
  }
  if ( mLineEdit )
  {
    mLineEdit->setText( QString() );
  }
}

void QgsValueRelationSearchWidgetWrapper::setEnabled( bool enabled )
{
  if ( mComboBox )
  {
    mComboBox->setEnabled( enabled );
  }
  if ( mLineEdit )
  {
    mLineEdit->setEnabled( enabled );
  }
}

bool QgsValueRelationSearchWidgetWrapper::valid() const
{
  return true;
}

void QgsValueRelationSearchWidgetWrapper::onValueChanged()
{
  QVariant vl = value();
  if ( !vl.isValid() )
  {
    clearExpression();
    emit valueCleared();
  }
  else
  {
    setExpression( QgsVariantUtils::isNull( vl ) ? QgsApplication::nullRepresentation() : vl.toString() );
    emit valueChanged();
  }
  emit expressionChanged( mExpression );
}

void QgsValueRelationSearchWidgetWrapper::setExpression( const QString &expression )
{
  QString exp = expression;
  QString nullValue = QgsApplication::nullRepresentation();
  QString fieldName = layer()->fields().at( mFieldIdx ).name();

  QString str;
  if ( exp == nullValue )
  {
    str = QStringLiteral( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QStringLiteral( "%1 = '%3'" )
          .arg( QgsExpression::quotedColumnRef( fieldName ),
                exp.replace( '\'', QLatin1String( "''" ) )
              );
  }
  mExpression = str;
}

QWidget *QgsValueRelationSearchWidgetWrapper::createWidget( QWidget *parent )
{
  if ( config( QStringLiteral( "AllowMulti" ) ).toBool() )
  {
    return new QgsFilterLineEdit( parent );
  }
  else if ( config( QStringLiteral( "UseCompleter" ) ).toBool() )
  {
    return new QgsFilterLineEdit( parent );
  }
  else
  {
    return new QComboBox( parent );
  }
}

void QgsValueRelationSearchWidgetWrapper::initWidget( QWidget *editor )
{
  mCache = QgsValueRelationFieldFormatter::createCache( config() );

  mComboBox = qobject_cast<QComboBox *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  if ( mComboBox )
  {
    mComboBox->addItem( tr( "Please Select" ), QVariant() ); // creates an invalid to allow selecting all features
    if ( config( QStringLiteral( "AllowNull" ) ).toBool() )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( layer()->fields().at( mFieldIdx ).type() ) );
    }

    const auto constMCache = mCache;
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : constMCache )
    {
      mComboBox->addItem( element.value, element.key );
    }

    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsValueRelationSearchWidgetWrapper::onValueChanged );
  }
  else if ( mLineEdit )
  {
    QStringList values;
    values.reserve( mCache.size() );
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : std::as_const( mCache ) )
    {
      values << i.value;
    }

    QStringListModel *m = new QStringListModel( values, mLineEdit );
    QCompleter *completer = new QCompleter( m, mLineEdit );
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );
    connect( mLineEdit, &QLineEdit::textChanged, this, &QgsValueRelationSearchWidgetWrapper::onValueChanged );
  }
}


