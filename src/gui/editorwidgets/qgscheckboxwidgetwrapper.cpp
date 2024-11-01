/***************************************************************************
    qgscheckboxwidgetwrapper.cpp
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

#include "qgscheckboxwidgetwrapper.h"
#include "moc_qgscheckboxwidgetwrapper.cpp"

QgsCheckboxWidgetWrapper::QgsCheckboxWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )

{
}


QVariant QgsCheckboxWidgetWrapper::value() const
{
  if ( config( QStringLiteral( "AllowNullState" ) ).toBool() && mCheckBox && mCheckBox->checkState() == Qt::PartiallyChecked )
  {
    return QVariant();
  }

  if ( field().type() == QMetaType::Type::Bool )
  {
    if ( mGroupBox )
    {
      return mGroupBox->isChecked();
    }
    else if ( mCheckBox )
    {
      return mCheckBox->isChecked();
    }
  }
  else
  {
    if ( mGroupBox )
    {
      return mGroupBox->isChecked() ? config( QStringLiteral( "CheckedState" ) ) : config( QStringLiteral( "UncheckedState" ) );
    }
    else if ( mCheckBox )
    {
      return mCheckBox->isChecked() ? config( QStringLiteral( "CheckedState" ) ) : config( QStringLiteral( "UncheckedState" ) );
    }
  }

  return QVariant();
}

void QgsCheckboxWidgetWrapper::showIndeterminateState()
{
  if ( mCheckBox )
  {
    mIndeterminateStateEnabled = true;
    whileBlocking( mCheckBox )->setCheckState( Qt::PartiallyChecked );
  }
}

QWidget *QgsCheckboxWidgetWrapper::createWidget( QWidget *parent )
{
  return new QCheckBox( parent );
}

void QgsCheckboxWidgetWrapper::initWidget( QWidget *editor )
{
  mCheckBox = qobject_cast<QCheckBox *>( editor );
  mGroupBox = qobject_cast<QGroupBox *>( editor );

  if ( mCheckBox )
    connect( mCheckBox, &QCheckBox::stateChanged, this, [=]( int state ) {
      if ( !mIndeterminateStateEnabled && mCheckBox->checkState() != Qt::PartiallyChecked )
      {
        mCheckBox->setTristate( false );
      }
      Q_NOWARN_DEPRECATED_PUSH
      emit valueChanged( state != Qt::Unchecked );
      Q_NOWARN_DEPRECATED_POP
      emit valuesChanged( state != Qt::Unchecked );
    } );
  if ( mGroupBox )
    connect( mGroupBox, &QGroupBox::toggled, this, [=]( bool state ) {
      Q_NOWARN_DEPRECATED_PUSH
      emit valueChanged( state );
      Q_NOWARN_DEPRECATED_POP
      emit valuesChanged( state );
    } );
}

bool QgsCheckboxWidgetWrapper::valid() const
{
  return mCheckBox || mGroupBox;
}

void QgsCheckboxWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  Qt::CheckState state = Qt::Unchecked;

  if ( config( QStringLiteral( "AllowNullState" ) ).toBool() && value.isNull() )
  {
    state = Qt::PartiallyChecked;
  }
  else
  {
    if ( field().type() == QMetaType::Type::Bool )
    {
      state = value.toBool() ? Qt::Checked : Qt::Unchecked;
    }
    else
    {
      state = value == config( QStringLiteral( "CheckedState" ) ) ? Qt::Checked : Qt::Unchecked;
    }
  }

  if ( mGroupBox )
  {
    mGroupBox->setChecked( state == Qt::Checked );
  }

  if ( mCheckBox )
  {
    mCheckBox->setTristate( state == Qt::PartiallyChecked );
    mCheckBox->setCheckState( state );
  }
}
