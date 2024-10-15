/***************************************************************************
    qgsvariantutils.h
    ------------------
    Date                 : January 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvariantutils.h"
#include "qgslogger.h"

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QBitArray>
#include <QRect>
#include <QLine>
#include <QUuid>
#include <QImage>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QBitmap>
#include <QIcon>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>

QString QgsVariantUtils::typeToDisplayString( QMetaType::Type type, QMetaType::Type subType )
{
  switch ( type )
  {
    case QMetaType::Type::UnknownType:
      break;
    case QMetaType::Type::Bool:
      return QObject::tr( "Boolean" );
    case QMetaType::Type::Int:
      return QObject::tr( "Integer (32 bit)" );
    case QMetaType::Type::UInt:
      return QObject::tr( "Integer (unsigned 32 bit)" );
    case QMetaType::Type::LongLong:
      return QObject::tr( "Integer (64 bit)" );
    case QMetaType::Type::ULongLong:
      return QObject::tr( "Integer (unsigned 64 bit)" );
    case QMetaType::Type::Double:
      return QObject::tr( "Decimal (double)" );
    case QMetaType::Type::QChar:
      return QObject::tr( "Character" );
    case QMetaType::Type::QVariantMap:
      return QObject::tr( "Map" );
    case QMetaType::Type::QVariantList:
    {
      switch ( subType )
      {
        case QMetaType::Type::Int:
          return QObject::tr( "Integer List" );
        case QMetaType::Type::LongLong:
          return QObject::tr( "Integer (64 bit) List" );
        case QMetaType::Type::Double:
          return QObject::tr( "Decimal (double) List" );
        default:
          return QObject::tr( "List" );
      }
    }
    case QMetaType::Type::QString:
      return QObject::tr( "Text (string)" );
    case QMetaType::Type::QStringList:
      return QObject::tr( "String List" );
    case QMetaType::Type::QByteArray:
      return QObject::tr( "Binary Object (BLOB)" );
    case QMetaType::Type::QBitArray:
      return QObject::tr( "Bit Array" );
    case QMetaType::Type::QDate:
      return QObject::tr( "Date" );
    case QMetaType::Type::QTime:
      return QObject::tr( "Time" );
    case QMetaType::Type::QDateTime:
      return QObject::tr( "Date & Time" );
    case QMetaType::Type::QUrl:
      return QObject::tr( "URL" );
    case QMetaType::Type::QLocale:
      return QObject::tr( "Locale" );
    case QMetaType::Type::QRect:
    case QMetaType::Type::QRectF:
      return QObject::tr( "Rectangle" );
    case QMetaType::Type::QSize:
    case QMetaType::Type::QSizeF:
      return QObject::tr( "Size" );
    case QMetaType::Type::QLine:
    case QMetaType::Type::QLineF:
      return QObject::tr( "Line" );
    case QMetaType::Type::QPoint:
    case QMetaType::Type::QPointF:
      return QObject::tr( "Point" );
    case QMetaType::Type::QRegularExpression:
      return QObject::tr( "Regular Expression" );
    case QMetaType::Type::QVariantHash:
      return QObject::tr( "Hash" );
    case QMetaType::Type::QEasingCurve:
      return QObject::tr( "Easing Curve" );
    case QMetaType::Type::QUuid:
      return QObject::tr( "UUID" );
    case QMetaType::Type::QModelIndex:
    case QMetaType::Type::QPersistentModelIndex:
      return QObject::tr( "Model Index" );
    case QMetaType::Type::QFont:
      return QObject::tr( "Font" );
    case QMetaType::Type::QPixmap:
      return QObject::tr( "Pixmap" );
    case QMetaType::Type::QBrush:
      return QObject::tr( "Brush" );
    case QMetaType::Type::QColor:
      return QObject::tr( "Color" );
    case QMetaType::Type::QPalette:
      return QObject::tr( "Palette" );
    case QMetaType::Type::QImage:
      return QObject::tr( "Image" );
    case QMetaType::Type::QPolygon:
    case QMetaType::Type::QPolygonF:
      return QObject::tr( "Polygon" );
    case QMetaType::Type::QRegion:
      return QObject::tr( "Region" );
    case QMetaType::Type::QBitmap:
      return QObject::tr( "Bitmap" );
    case QMetaType::Type::QCursor:
      return QObject::tr( "Cursor" );
    case QMetaType::Type::QKeySequence:
      return QObject::tr( "Key Sequence" );
    case QMetaType::Type::QPen:
      return QObject::tr( "Pen" );
    case QMetaType::Type::QTextLength:
      return QObject::tr( "Text Length" );
    case QMetaType::Type::QTextFormat:
      return QObject::tr( "Text Format" );
    case QMetaType::Type::QMatrix4x4:
      return QObject::tr( "Matrix" );
    case QMetaType::Type::QTransform:
      return QObject::tr( "Transform" );
    case QMetaType::Type::QVector2D:
    case QMetaType::Type::QVector3D:
    case QMetaType::Type::QVector4D:
      return QObject::tr( "Vector" );
    case QMetaType::Type::QQuaternion:
      return QObject::tr( "Quaternion" );
    case QMetaType::Type::QIcon:
      return QObject::tr( "Icon" );
    case QMetaType::Type::QSizePolicy:
      return QObject::tr( "Size Policy" );

    default:
      break;
  }
  return QString();
}

QString QgsVariantUtils::typeToDisplayString( QVariant::Type type, QVariant::Type subType )
{
  return typeToDisplayString( QgsVariantUtils::variantTypeToMetaType( type ), QgsVariantUtils::variantTypeToMetaType( subType ) );
}

bool QgsVariantUtils::isNull( const QVariant &variant, bool silenceNullWarnings )
{
#ifndef QGISDEBUG
  ( void )silenceNullWarnings;
#endif

  if ( variant.isNull() || !variant.isValid() )
    return true;

  switch ( variant.userType() )
  {
    case QMetaType::Type::UnknownType:
    case QMetaType::Type::Bool:
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Double:
    case QMetaType::Type::QVariantMap:
    case QMetaType::Type::QVariantList:
    case QMetaType::Type::QStringList:
    case QMetaType::Type::QUrl:
    case QMetaType::Type::QLocale:
    case QMetaType::Type::QRegularExpression:
    case QMetaType::Type::QVariantHash:
    case QMetaType::Type::QEasingCurve:
    case QMetaType::Type::QModelIndex:
    case QMetaType::Type::QPersistentModelIndex:

      return false;

    case QMetaType::Type::QDate:
      if ( variant.toDate().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QDateTime was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QTime:
      if ( variant.toTime().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QTime was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QDateTime:
      if ( variant.toDate().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QDate was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QChar:
      if ( variant.toChar().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QChar was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QString:
      if ( variant.toString().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QString was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QByteArray:
      if ( variant.toByteArray().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QByteArray was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QBitArray:
      if ( variant.toBitArray().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QBitArray was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QRect:
      if ( variant.toRect().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QRect was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QRectF:
      if ( variant.toRectF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QRectF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QSize:
      if ( variant.toSize().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QSize was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QSizeF:
      if ( variant.toSizeF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QSizeF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QLine:
      if ( variant.toLine().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QLine was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QLineF:
      if ( variant.toLineF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QLineF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QPoint:
      if ( variant.toPoint().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QPoint was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QPointF:
      if ( variant.toPointF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QPointF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QUuid:
      if ( variant.toUuid().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QUuid was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QPixmap:
      if ( variant.value< QPixmap >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QPixmap was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QImage:
      if ( variant.value< QImage >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QImage was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QRegion:
      if ( variant.value< QRegion >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QRegion was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QBitmap:
      if ( variant.value< QBitmap >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QBitmap was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QIcon:
      if ( variant.value< QIcon >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QIcon was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QVector2D:
      if ( variant.value< QVector2D >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QVector2D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QVector3D:
      if ( variant.value< QVector3D >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QVector3D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QVector4D:
      if ( variant.value< QVector4D >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QVector4D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QMetaType::Type::QQuaternion:
      if ( variant.value< QQuaternion >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QQuaternion was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;

    case QMetaType::Type::QColor:
    case QMetaType::Type::QFont:
    case QMetaType::Type::QBrush:
    case QMetaType::Type::QPolygon:
    case QMetaType::Type::QPalette:
    case QMetaType::Type::QCursor:
    case QMetaType::Type::QKeySequence:
    case QMetaType::Type::QPen:
    case QMetaType::Type::QTextLength:
    case QMetaType::Type::QPolygonF:
    case QMetaType::Type::QTextFormat:
    case QMetaType::Type::QTransform:
    case QMetaType::Type::QMatrix4x4:
    case QMetaType::Type::QSizePolicy:
    case QMetaType::Type::User:
    default:
      break;
  }

  return false;
}

bool QgsVariantUtils::isNumericType( QMetaType::Type metaType )
{
  switch ( metaType )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
    case QMetaType::Type::Double:
    case QMetaType::Type::Float:
    case QMetaType::Type::Short:
    case QMetaType::Type::UShort:
    case QMetaType::Type::Char:
    case QMetaType::Type::UChar:
    case QMetaType::Type::SChar:
      return true;
    default:
      return false;
  }
}

QMetaType::Type QgsVariantUtils::variantTypeToMetaType( QVariant::Type variantType )
{
  // variant types can be directly mapped to meta types
  return static_cast< QMetaType::Type >( variantType );
}

QVariant::Type QgsVariantUtils::metaTypeToVariantType( QMetaType::Type metaType )
{
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( metaType )
  {
    // exact mapping, these are identical:
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::QChar:
    case QMetaType::QVariantMap:
    case QMetaType::QVariantList:
    case QMetaType::QString:
    case QMetaType::QStringList:
    case QMetaType::QByteArray:
    case QMetaType::QBitArray:
    case QMetaType::QDate:
    case QMetaType::QTime:
    case QMetaType::QDateTime:
    case QMetaType::QUrl:
    case QMetaType::QLocale:
    case QMetaType::QRect:
    case QMetaType::QRectF:
    case QMetaType::QSize:
    case QMetaType::QSizeF:
    case QMetaType::QLine:
    case QMetaType::QLineF:
    case QMetaType::QPoint:
    case QMetaType::QPointF:
    case QMetaType::QRegularExpression:
    case QMetaType::QVariantHash:
    case QMetaType::QEasingCurve:
    case QMetaType::QUuid:
    case QMetaType::QModelIndex:
    case QMetaType::QPersistentModelIndex:
    case QMetaType::QFont:
    case QMetaType::QPixmap:
    case QMetaType::QBrush:
    case QMetaType::QColor:
    case QMetaType::QPalette:
    case QMetaType::QImage:
    case QMetaType::QPolygon:
    case QMetaType::QRegion:
    case QMetaType::QBitmap:
    case QMetaType::QCursor:
    case QMetaType::QKeySequence:
    case QMetaType::QPen:
    case QMetaType::QTextLength:
    case QMetaType::QTextFormat:
    case QMetaType::QTransform:
    case QMetaType::QMatrix4x4:
    case QMetaType::QVector2D:
    case QMetaType::QVector3D:
    case QMetaType::QVector4D:
    case QMetaType::QQuaternion:
    case QMetaType::QPolygonF:
    case QMetaType::QIcon:
    case QMetaType::QSizePolicy:
    case QMetaType::UnknownType:
    case QMetaType::User:
      return static_cast< QVariant::Type >( metaType );

    // lossy, not exact mappings. We prefer to "expand" types
    // to avoid truncation
    case QMetaType::Long:
      return QVariant::Type::LongLong;

    case QMetaType::ULong:
      return QVariant::Type::ULongLong;

    case QMetaType::Char:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Char16:
    case QMetaType::Char32:
#endif
    case QMetaType::Short:
    case QMetaType::SChar:
      return QVariant::Type::Int;

    case QMetaType::UShort:
    case QMetaType::UChar:
      return QVariant::Type::UInt;

    case QMetaType::Float:
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    case QMetaType::Float16:
#endif
      return QVariant::Type::Double;

    // no mapping possible:
    case QMetaType::Nullptr:
    case QMetaType::QCborSimpleType:
    case QMetaType::Void:
    case QMetaType::VoidStar:
    case QMetaType::QVariant:
    case QMetaType::QJsonValue:
    case QMetaType::QJsonObject:
    case QMetaType::QJsonArray:
    case QMetaType::QJsonDocument:
    case QMetaType::QCborValue:
    case QMetaType::QCborArray:
    case QMetaType::QCborMap:
    case QMetaType::QObjectStar:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::QVariantPair:
#endif
    case QMetaType::QByteArrayList:
    case QMetaType::QColorSpace:
      break;

    default:
      break;
  }
  // NOLINTEND(bugprone-branch-clone)
  return QVariant::Type::UserType;
}

QVariant QgsVariantUtils::createNullVariant( QMetaType::Type metaType )
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  return QVariant( QgsVariantUtils::metaTypeToVariantType( metaType ) );
#else
  return QVariant( QMetaType( metaType ) );
#endif

}
