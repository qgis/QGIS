/***************************************************************************
  qgsrasterattributetablemodel.cpp - QgsRasterAttributeTableModel

 ---------------------
 begin                : 29.9.2022
 copyright            : (C) 2022 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetablemodel.h"
#include <QColor>

QString QgsRasterAttributeTableModel::RAT_COLOR_HEADER_NAME = QObject::tr( "Color" );

QgsRasterAttributeTableModel::QgsRasterAttributeTableModel( QgsRasterAttributeTable *rat, QObject *parent )
  : QAbstractTableModel( parent )
  , mRat( rat )
{

}

bool QgsRasterAttributeTableModel::editable() const
{
  return mEditable;
}

void QgsRasterAttributeTableModel::setEditable( bool editable )
{
  mEditable = editable;
}

bool QgsRasterAttributeTableModel::hasColor() const
{
  return mRat && mRat->hasColor();
}

bool QgsRasterAttributeTableModel::hasRamp() const
{
  return mRat && mRat->hasRamp();
}

QStringList QgsRasterAttributeTableModel::headerNames() const
{
  QStringList headers;
  if ( mRat )
  {
    const QList<QgsRasterAttributeTable::Field> &ratFields { mRat->fields() };
    for ( const QgsRasterAttributeTable::Field &f : std::as_const( ratFields ) )
    {
      headers.push_back( f.name );
    }

    if ( hasColor() || hasRamp() )
    {
      headers.append( RAT_COLOR_HEADER_NAME );
    }
  }
  return headers;
}

QString QgsRasterAttributeTableModel::headerTooltip( const int section ) const
{
  const QStringList hNames { headerNames() };
  if ( section < 0 || section >= hNames.count( ) )
  {
    return QString( );
  }

  const QString fieldName { hNames.at( section ) };
  const bool isColor { section == hNames.count( ) - 1 };

  if ( isColor )
  {
    return tr( "Virtual color field generated from the values in RGB(A) data columns" );
  }

  bool ok;
  const QgsRasterAttributeTable::Field field { mRat->fieldByName( fieldName, &ok ) };

  if ( ! ok )
  {
    return QString();
  }

  return QStringLiteral( R"HTML(
            <dl>
                <dt>Field Role</dt><dd>%1</dd>
                <dt>Field Type</dt><dd>%2</dd>
            </dl>
            """)HTML" ).arg( QgsRasterAttributeTable::usageName( field.usage ), QVariant::typeToName( field.type ) ) ;
}

bool QgsRasterAttributeTableModel::isValid( QString *errorMessage )
{
  if ( ! mRat )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Raster Attribute Table is not set for this model." );
    }
    return false;
  }
  return mRat->isValid( errorMessage );
}

bool QgsRasterAttributeTableModel::isDirty()
{
  return mRat && mRat->isDirty( );
}

bool QgsRasterAttributeTableModel::insertField( const int position, const QString &name, const Qgis::RasterAttributeTableFieldUsage usage, const QVariant::Type type, QString *errorMessage )
{

  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( position < 0 )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Invalid position '%1' for field insertion." ).arg( position );
    }
    return false;
  }

  const int newPosition { std::clamp( position, 0, mRat->fields().count( ) ) };
  const QgsRasterAttributeTable::Field field { name, usage, type };
  beginResetModel( );
  const bool retVal { mRat->insertField( newPosition, field, errorMessage ) };
  endResetModel();
  return retVal;
}

bool QgsRasterAttributeTableModel::removeField( const int position, QString *errorMessage )
{

  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( position < 0 || position >= mRat->fields().count() )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Invalid position '%1' for field removal." ).arg( position );
    }
    return false;
  }

  beginResetModel( );
  const bool retVal { mRat->removeField( mRat->fields().at( position ).name, errorMessage ) };
  endResetModel();
  return retVal;
}

bool QgsRasterAttributeTableModel::removeColorOrRamp( QString *errorMessage )
{

  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( ! mRat->hasColor() && ! mRat->hasRamp( ) )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "RAT has not color or ramp information." );
    }
    return false;
  }

  bool ret { true };
  beginResetModel();
  const QList<QgsRasterAttributeTable::Field> ratFields { mRat->fields() };
  for ( const QgsRasterAttributeTable::Field &f : std::as_const( ratFields ) )
  {
    if ( f.isColor() || f.isRamp() )
    {
      ret &= mRat->removeField( f.name, errorMessage );
      if ( ! ret )
      {
        break;
      }
    }
  }
  endResetModel();
  return ret;
}

bool QgsRasterAttributeTableModel::insertRow( const int position, const QVariantList &rowData, QString *errorMessage )
{
  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( position < 0 || position > mRat->data().count( ) )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Position is not valid or the table is empty." );
    }
    return false;
  }

  beginResetModel();
  const bool retVal { mRat->insertRow( position, rowData, errorMessage ) };
  endResetModel();
  return retVal;
}

bool QgsRasterAttributeTableModel::removeRow( const int position, QString *errorMessage )
{
  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( position < 0 || position >= mRat->data().count( ) )
  {
    if ( errorMessage )
    {
      *errorMessage = tr( "Position is not valid or the table is empty." );
    }
    return false;
  }

  beginResetModel();
  const bool retVal { mRat->removeRow( position, errorMessage ) };
  endResetModel();
  return retVal;
}

bool QgsRasterAttributeTableModel::editChecks( QString *errorMessage )
{
  if ( ! mRat )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Raster Attribute Table is not set for this model." );
    }
    return false;
  }

  if ( ! mEditable )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Raster Attribute Table is not editable." );
    }
    return false;
  }

  return true;
}

int QgsRasterAttributeTableModel::rowCount( const QModelIndex &parent ) const
{
  return ( !parent.isValid() && mRat ) ? mRat->data().count() : 0;
}

int QgsRasterAttributeTableModel::columnCount( const QModelIndex &parent ) const
{
  return ( ! parent.isValid() && mRat ) ? ( mRat->fields().count() + ( mRat->hasColor() || mRat->hasRamp() ? 1 : 0 ) ) : 0;
}

QVariant QgsRasterAttributeTableModel::data( const QModelIndex &index, int role ) const
{
  if ( mRat && index.isValid() && index.row() < rowCount( QModelIndex() ) && index.column() < columnCount( QModelIndex() ) )
  {
    const QString fieldName { headerNames().at( index.column() ) };
    const bool isColor { index.column() == columnCount( QModelIndex() ) - 1 };
    bool ok;
    const QgsRasterAttributeTable::Field field { mRat->fieldByName( fieldName, &ok ) };
    if ( ! isColor && ! ok )
    {
      return QVariant();
    }
    if ( hasColor() && isColor )
    {
      switch ( role )
      {
        case Qt::ItemDataRole::BackgroundRole:
          return mRat->color( index.row() );
        case Qt::ItemDataRole::DisplayRole:
          return mRat->color( index.row() ).name();
        default:
          return QVariant();
      }
    }
    else if ( ! isColor && role == Qt::ItemDataRole::TextAlignmentRole && field.type != QVariant::String )
    {
      return Qt::AlignmentFlag::AlignRight + Qt::AlignmentFlag::AlignVCenter;
    }
    else if ( role == Qt::ItemDataRole::ToolTipRole && ( field.isColor() || field.isRamp() ) )
    {
      return tr( "This data is part of a color definition: click on '%1' column to edit." ).arg( RAT_COLOR_HEADER_NAME );
    }
    else if ( role == Qt::ItemDataRole::DisplayRole || role == Qt::ItemDataRole::EditRole )
    {
      return mRat->data().at( index.row() ).at( index.column() );
    }
  }
  return QVariant();
}

bool QgsRasterAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( mRat && index.isValid() && role == Qt::ItemDataRole::EditRole )
  {
    const QString fieldName { headerNames().at( index.column() ) };
    const bool isColor { fieldName == RAT_COLOR_HEADER_NAME };
    bool ok;
    const QgsRasterAttributeTable::Field field { mRat->fieldByName( fieldName, &ok ) };
    if ( ! isColor && ! ok )
    {
      return false;
    }
    if ( hasColor() && isColor )
    {
      if ( ! value.canConvert( QVariant::Type::Color ) )
      {
        return false;
      }
      else if ( ! mRat->setColor( index.row(), value.value<QColor>( ) ) )
      {
        return false;
      }
      const QModelIndex colorColIdx { QgsRasterAttributeTableModel::index( index.row(), headerNames().indexOf( RAT_COLOR_HEADER_NAME ), QModelIndex() )};
      emit dataChanged( colorColIdx, colorColIdx );
      const QModelIndex fieldColIdx { QgsRasterAttributeTableModel::index( index.row(), index.column(), QModelIndex() )};
      emit dataChanged( fieldColIdx, fieldColIdx );
      return true;
    }
    // TODO: ramp
    else if ( ok )
    {
      const bool retVal { mRat->setValue( index.row(), index.column(), value ) };
      if ( retVal )
      {
        const QModelIndex fieldColIdx { QgsRasterAttributeTableModel::index( index.row(), index.column(), QModelIndex() )};
        emit dataChanged( fieldColIdx, fieldColIdx );
      }
      return retVal;
    }
  }
  return false;
}

QVariant QgsRasterAttributeTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Orientation::Horizontal )
  {
    const QStringList hNames { headerNames( ) };
    if ( section < hNames.length() )
    {
      switch ( role )
      {
        case Qt::ItemDataRole::DisplayRole:
          {
            return hNames.at( section );
          }
        case Qt::ItemDataRole::ToolTipRole:
          {
            return headerTooltip( section );
          }
        default:
          return QAbstractTableModel::headerData( section, orientation, role );
      }
    }
  }
  return QAbstractTableModel::headerData( section, orientation, role );
}

Qt::ItemFlags QgsRasterAttributeTableModel::flags( const QModelIndex &index ) const
{
  if ( mRat )
  {
    Qt::ItemFlags flags;
    if ( index.isValid() )
    {
      flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
      if ( mEditable )
      {
        if ( index.column() <= mRat->fields().count( ) )
        {
          const QgsRasterAttributeTable::Field &field { mRat->fields().at( index.column() ) };
          if ( ! field.isColor() && ! field.isRamp() )
          {
            flags |= Qt::ItemIsEditable;
          }
        }
        else  // Must be the color column
        {
          flags |= Qt::ItemIsEditable;
        }
      }
    }
  }
  return Qt::NoItemFlags;
}
