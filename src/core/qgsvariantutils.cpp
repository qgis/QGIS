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
