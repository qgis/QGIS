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
      return variant.toDate().isNull();
    case QVariant::Time:
      return variant.toTime().isNull();
    case QVariant::DateTime:
      return variant.toDate().isNull();
    case QVariant::Char:
      return variant.toChar().isNull();
    case QVariant::String:
      return variant.toString().isNull();
    case QVariant::ByteArray:
      return variant.toByteArray().isNull();
    case QVariant::BitArray:
      return variant.toBitArray().isNull();
    case QVariant::Rect:
      return variant.toRect().isNull();
    case QVariant::RectF:
      return variant.toRectF().isNull();
    case QVariant::Size:
      return variant.toSize().isNull();
    case QVariant::SizeF:
      return variant.toSizeF().isNull();
    case QVariant::Line:
      return variant.toLine().isNull();
    case QVariant::LineF:
      return variant.toLineF().isNull();
    case QVariant::Point:
      return variant.toPoint().isNull();
    case QVariant::PointF:
      return variant.toPointF().isNull();
    case QVariant::Uuid:
      return variant.toUuid().isNull();
    case QVariant::Pixmap:
      return variant.value< QPixmap >().isNull();
    case QVariant::Image:
      return variant.value< QImage >().isNull();
    case QVariant::Color:
      return !variant.value< QColor >().isValid();
    case QVariant::Region:
      return variant.value< QRegion >().isNull();
    case QVariant::Bitmap:
      return variant.value< QBitmap >().isNull();
    case QVariant::Icon:
      return variant.value< QIcon>().isNull();
    case QVariant::Vector2D:
      return variant.value< QVector2D>().isNull();
    case QVariant::Vector3D:
      return variant.value< QVector3D>().isNull();
    case QVariant::Vector4D:
      return variant.value< QVector4D>().isNull();
    case QVariant::Quaternion:
      return variant.value< QQuaternion>().isNull();

    case QVariant::LastCoreType:
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
    case QVariant::LastGuiType:
      break;

    case QVariant::UserType:
      break;
  }

  return false;
}


