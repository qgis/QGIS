/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QLineEdit>
#include <QDateTime>

#include "qgsvariantdelegate.h"

QgsVariantDelegate::QgsVariantDelegate( QObject* parent )
    : QItemDelegate( parent )
{
  mBoolExp.setPattern( "true|false" );
  mBoolExp.setCaseSensitivity( Qt::CaseInsensitive );

  mByteArrayExp.setPattern( "[\\x00-\\xff]*" );
  mCharExp.setPattern( "." );
  mColorExp.setPattern( "\\(([0-9]*),([0-9]*),([0-9]*),([0-9]*)\\)" );
  mDoubleExp.setPattern( "" );
  mPointExp.setPattern( "\\((-?[0-9]*),(-?[0-9]*)\\)" );
  mRectExp.setPattern( "\\((-?[0-9]*),(-?[0-9]*),(-?[0-9]*),(-?[0-9]*)\\)" );
  mSignedIntegerExp.setPattern( "-?[0-9]*" );
  mSizeExp = mPointExp;
  mUnsignedIntegerExp.setPattern( "[0-9]*" );

  mDateExp.setPattern( "([0-9]{,4})-([0-9]{,2})-([0-9]{,2})" );
  mTimeExp.setPattern( "([0-9]{,2}):([0-9]{,2}):([0-9]{,2})" );
  mDateTimeExp.setPattern( mDateExp.pattern() + 'T' + mTimeExp.pattern() );
}

