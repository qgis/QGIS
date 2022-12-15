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
#include <QFont>


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
      headers.append( ratColorHeaderName() );
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
  const bool isColor { hasColor() && section == hNames.count( ) - 1 };  // *NOPAD*

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
                <dt>Role</dt><dd>%1</dd>
                <dt>Type</dt><dd>%2</dd>
                <dt>Description</dt><dd>%3</dd>
            </dl>
            )HTML" ).arg( QgsRasterAttributeTable::usageName( field.usage ),
                             QVariant::typeToName( field.type ),
                             QgsRasterAttributeTable::usageInformation().value( field.usage ).description ) ;
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

  const int newPosition { std::clamp( position, 0, static_cast<int>( mRat->fields().count( ) ) ) };
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
      *errorMessage = tr( "Raster attribute table does not have color or ramp information." );
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

bool QgsRasterAttributeTableModel::insertColor( int position, QString *errorMessage )
{
  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( position < 0 )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Invalid position '%1' for color insertion." ).arg( position );
    }
    return false;
  }

  beginResetModel();
  const bool retVal { mRat->insertColor( position, errorMessage ) };
  endResetModel();
  return retVal;
}

bool QgsRasterAttributeTableModel::insertRamp( int position, QString *errorMessage )
{
  if ( ! editChecks( errorMessage ) )
  {
    return false;
  }

  if ( position < 0 )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Invalid position '%1' for color ramp insertion." ).arg( position );
    }
    return false;
  }

  beginResetModel();
  const bool retVal { mRat->insertRamp( position, errorMessage ) };
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

QString QgsRasterAttributeTableModel::ratColorHeaderName() const
{
  return tr( "Color" );
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
    const bool isColorOrRamp { ( hasColor() || hasRamp() ) && index.column() == columnCount( QModelIndex() ) - 1 }; // *NOPAD*
    bool ok;
    const QgsRasterAttributeTable::Field field { mRat->fieldByName( fieldName, &ok ) };
    if ( ! isColorOrRamp && ! ok )
    {
      return QVariant();
    }
    if ( isColorOrRamp && hasColor() )
    {
      switch ( role )
      {
        case Qt::ItemDataRole::ForegroundRole:
        {
          // Choose black or white for a decent contrast.
          const QColor tempColor { mRat->color( index.row() ) };
          const double darkness { 1 - ( 0.299 * tempColor.red() + 0.587 * tempColor.green() + 0.114 * tempColor.blue() ) / 255};
          return darkness > 0.5 ? QColor( Qt::GlobalColor::white ) : QColor( Qt::GlobalColor::black );
        }
        case Qt::ItemDataRole::EditRole:
        case Qt::ItemDataRole::BackgroundRole:
          return mRat->color( index.row() );
        case Qt::ItemDataRole::DisplayRole:
          return mRat->color( index.row() ).name();
        default:
          return QVariant();
      }
    }
    else if ( isColorOrRamp && hasRamp() )
    {
      switch ( role )
      {
        case Qt::ItemDataRole::BackgroundRole:
        {
          return QVariant();
          // This doesn't work (even if it should), so after a large amount
          // of wasted hours I had to settle for ColorRampDelegate::paint override
          /*
          const QgsGradientColorRamp ramp { mRat->ramp( index.row() )};
          QLinearGradient gradient( QPointF(0, 0), QPointF(1, 0) );
          gradient.setCoordinateMode( QGradient::CoordinateMode::ObjectBoundingMode );
          gradient.setColorAt(0, ramp.color1() );
          gradient.setColorAt(1, ramp.color2() );
          return QBrush{ gradient };
          */
        }
        case Qt::ItemDataRole::EditRole:
        {
          return QVariant::fromValue( mRat->ramp( index.row() ) );
        }
        default:
          return QVariant();
      }
    }
    else if ( role == Qt::ItemDataRole::TextAlignmentRole && field.type != QVariant::String )
    {
      return QVariant( Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter );
    }
    else if ( role == Qt::ItemDataRole::ToolTipRole && ( isColorOrRamp ) )
    {
      return tr( "This data is part of a color definition: click on '%1' column to edit." ).arg( ratColorHeaderName() );
    }
    else if ( role == Qt::ItemDataRole::DisplayRole || role == Qt::ItemDataRole::EditRole )
    {
      return mRat->data().at( index.row() ).at( index.column() );
    }
    else if ( role == Qt::ItemDataRole::FontRole && ( isColorOrRamp ) )
    {
      QFont font;
      font.setItalic( true );
      return font;
    }
  }
  return QVariant();
}

bool QgsRasterAttributeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( mRat && index.isValid() && role == Qt::ItemDataRole::EditRole )
  {
    const QString fieldName { headerNames().at( index.column() ) };
    const bool isColorOrRamp { ( hasColor() || hasRamp() ) &&index.column() == columnCount( QModelIndex( ) ) - 1 };
    bool ok;
    const QgsRasterAttributeTable::Field field { mRat->fieldByName( fieldName, &ok ) };
    if ( ! isColorOrRamp && ! ok )
    {
      return false;
    }
    if ( hasColor() && isColorOrRamp )
    {
      if ( ! value.canConvert( QVariant::Type::Color ) || ! mRat->setColor( index.row(), value.value<QColor>( ) ) )
      {
        return false;
      }
      const QModelIndex colorColIdx { QgsRasterAttributeTableModel::index( index.row(), columnCount( QModelIndex() ) - 1, QModelIndex() )};
      emit dataChanged( colorColIdx, colorColIdx );
      // Change all color columns
      const QList<QgsRasterAttributeTable::Field> &ratFields { mRat->fields() };
      for ( int fIdx = 0; fIdx < ratFields.count(); ++fIdx )
      {
        if ( ratFields[ fIdx ].isColor() )
        {
          const QModelIndex fieldColIdx { QgsRasterAttributeTableModel::index( index.row(), fIdx, QModelIndex() )};
          emit dataChanged( fieldColIdx, fieldColIdx );
        }
      }
      return true;
    }
    else if ( hasRamp() && isColorOrRamp )
    {
      const QgsGradientColorRamp ramp { qvariant_cast<QgsGradientColorRamp>( value ) };
      if ( ! mRat->setRamp( index.row(), ramp.color1(), ramp.color2() ) )
      {
        return false;
      }
      const QModelIndex colorColIdx { QgsRasterAttributeTableModel::index( index.row(), columnCount( QModelIndex() ) - 1, QModelIndex() )};
      emit dataChanged( colorColIdx, colorColIdx );
      // Change all ramp columns
      const QList<QgsRasterAttributeTable::Field> &ratFields { mRat->fields() };
      for ( int fIdx = 0; fIdx < ratFields.count(); ++fIdx )
      {
        if ( ratFields[ fIdx ].isRamp() )
        {
          const QModelIndex fieldColIdx { QgsRasterAttributeTableModel::index( index.row(), fIdx, QModelIndex() )};
          emit dataChanged( fieldColIdx, fieldColIdx );
        }
      }
      return true;
    }
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
        if ( index.column() < mRat->fields().count( ) )
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
    return flags;
  }
  return Qt::NoItemFlags;
}
