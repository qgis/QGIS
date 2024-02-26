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

QString QgsVariantUtils::typeToDisplayString( QVariant::Type type, QVariant::Type subType )
{
  switch ( type )
  {
    case QVariant::Invalid:
      break;
    case QVariant::Bool:
      return QObject::tr( "Boolean" );
    case QVariant::Int:
      return QObject::tr( "Integer (32 bit)" );
    case QVariant::UInt:
      return QObject::tr( "Integer (unsigned 32 bit)" );
    case QVariant::LongLong:
      return QObject::tr( "Integer (64 bit)" );
    case QVariant::ULongLong:
      return QObject::tr( "Integer (unsigned 64 bit)" );
    case QVariant::Double:
      return QObject::tr( "Decimal (double)" );
    case QVariant::Char:
      return QObject::tr( "Character" );
    case QVariant::Map:
      return QObject::tr( "Map" );
    case QVariant::List:
    {
      switch ( subType )
      {
        case QVariant::Int:
          return QObject::tr( "Integer List" );
        case QVariant::LongLong:
          return QObject::tr( "Integer (64 bit) List" );
        case QVariant::Double:
          return QObject::tr( "Decimal (double) List" );
        default:
          return QObject::tr( "List" );
      }
    }
    case QVariant::String:
      return QObject::tr( "Text (string)" );
    case QVariant::StringList:
      return QObject::tr( "String List" );
    case QVariant::ByteArray:
      return QObject::tr( "Binary Object (BLOB)" );
    case QVariant::BitArray:
      return QObject::tr( "Bit Array" );
    case QVariant::Date:
      return QObject::tr( "Date" );
    case QVariant::Time:
      return QObject::tr( "Time" );
    case QVariant::DateTime:
      return QObject::tr( "Date & Time" );
    case QVariant::Url:
      return QObject::tr( "URL" );
    case QVariant::Locale:
      return QObject::tr( "Locale" );
    case QVariant::Rect:
    case QVariant::RectF:
      return QObject::tr( "Rectangle" );
    case QVariant::Size:
    case QVariant::SizeF:
      return QObject::tr( "Size" );
    case QVariant::Line:
    case QVariant::LineF:
      return QObject::tr( "Line" );
    case QVariant::Point:
    case QVariant::PointF:
      return QObject::tr( "Point" );
    case QVariant::RegularExpression:
      return QObject::tr( "Regular Expression" );
    case QVariant::Hash:
      return QObject::tr( "Hash" );
    case QVariant::EasingCurve:
      return QObject::tr( "Easing Curve" );
    case QVariant::Uuid:
      return QObject::tr( "UUID" );
    case QVariant::ModelIndex:
    case QVariant::PersistentModelIndex:
      return QObject::tr( "Model Index" );
    case QVariant::Font:
      return QObject::tr( "Font" );
    case QVariant::Pixmap:
      return QObject::tr( "Pixmap" );
    case QVariant::Brush:
      return QObject::tr( "Brush" );
    case QVariant::Color:
      return QObject::tr( "Color" );
    case QVariant::Palette:
      return QObject::tr( "Palette" );
    case QVariant::Image:
      return QObject::tr( "Image" );
    case QVariant::Polygon:
    case QVariant::PolygonF:
      return QObject::tr( "Polygon" );
    case QVariant::Region:
      return QObject::tr( "Region" );
    case QVariant::Bitmap:
      return QObject::tr( "Bitmap" );
    case QVariant::Cursor:
      return QObject::tr( "Cursor" );
    case QVariant::KeySequence:
      return QObject::tr( "Key Sequence" );
    case QVariant::Pen:
      return QObject::tr( "Pen" );
    case QVariant::TextLength:
      return QObject::tr( "Text Length" );
    case QVariant::TextFormat:
      return QObject::tr( "Text Format" );
    case QVariant::Matrix4x4:
      return QObject::tr( "Matrix" );
    case QVariant::Transform:
      return QObject::tr( "Transform" );
    case QVariant::Vector2D:
    case QVariant::Vector3D:
    case QVariant::Vector4D:
      return QObject::tr( "Vector" );
    case QVariant::Quaternion:
      return QObject::tr( "Quaternion" );
    case QVariant::Icon:
      return QObject::tr( "Icon" );
    case QVariant::SizePolicy:
      return QObject::tr( "Size Policy" );

    default:
      break;
  }
  return QString();
}

bool QgsVariantUtils::isNull( const QVariant &variant, bool silenceNullWarnings )
{
#ifndef QGISDEBUG
  ( void )silenceNullWarnings;
#endif

  if ( variant.isNull() || !variant.isValid() )
    return true;

  switch ( variant.type() )
  {
    case QVariant::Invalid:
    case QVariant::Bool:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
    case QVariant::Map:
    case QVariant::List:
    case QVariant::StringList:
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::RegularExpression:
    case QVariant::Hash:
    case QVariant::EasingCurve:
    case QVariant::ModelIndex:
    case QVariant::PersistentModelIndex:

    case QVariant::LastType:

      return false;

    case QVariant::Date:
      if ( variant.toDate().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QDateTime was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Time:
      if ( variant.toTime().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QTime was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::DateTime:
      if ( variant.toDate().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QDate was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Char:
      if ( variant.toChar().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QChar was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::String:
      if ( variant.toString().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QString was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::ByteArray:
      if ( variant.toByteArray().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QByteArray was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::BitArray:
      if ( variant.toBitArray().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QBitArray was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Rect:
      if ( variant.toRect().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QRect was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::RectF:
      if ( variant.toRectF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QRectF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Size:
      if ( variant.toSize().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QSize was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::SizeF:
      if ( variant.toSizeF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QSizeF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Line:
      if ( variant.toLine().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QLine was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::LineF:
      if ( variant.toLineF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QLineF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Point:
      if ( variant.toPoint().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QPoint was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::PointF:
      if ( variant.toPointF().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QPointF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Uuid:
      if ( variant.toUuid().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QUuid was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Pixmap:
      if ( variant.value< QPixmap >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QPixmap was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Image:
      if ( variant.value< QImage >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QImage was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Region:
      if ( variant.value< QRegion >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QRegion was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Bitmap:
      if ( variant.value< QBitmap >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QBitmap was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Icon:
      if ( variant.value< QIcon >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QIcon was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Vector2D:
      if ( variant.value< QVector2D >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QVector2D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Vector3D:
      if ( variant.value< QVector3D >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QVector3D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Vector4D:
      if ( variant.value< QVector4D >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QVector4D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;
    case QVariant::Quaternion:
      if ( variant.value< QQuaternion >().isNull() )
      {
        if ( !silenceNullWarnings )
        {
          QgsDebugError( QStringLiteral( "NULL QQuaternion was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        }
        return true;
      }
      return false;

    case QVariant::Color:
    case QVariant::Font:
    case QVariant::Brush:
    case QVariant::Polygon:
    case QVariant::Palette:
    case QVariant::Cursor:
    case QVariant::KeySequence:
    case QVariant::Pen:
    case QVariant::TextLength:
    case QVariant::PolygonF:
    case QVariant::TextFormat:
    case QVariant::Transform:
    case QVariant::Matrix4x4:
    case QVariant::SizePolicy:
      break;

    case QVariant::UserType:
      break;

    default:
      break;
  }

  return false;
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


