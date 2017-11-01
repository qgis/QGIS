/***************************************************************************
    qgsdoublespinbox.cpp
     --------------------------------------
    Date                 : 09.2014
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

#include <QLineEdit>
#include <QMouseEvent>
#include <QSettings>
#include <QStyle>

#include "qgsdoublespinbox.h"
#include "qgsexpression.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsfilterlineedit.h"

#define CLEAR_ICON_SIZE 16

QgsDoubleSpinBox::QgsDoubleSpinBox( QWidget *parent )
  : QDoubleSpinBox( parent )
{
  mLineEdit = new QgsSpinBoxLineEdit();

  setLineEdit( mLineEdit );

  QSize msz = minimumSizeHint();
  setMinimumSize( msz.width() + CLEAR_ICON_SIZE + 9 + frameWidth() * 2 + 2,
                  std::max( msz.height(), CLEAR_ICON_SIZE + frameWidth() * 2 + 2 ) );

  connect( mLineEdit, &QgsFilterLineEdit::cleared, this, &QgsDoubleSpinBox::clear );
  connect( this, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsDoubleSpinBox::changed );
}

void QgsDoubleSpinBox::setShowClearButton( const bool showClearButton )
{
  mShowClearButton = showClearButton;
  mLineEdit->setShowClearButton( showClearButton );
}

void QgsDoubleSpinBox::setExpressionsEnabled( const bool enabled )
{
  mExpressionsEnabled = enabled;
}

void QgsDoubleSpinBox::changeEvent( QEvent *event )
{
  QDoubleSpinBox::changeEvent( event );
  mLineEdit->setShowClearButton( shouldShowClearForValue( value() ) );
}

void QgsDoubleSpinBox::wheelEvent( QWheelEvent *event )
{
  double step = singleStep();
  if ( event->modifiers() & Qt::ControlModifier )
  {
    // ctrl modifier results in finer increments - 10% of usual step
    double newStep = step / 10;
    // but don't ever use an increment smaller than would be visible in the widget
    // i.e. if showing 2 decimals, smallest increment will be 0.01
    newStep = std::max( newStep, std::pow( 10.0, 0.0 - decimals() ) );

    setSingleStep( newStep );

    // clear control modifier before handing off event - Qt uses it for unwanted purposes
    // (*increasing* step size, whereas QGIS UX convention is that control modifier
    // results in finer changes!)
    event->setModifiers( event->modifiers() & ~Qt::ControlModifier );
  }
  QDoubleSpinBox::wheelEvent( event );
  setSingleStep( step );
}

void QgsDoubleSpinBox::paintEvent( QPaintEvent *event )
{
  mLineEdit->setShowClearButton( shouldShowClearForValue( value() ) );
  QDoubleSpinBox::paintEvent( event );
}

void QgsDoubleSpinBox::changed( double value )
{
  mLineEdit->setShowClearButton( shouldShowClearForValue( value ) );
}

void QgsDoubleSpinBox::clear()
{
  setValue( clearValue() );
}

void QgsDoubleSpinBox::setClearValue( double customValue, const QString &specialValueText )
{
  mClearValueMode = CustomValue;
  mCustomClearValue = customValue;

  if ( !specialValueText.isEmpty() )
  {
    double v = value();
    clear();
    setSpecialValueText( specialValueText );
    setValue( v );
  }
}

void QgsDoubleSpinBox::setClearValueMode( QgsDoubleSpinBox::ClearValueMode mode, const QString &clearValueText )
{
  mClearValueMode = mode;
  mCustomClearValue = 0;

  if ( !clearValueText.isEmpty() )
  {
    double v = value();
    clear();
    setSpecialValueText( clearValueText );
    setValue( v );
  }
}

double QgsDoubleSpinBox::clearValue() const
{
  if ( mClearValueMode == MinimumValue )
    return minimum();
  else if ( mClearValueMode == MaximumValue )
    return maximum();
  else
    return mCustomClearValue;
}

QString QgsDoubleSpinBox::stripped( const QString &originalText ) const
{
  //adapted from QAbstractSpinBoxPrivate::stripped
  //trims whitespace, prefix and suffix from spin box text
  QString text = originalText;
  if ( specialValueText().isEmpty() || text != specialValueText() )
  {
    int from = 0;
    int size = text.size();
    bool changed = false;
    if ( !prefix().isEmpty() && text.startsWith( prefix() ) )
    {
      from += prefix().size();
      size -= from;
      changed = true;
    }
    if ( !suffix().isEmpty() && text.endsWith( suffix() ) )
    {
      size -= suffix().size();
      changed = true;
    }
    if ( changed )
      text = text.mid( from, size );
  }

  text = text.trimmed();

  return text;
}

double QgsDoubleSpinBox::valueFromText( const QString &text ) const
{
  if ( !mExpressionsEnabled )
  {
    return QDoubleSpinBox::valueFromText( text );
  }

  QString trimmedText = stripped( text );
  if ( trimmedText.isEmpty() )
  {
    return mShowClearButton ? clearValue() : value();
  }

  return QgsExpression::evaluateToDouble( trimmedText, value() );
}

QValidator::State QgsDoubleSpinBox::validate( QString &input, int &pos ) const
{
  if ( !mExpressionsEnabled )
  {
    QValidator::State r = QDoubleSpinBox::validate( input, pos );
    return r;
  }

  return QValidator::Acceptable;
}

int QgsDoubleSpinBox::frameWidth() const
{
  return style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
}

bool QgsDoubleSpinBox::shouldShowClearForValue( const double value ) const
{
  if ( !mShowClearButton || !isEnabled() )
  {
    return false;
  }
  return value != clearValue();
}
