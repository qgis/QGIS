/***************************************************************************
    qgsspinbox.cpp
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

#include "qgsspinbox.h"
#include "qgsexpression.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsfilterlineedit.h"

#define CLEAR_ICON_SIZE 16

// This is required because private implementation of
// QAbstractSpinBoxPrivate checks for specialText emptiness
// and skips specialText handling if it's empty
#ifdef _MSC_VER
static QChar SPECIAL_TEXT_WHEN_EMPTY = QChar( 0x2063 );
#else
static constexpr QChar SPECIAL_TEXT_WHEN_EMPTY = QChar( 0x2063 );
#endif

QgsSpinBox::QgsSpinBox( QWidget *parent )
  : QSpinBox( parent )
{
  mLineEdit = new QgsSpinBoxLineEdit();
  setLineEdit( mLineEdit );

  const QSize msz = minimumSizeHint();
  setMinimumSize( msz.width() + CLEAR_ICON_SIZE + 9 + frameWidth() * 2 + 2,
                  std::max( msz.height(), CLEAR_ICON_SIZE + frameWidth() * 2 + 2 ) );

  connect( mLineEdit, &QgsFilterLineEdit::cleared, this, &QgsSpinBox::clear );
  connect( this, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsSpinBox::changed );
}

void QgsSpinBox::setShowClearButton( const bool showClearButton )
{
  mShowClearButton = showClearButton;
  mLineEdit->setShowClearButton( showClearButton );
}

void QgsSpinBox::setExpressionsEnabled( const bool enabled )
{
  mExpressionsEnabled = enabled;
}

void QgsSpinBox::changeEvent( QEvent *event )
{
  QSpinBox::changeEvent( event );

  if ( event->type() == QEvent::FontChange )
  {
    lineEdit()->setFont( font() );
  }

  mLineEdit->setShowClearButton( shouldShowClearForValue( value() ) );
}

void QgsSpinBox::paintEvent( QPaintEvent *event )
{
  mLineEdit->setShowClearButton( shouldShowClearForValue( value() ) );
  QSpinBox::paintEvent( event );
}

void QgsSpinBox::wheelEvent( QWheelEvent *event )
{
  const int step = singleStep();
  if ( event->modifiers() & Qt::ControlModifier )
  {
    // ctrl modifier results in finer increments - 10% of usual step
    int newStep = step / 10;
    // step should be at least 1
    newStep = std::max( newStep, 1 );

    setSingleStep( newStep );

    // clear control modifier before handing off event - Qt uses it for unwanted purposes
    // (*increasing* step size, whereas QGIS UX convention is that control modifier
    // results in finer changes!)
    event->setModifiers( event->modifiers() & ~Qt::ControlModifier );
  }
  QSpinBox::wheelEvent( event );
  setSingleStep( step );
}

void QgsSpinBox::timerEvent( QTimerEvent *event )
{
  // Process all events, which may include a mouse release event
  // Only allow the timer to trigger additional value changes if the user
  // has in fact held the mouse button, rather than the timer expiry
  // simply appearing before the mouse release in the event queue
  qApp->processEvents();
  if ( QApplication::mouseButtons() & Qt::LeftButton )
    QSpinBox::timerEvent( event );
}

void QgsSpinBox::changed( int value )
{
  mLineEdit->setShowClearButton( shouldShowClearForValue( value ) );
}

void QgsSpinBox::clear()
{
  setValue( clearValue() );
  if ( mLineEdit->isNull() )
    mLineEdit->clear();
}

void QgsSpinBox::setClearValue( int customValue, const QString &specialValueText )
{
  mClearValueMode = CustomValue;
  mCustomClearValue = customValue;

  if ( !specialValueText.isEmpty() )
  {
    const int v = value();
    clear();
    setSpecialValueText( specialValueText );
    setValue( v );
  }
}

void QgsSpinBox::setClearValueMode( QgsSpinBox::ClearValueMode mode, const QString &specialValueText )
{
  mClearValueMode = mode;
  mCustomClearValue = 0;

  if ( !specialValueText.isEmpty() )
  {
    const int v = value();
    clear();
    setSpecialValueText( specialValueText );
    setValue( v );
  }
}

int QgsSpinBox::clearValue() const
{
  if ( mClearValueMode == MinimumValue )
    return minimum();
  else if ( mClearValueMode == MaximumValue )
    return maximum();
  else
    return mCustomClearValue;
}

void QgsSpinBox::setLineEditAlignment( Qt::Alignment alignment )
{
  mLineEdit->setAlignment( alignment );
}

void QgsSpinBox::setSpecialValueText( const QString &txt )
{
  if ( txt.isEmpty() )
  {
    QSpinBox::setSpecialValueText( SPECIAL_TEXT_WHEN_EMPTY );
    mLineEdit->setNullValue( SPECIAL_TEXT_WHEN_EMPTY );
  }
  else
  {
    QSpinBox::setSpecialValueText( txt );
    mLineEdit->setNullValue( txt );
  }
}

int QgsSpinBox::valueFromText( const QString &text ) const
{
  if ( !mExpressionsEnabled )
  {
    return QSpinBox::valueFromText( text );
  }

  const QString trimmedText = stripped( text );
  if ( trimmedText.isEmpty() )
  {
    return mShowClearButton ? clearValue() : value();
  }

  return std::round( QgsExpression::evaluateToDouble( trimmedText, value() ) );
}

QValidator::State QgsSpinBox::validate( QString &input, int &pos ) const
{
  if ( !mExpressionsEnabled )
  {
    const QValidator::State r = QSpinBox::validate( input, pos );
    return r;
  }

  return QValidator::Acceptable;
}

int QgsSpinBox::frameWidth() const
{
  return style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
}

bool QgsSpinBox::shouldShowClearForValue( const int value ) const
{
  if ( !mShowClearButton || !isEnabled() )
  {
    return false;
  }
  return value != clearValue();
}

QString QgsSpinBox::stripped( const QString &originalText ) const
{
  //adapted from QAbstractSpinBoxPrivate::stripped
  //trims whitespace, prefix and suffix from spin box text
  QString text = originalText;
  if ( specialValueText().isEmpty() || text != specialValueText() )
  {
    // Strip SPECIAL_TEXT_WHEN_EMPTY
    if ( text.contains( SPECIAL_TEXT_WHEN_EMPTY ) )
      text = text.replace( SPECIAL_TEXT_WHEN_EMPTY, QString() );
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
