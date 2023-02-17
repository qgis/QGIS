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

bool QgsVariantUtils::isNull( const QVariant &variant )
{
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
        QgsDebugMsg( QStringLiteral( "NULL QDateTime was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Time:
      if ( variant.toTime().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QTime was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::DateTime:
      if ( variant.toDate().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QDate was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Char:
      if ( variant.toChar().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QChar was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::String:
      if ( variant.toString().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QString was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::ByteArray:
      if ( variant.toByteArray().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QByteArray was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::BitArray:
      if ( variant.toBitArray().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QBitArray was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Rect:
      if ( variant.toRect().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QRect was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::RectF:
      if ( variant.toRectF().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QRectF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Size:
      if ( variant.toSize().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QSize was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::SizeF:
      if ( variant.toSizeF().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QSizeF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Line:
      if ( variant.toLine().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QLine was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::LineF:
      if ( variant.toLineF().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QLineF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Point:
      if ( variant.toPoint().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QPoint was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::PointF:
      if ( variant.toPointF().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QPointF was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Uuid:
      if ( variant.toUuid().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QUuid was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Pixmap:
      if ( variant.value< QPixmap >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QPixmap was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Image:
      if ( variant.value< QImage >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QImage was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Region:
      if ( variant.value< QRegion >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QRegion was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Bitmap:
      if ( variant.value< QBitmap >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QBitmap was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Icon:
      if ( variant.value< QIcon >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QIcon was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Vector2D:
      if ( variant.value< QVector2D >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QVector2D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Vector3D:
      if ( variant.value< QVector3D >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QVector3D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Vector4D:
      if ( variant.value< QVector4D >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QVector4D was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
        return true;
      }
      return false;
    case QVariant::Quaternion:
      if ( variant.value< QQuaternion >().isNull() )
      {
        QgsDebugMsg( QStringLiteral( "NULL QQuaternion was stored in a QVariant -- stop it! Always use an invalid QVariant() instead." ) );
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