void QgsVariantDelegate::paint( QPainter* painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const
{
  if ( index.column() == 2 )
  {
    QVariant value = index.model()->data( index, Qt::UserRole );
    if ( !isSupportedType( QgsVariantDelegate::type( value ) ) )
    {
      QStyleOptionViewItem myOption = option;
      myOption.state &= ~QStyle::State_Enabled;
      QItemDelegate::paint( painter, myOption, index );
      return;
    }
  }

  QItemDelegate::paint( painter, option, index );
}

QWidget* QgsVariantDelegate::createEditor( QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index ) const
{
  Q_UNUSED( option )
  if ( index.column() != 2 )
    return nullptr;

  QVariant originalValue = index.model()->data( index, Qt::UserRole );
  if ( !isSupportedType( QgsVariantDelegate::type( originalValue ) ) )
    return nullptr;

  QLineEdit* lineEdit = new QLineEdit( parent );
  lineEdit->setFrame( false );

  QRegExp regExp;

  switch ( QgsVariantDelegate::type( originalValue ) )
  {
    case QVariant::Bool:
      regExp = mBoolExp;
      break;
    case QVariant::ByteArray:
      regExp = mByteArrayExp;
      break;
    case QVariant::Char:
      regExp = mCharExp;
      break;
    case QVariant::Color:
      regExp = mColorExp;
      break;
    case QVariant::Date:
      regExp = mDateExp;
      break;
    case QVariant::DateTime:
      regExp = mDateTimeExp;
      break;
    case QVariant::Double:
      regExp = mDoubleExp;
      break;
    case QVariant::Int:
    case QVariant::LongLong:
      regExp = mSignedIntegerExp;
      break;
    case QVariant::Point:
      regExp = mPointExp;
      break;
    case QVariant::Rect:
      regExp = mRectExp;
      break;
    case QVariant::Size:
      regExp = mSizeExp;
      break;
    case QVariant::Time:
      regExp = mTimeExp;
      break;
    case QVariant::UInt:
    case QVariant::ULongLong:
      regExp = mUnsignedIntegerExp;
      break;
    default:
      ;
  }

  if ( !regExp.isEmpty() )
  {
    QValidator* validator = new QRegExpValidator( regExp, lineEdit );
    lineEdit->setValidator( validator );
  }

  return lineEdit;
}

void QgsVariantDelegate::setEditorData( QWidget* editor,
                                        const QModelIndex& index ) const
{
  QVariant value = index.model()->data( index, Qt::UserRole );
  if ( QLineEdit* lineEdit = qobject_cast<QLineEdit* >( editor ) )
    lineEdit->setText( displayText( value ) );
}

void QgsVariantDelegate::setModelData( QWidget* editor, QAbstractItemModel* model,
                                       const QModelIndex& index ) const
{
  QLineEdit* lineEdit = qobject_cast<QLineEdit* >( editor );
  if ( !lineEdit->isModified() )
    return;

  QString text = lineEdit->text();
  const QValidator* validator = lineEdit->validator();
  if ( validator )
  {
    int pos;
    if ( validator->validate( text, pos ) != QValidator::Acceptable )
      return;
  }

  QVariant originalValue = index.model()->data( index, Qt::UserRole );
  QVariant value;

  switch ( QgsVariantDelegate::type( originalValue ) )
  {
    case QVariant::Char:
      value = text.at( 0 );
      break;
    case QVariant::Color:
      ( void )mColorExp.exactMatch( text );
      value = QColor( qMin( mColorExp.cap( 1 ).toInt(), 255 ),
                      qMin( mColorExp.cap( 2 ).toInt(), 255 ),
                      qMin( mColorExp.cap( 3 ).toInt(), 255 ),
                      qMin( mColorExp.cap( 4 ).toInt(), 255 ) );
      break;
    case QVariant::Date:
    {
      QDate date = QDate::fromString( text, Qt::ISODate );
      if ( !date.isValid() )
        return;
      value = date;
    }
    break;
    case QVariant::DateTime:
    {
      QDateTime dateTime = QDateTime::fromString( text, Qt::ISODate );
      if ( !dateTime.isValid() )
        return;
      value = dateTime;
    }
    break;
    case QVariant::Point:
      ( void )mPointExp.exactMatch( text );
      value = QPoint( mPointExp.cap( 1 ).toInt(), mPointExp.cap( 2 ).toInt() );
      break;
    case QVariant::Rect:
      ( void )mRectExp.exactMatch( text );
      value = QRect( mRectExp.cap( 1 ).toInt(), mRectExp.cap( 2 ).toInt(),
                     mRectExp.cap( 3 ).toInt(), mRectExp.cap( 4 ).toInt() );
      break;
    case QVariant::Size:
      ( void )mSizeExp.exactMatch( text );
      value = QSize( mSizeExp.cap( 1 ).toInt(), mSizeExp.cap( 2 ).toInt() );
      break;
    case QVariant::StringList:
      value = text.split( ',' );
      break;
    case QVariant::Time:
    {
      QTime time = QTime::fromString( text, Qt::ISODate );
      if ( !time.isValid() )
        return;
      value = time;
    }
    break;
    default:
      value = text;
      value.convert( QgsVariantDelegate::type( originalValue ) );
  }

  model->setData( index, displayText( value ), Qt::DisplayRole );
  model->setData( index, value, Qt::UserRole );
}

bool QgsVariantDelegate::isSupportedType( QVariant::Type type )
{
  switch ( type )
  {
    case QVariant::Bool:
    case QVariant::ByteArray:
    case QVariant::Char:
    case QVariant::Color:
    case QVariant::Date:
    case QVariant::DateTime:
    case QVariant::Double:
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Point:
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::String:
    case QVariant::StringList:
    case QVariant::Time:
    case QVariant::UInt:
    case QVariant::ULongLong:
      return true;
    default:
      return false;
  }
}

QString QgsVariantDelegate::displayText( const QVariant& value )
{
  switch ( QgsVariantDelegate::type( value ) )
  {
    case QVariant::Bool:
    case QVariant::ByteArray:
    case QVariant::Char:
    case QVariant::Double:
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::String:
    case QVariant::UInt:
    case QVariant::ULongLong:
      return value.toString();
    case QVariant::Color:
    {
      QColor color = qvariant_cast<QColor>( value );
      return QString( "(%1,%2,%3,%4)" )
             .arg( color.red() ).arg( color.green() )
             .arg( color.blue() ).arg( color.alpha() );
    }
    case QVariant::Date:
      return value.toDate().toString( Qt::ISODate );
    case QVariant::DateTime:
      return value.toDateTime().toString( Qt::ISODate );
    case QVariant::Invalid:
      return "<Invalid>";
    case QVariant::Point:
    {
      QPoint point = value.toPoint();
      return QString( "(%1,%2)" ).arg( point.x() ).arg( point.y() );
    }
    case QVariant::Rect:
    {
      QRect rect = value.toRect();
      return QString( "(%1,%2,%3,%4)" )
             .arg( rect.x() ).arg( rect.y() )
             .arg( rect.width() ).arg( rect.height() );
    }
    case QVariant::Size:
    {
      QSize size = value.toSize();
      return QString( "(%1,%2)" ).arg( size.width() ).arg( size.height() );
    }
    case QVariant::StringList:
      return value.toStringList().join( "," );
    case QVariant::Time:
      return value.toTime().toString( Qt::ISODate );
    default:
      break;
  }
  return QString( "<%1>" ).arg( value.toString() );

}

/* hack to get "real" type of a variant, because QVariant::type() almost always returns QString */
QVariant::Type QgsVariantDelegate::type( const QVariant& value )
{
  if ( value.type() == QVariant::String )
  {
    QString str = value.toString();
    QRegExp regExp;
    bool ok;

    // is this a bool (true,false)
    regExp.setPattern( "true|false" );
    regExp.setCaseSensitivity( Qt::CaseInsensitive );
    if ( regExp.indexIn( str ) != -1 )
      return QVariant::Bool;

    // is this an int?
    // perhaps we should treat as double for more flexibility
    str.toInt( &ok );
    if ( ok )
      return QVariant::Int;

    // is this a double?
    str.toDouble( &ok );
    if ( ok )
      return QVariant::Double;

  }

  // fallback to QVariant::type()
  return value.type();
}
