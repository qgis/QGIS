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

#include <QComboBox>
#include <QLineEdit>
#include <QDateTime>
#include <QRegularExpressionMatch>
#include <QRegularExpressionValidator>

#include "qgsvariantdelegate.h"
#include "moc_qgsvariantdelegate.cpp"

QgsVariantDelegate::QgsVariantDelegate( QObject *parent )
  : QItemDelegate( parent )
{
  mBoolExp.setPattern( QStringLiteral( "true|false" ) );
  mBoolExp.setPatternOptions( QRegularExpression::CaseInsensitiveOption );

  mByteArrayExp.setPattern( QStringLiteral( "[\\x00-\\xff]*" ) );
  mCharExp.setPattern( QStringLiteral( "." ) );
  mColorExp.setPattern( QRegularExpression::anchoredPattern( QStringLiteral( "\\(([0-9]*),([0-9]*),([0-9]*),([0-9]*)\\)" ) ) );
  mDoubleExp.setPattern( QString() );
  mPointExp.setPattern( QRegularExpression::anchoredPattern( QStringLiteral( "\\((-?[0-9]*),(-?[0-9]*)\\)" ) ) );
  mRectExp.setPattern( QRegularExpression::anchoredPattern( QStringLiteral( "\\((-?[0-9]*),(-?[0-9]*),(-?[0-9]*),(-?[0-9]*)\\)" ) ) );
  mSignedIntegerExp.setPattern( QStringLiteral( "-?[0-9]*" ) );
  mSizeExp = mPointExp;
  mUnsignedIntegerExp.setPattern( QStringLiteral( "[0-9]*" ) );

  mDateExp.setPattern( QStringLiteral( "([0-9]{,4})-([0-9]{,2})-([0-9]{,2})" ) );
  mTimeExp.setPattern( QStringLiteral( "([0-9]{,2}):([0-9]{,2}):([0-9]{,2})" ) );
  mDateTimeExp.setPattern( mDateExp.pattern() + 'T' + mTimeExp.pattern() );
}

void QgsVariantDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == 2 )
  {
    const QVariant value = index.model()->data( index, Qt::UserRole );
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

QWidget *QgsVariantDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  if ( index.column() != 2 )
    return nullptr;

  const QVariant originalValue = index.model()->data( index, Qt::UserRole );
  if ( !isSupportedType( QgsVariantDelegate::type( originalValue ) ) )
    return nullptr;

  QRegularExpression regExp;
  switch ( QgsVariantDelegate::type( originalValue ) )
  {
    case QMetaType::Type::QByteArray:
      regExp = mByteArrayExp;
      break;
    case QMetaType::Type::QChar:
      regExp = mCharExp;
      break;
    case QMetaType::Type::QColor:
      regExp = mColorExp;
      break;
    case QMetaType::Type::QDate:
      regExp = mDateExp;
      break;
    case QMetaType::Type::QDateTime:
      regExp = mDateTimeExp;
      break;
    case QMetaType::Type::Double:
      regExp = mDoubleExp;
      break;
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
      regExp = mSignedIntegerExp;
      break;
    case QMetaType::Type::QPoint:
      regExp = mPointExp;
      break;
    case QMetaType::Type::QRect:
      regExp = mRectExp;
      break;
    case QMetaType::Type::QSize:
      regExp = mSizeExp;
      break;
    case QMetaType::Type::QTime:
      regExp = mTimeExp;
      break;
    case QMetaType::Type::UInt:
    case QMetaType::Type::ULongLong:
      regExp = mUnsignedIntegerExp;
      break;
    default:;
  }

  if ( QgsVariantDelegate::type( originalValue ) == QMetaType::Type::Bool )
  {
    QComboBox *comboBox = new QComboBox( parent );
    comboBox->addItem( QStringLiteral( "false" ) );
    comboBox->addItem( QStringLiteral( "true" ) );
    return comboBox;
  }
  else
  {
    QLineEdit *lineEdit = new QLineEdit( parent );
    lineEdit->setFrame( false );
    if ( !regExp.pattern().isEmpty() )
    {
      QValidator *validator = new QRegularExpressionValidator( regExp, lineEdit );
      lineEdit->setValidator( validator );
    }
    return lineEdit;
  }
}

void QgsVariantDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const QVariant value = index.model()->data( index, Qt::UserRole );

  if ( QComboBox *comboBox = qobject_cast<QComboBox *>( editor ) )
  {
    comboBox->setCurrentIndex( value.toBool() ? 1 : 0 );
  }
  else if ( QLineEdit *lineEdit = qobject_cast<QLineEdit *>( editor ) )
  {
    lineEdit->setText( displayText( value ) );
  }
}

void QgsVariantDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  const QVariant originalValue = index.model()->data( index, Qt::UserRole );
  QVariant value;

  if ( QComboBox *comboBox = qobject_cast<QComboBox *>( editor ) )
  {
    value = comboBox->currentIndex() == 1;
  }
  else if ( QLineEdit *lineEdit = qobject_cast<QLineEdit *>( editor ) )
  {
    if ( !lineEdit->isModified() )
      return;

    QString text = lineEdit->text();
    const QValidator *validator = lineEdit->validator();
    if ( validator )
    {
      int pos;
      if ( validator->validate( text, pos ) != QValidator::Acceptable )
        return;
    }

    switch ( QgsVariantDelegate::type( originalValue ) )
    {
      case QMetaType::Type::QChar:
        value = text.at( 0 );
        break;
      case QMetaType::Type::QColor:
      {
        const QRegularExpressionMatch match = mColorExp.match( text );
        value = QColor( std::min( match.captured( 1 ).toInt(), 255 ), std::min( match.captured( 2 ).toInt(), 255 ), std::min( match.captured( 3 ).toInt(), 255 ), std::min( match.captured( 4 ).toInt(), 255 ) );
        break;
      }
      case QMetaType::Type::QDate:
      {
        const QDate date = QDate::fromString( text, Qt::ISODate );
        if ( !date.isValid() )
          return;
        value = date;
      }
      break;
      case QMetaType::Type::QDateTime:
      {
        const QDateTime dateTime = QDateTime::fromString( text, Qt::ISODate );
        if ( !dateTime.isValid() )
          return;
        value = dateTime;
      }
      break;
      case QMetaType::Type::QPoint:
      {
        const QRegularExpressionMatch match = mPointExp.match( text );
        value = QPoint( match.captured( 1 ).toInt(), match.captured( 2 ).toInt() );
        break;
      }
      case QMetaType::Type::QRect:
      {
        const QRegularExpressionMatch match = mRectExp.match( text );
        value = QRect( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(), match.captured( 3 ).toInt(), match.captured( 4 ).toInt() );
        break;
      }
      case QMetaType::Type::QSize:
      {
        const QRegularExpressionMatch match = mSizeExp.match( text );
        value = QSize( match.captured( 1 ).toInt(), match.captured( 2 ).toInt() );
        break;
      }
      case QMetaType::Type::QStringList:
        value = text.split( ',' );
        break;
      case QMetaType::Type::QTime:
      {
        const QTime time = QTime::fromString( text, Qt::ISODate );
        if ( !time.isValid() )
          return;
        value = time;
      }
      break;
      default:
        value = text;
        value.convert( QgsVariantDelegate::type( originalValue ) );
    }
  }

  model->setData( index, displayText( value ), Qt::DisplayRole );
  model->setData( index, value, Qt::UserRole );
}

bool QgsVariantDelegate::isSupportedType( QMetaType::Type type )
{
  switch ( type )
  {
    case QMetaType::Type::Bool:
    case QMetaType::Type::QByteArray:
    case QMetaType::Type::QChar:
    case QMetaType::Type::QColor:
    case QMetaType::Type::QDate:
    case QMetaType::Type::QDateTime:
    case QMetaType::Type::Double:
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::QPoint:
    case QMetaType::Type::QRect:
    case QMetaType::Type::QSize:
    case QMetaType::Type::QString:
    case QMetaType::Type::QStringList:
    case QMetaType::Type::QTime:
    case QMetaType::Type::UInt:
    case QMetaType::Type::ULongLong:
      return true;
    default:
      return false;
  }
}

QString QgsVariantDelegate::displayText( const QVariant &value )
{
  switch ( QgsVariantDelegate::type( value ) )
  {
    case QMetaType::Type::Bool:
    case QMetaType::Type::QByteArray:
    case QMetaType::Type::QChar:
    case QMetaType::Type::Double:
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::QString:
    case QMetaType::Type::UInt:
    case QMetaType::Type::ULongLong:
      return value.toString();
    case QMetaType::Type::QColor:
    {
      const QColor color = qvariant_cast<QColor>( value );
      return QStringLiteral( "(%1,%2,%3,%4)" )
        .arg( color.red() )
        .arg( color.green() )
        .arg( color.blue() )
        .arg( color.alpha() );
    }
    case QMetaType::Type::QDate:
      return value.toDate().toString( Qt::ISODate );
    case QMetaType::Type::QDateTime:
      return value.toDateTime().toString( Qt::ISODate );
    case QMetaType::Type::UnknownType:
      return QStringLiteral( "<Invalid>" );
    case QMetaType::Type::QPoint:
    {
      const QPoint point = value.toPoint();
      return QStringLiteral( "(%1,%2)" ).arg( point.x() ).arg( point.y() );
    }
    case QMetaType::Type::QRect:
    {
      const QRect rect = value.toRect();
      return QStringLiteral( "(%1,%2,%3,%4)" )
        .arg( rect.x() )
        .arg( rect.y() )
        .arg( rect.width() )
        .arg( rect.height() );
    }
    case QMetaType::Type::QSize:
    {
      const QSize size = value.toSize();
      return QStringLiteral( "(%1,%2)" ).arg( size.width() ).arg( size.height() );
    }
    case QMetaType::Type::QStringList:
      return value.toStringList().join( QLatin1Char( ',' ) );
    case QMetaType::Type::QTime:
      return value.toTime().toString( Qt::ISODate );
    default:
      break;
  }
  return QStringLiteral( "<%1>" ).arg( value.toString() );
}

/* hack to get "real" type of a variant, because QVariant::type() almost always returns QString */
QMetaType::Type QgsVariantDelegate::type( const QVariant &value )
{
  if ( value.userType() == QMetaType::Type::QString )
  {
    const QString str = value.toString();
    const thread_local QRegularExpression sBoolRegExp( QStringLiteral( "true|false" ), QRegularExpression::CaseInsensitiveOption );
    bool ok = false;

    // is this a bool (true,false)
    if ( sBoolRegExp.match( str ).hasMatch() )
      return QMetaType::Type::Bool;

    // is this an int?
    // perhaps we should treat as double for more flexibility
    ( void ) str.toInt( &ok );
    if ( ok )
      return QMetaType::Type::Int;

    // is this a double?
    ( void ) str.toDouble( &ok );
    if ( ok )
      return QMetaType::Type::Double;
  }

  // fallback to QVariant::type()
  return static_cast<QMetaType::Type>( value.userType() );
}
