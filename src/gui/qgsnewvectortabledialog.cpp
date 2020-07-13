/***************************************************************************
  qgsnewvectortabledialog.cpp - QgsNewVectorTableDialog

 ---------------------
 begin                : 12.7.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewvectortabledialog.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsgui.h"

QgsNewVectorTableDialog::QgsNewVectorTableDialog( QgsAbstractDatabaseProviderConnection *conn, QWidget *parent )
  : QDialog( parent )
  , mConnection( conn )
  , mFieldModel( new QgsNewVectorTableFieldModel( mConnection->nativeTypes(), this ) )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  setWindowTitle( tr( "New Table" ) );

  mFieldsTableView->setModel( mFieldModel );
  QgsNewVectorTableDialogFieldsDelegate *delegate { new QgsNewVectorTableDialogFieldsDelegate( mConnection->nativeTypes(), this )};
  mFieldsTableView->setItemDelegate( delegate );
  mFieldsTableView->setSelectionBehavior( QAbstractItemView::SelectionBehavior::SelectRows );
  mFieldsTableView->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

  // Cosmetics
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Name, 200 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Type, 400 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Comment, 300 );

  // Schema is not supported by all providers
  if ( mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) )
  {
    mSchemaCbo->addItems( mConnection->schemas() );
  }
  else
  {
    mSchemaCbo->hide();
    mSchemaLabel->hide();
  }

  connect( mFieldsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, mFieldsTableView, [ = ]( const QItemSelection & selected, const QItemSelection & )
  {
    if ( ! selected.isEmpty() )
    {
      mCurrentRow = selected.indexes().first().row();
    }
    updateButtons();
  } );

  // Default type for new fields
  QVariant::Type defaultFieldType { mFieldModel->typeList().isEmpty() ? QVariant::Int : mFieldModel->typeList().first().mType };
  QString defaultFieldTypeName { mFieldModel->typeList().isEmpty() ? QString() : mFieldModel->typeList().first().mTypeName };

  // Actions
  connect( mAddFieldBtn, &QPushButton::clicked, this, [ = ]
  {
    QgsFields fieldList { fields() };
    QgsField newField { QStringLiteral( "new_field_name" ), defaultFieldType, defaultFieldTypeName };
    fieldList.append( newField );
    setFields( fieldList );
    selectRow( fieldList.count() - 1 );
  } );

  connect( mDeleteFieldBtn, &QPushButton::clicked, this, [ = ]
  {
    QgsFields fieldList { fields() };
    if ( fieldList.exists( mCurrentRow ) )
    {
      fieldList.remove( mCurrentRow );
      setFields( fieldList );
      mCurrentRow = -1;
    }
  } );

  connect( mFieldUpBtn, &QPushButton::clicked, this, [ = ]
  {
    if ( fields().exists( mCurrentRow ) && fields().exists( mCurrentRow - 1 ) )
    {
      QgsFields fieldList;
      for ( int i = 0; i < fields().count(); ++i )
      {
        if ( i == mCurrentRow - 1 )
        {
          fieldList.append( fields().at( mCurrentRow ) );
          fieldList.append( fields().at( mCurrentRow - 1 ) );
        }
        else if ( i != mCurrentRow )
        {
          fieldList.append( fields().at( i ) );
        }
      }
      setFields( fieldList );
      selectRow( mCurrentRow - 1 );
    }
  } );

  connect( mFieldDownBtn, &QPushButton::clicked, this, [ = ]
  {
    if ( fields().exists( mCurrentRow ) && fields().exists( mCurrentRow + 1 ) )
    {
      QgsFields fieldList;
      for ( int i = 0; i < fields().count(); ++i )
      {
        if ( i == mCurrentRow )
        {
          fieldList.append( fields().at( mCurrentRow + 1 ) );
          fieldList.append( fields().at( mCurrentRow ) );
        }
        else if ( i != mCurrentRow + 1 )
        {
          fieldList.append( fields().at( i ) );
        }
      }
      setFields( fieldList );
      selectRow( mCurrentRow + 1 );
    }
  } );

  updateButtons();

}

QString QgsNewVectorTableDialog::tableName() const
{
  return mTableName;
}

QString QgsNewVectorTableDialog::schemaName() const
{
  return mSchemaName;
}

QgsFields QgsNewVectorTableDialog::fields() const
{
  return mFieldModel->fields();
}

QgsWkbTypes::Type QgsNewVectorTableDialog::geometryType() const
{
  return mGeometryType;
}

void QgsNewVectorTableDialog::setFields( const QgsFields &fields )
{
  mFieldModel->setFields( fields );
}

void QgsNewVectorTableDialog::updateButtons()
{
  mDeleteFieldBtn->setEnabled( mCurrentRow != -1 );
  mFieldUpBtn->setEnabled( mCurrentRow != -1 && mCurrentRow != 0 );
  mFieldDownBtn->setEnabled( mCurrentRow != -1 && mCurrentRow != fields().count() - 1 );
}

void QgsNewVectorTableDialog::selectRow( int row )
{
  QModelIndex index { mFieldsTableView->model()->index( row, 0 ) };
  mFieldsTableView->setCurrentIndex( index );
  QItemSelectionModel::SelectionFlags flags { QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current };
  mFieldsTableView->selectionModel()->select( index, flags );
  mFieldsTableView->scrollTo( index );
}

QgsNewVectorTableDialogFieldsDelegate::QgsNewVectorTableDialogFieldsDelegate( const QList<QgsVectorDataProvider::NativeType> &typeList, QObject *parent )
  : QStyledItemDelegate( parent )
  , mTypeList( typeList )
{

}


/// @cond private

QWidget *QgsNewVectorTableDialogFieldsDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == QgsNewVectorTableFieldModel::ColumnHeaders::Type )
  {
    QComboBox *cbo = new QComboBox { parent };
    cbo->setEditable( false );
    cbo->setFrame( false );
    for ( const auto &f : mTypeList )
    {
      cbo->addItem( f.mTypeDesc, f.mTypeName );
    }
    return cbo;
  }
  return QStyledItemDelegate::createEditor( parent, option, index );
}

void QgsNewVectorTableDialogFieldsDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const auto m { index.model() };
  if ( index.column() == QgsNewVectorTableFieldModel::ColumnHeaders::Type )
  {
    const QString txt = m->data( index, Qt::DisplayRole ).toString();
    QComboBox *cbo{ qobject_cast<QComboBox *>( editor ) };
    if ( cbo )
    {
      cbo->setCurrentIndex( cbo->findText( txt ) );
      return;
    }
  }
  QStyledItemDelegate::setEditorData( editor, index );
}

void QgsNewVectorTableDialogFieldsDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( index.column() == QgsNewVectorTableFieldModel::ColumnHeaders::Type )
  {
    QComboBox *cbo { qobject_cast<QComboBox *>( editor ) };
    if ( cbo )
    {
      model->setData( index, cbo->currentData() );
      return;
    }
  }
  QStyledItemDelegate::setModelData( editor, model, index );
}

QgsNewVectorTableFieldModel::QgsNewVectorTableFieldModel( const QList<QgsVectorDataProvider::NativeType> &typeList, QObject *parent )
  : QgsFieldModel( parent )
  , mTypeList( typeList )
{

}

int QgsNewVectorTableFieldModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

QVariant QgsNewVectorTableFieldModel::data( const QModelIndex &index, int role ) const
{
  if ( mFields.exists( index.row() ) )
  {
    const QgsField field { mFields.at( index.row() ) };
    switch ( static_cast<ColumnHeaders>( index.column() ) )
    {
      case ColumnHeaders::Name:
      {
        return QgsFieldModel::data( index, role );
      }

      case ColumnHeaders::Type:
      {
        switch ( role )
        {
          case Qt::ItemDataRole::DisplayRole:
          {
            if ( index.column() == ColumnHeaders::Type )
            {
              return typeDesc( field.typeName() );
            }
            break;
          }
          case Qt::ItemDataRole::UserRole:
          {
            if ( index.column() == ColumnHeaders::Type )
            {
              return field.typeName();
            }
            break;
          }
          default:
          {
            return QVariant();
          }
        }
        break;
      }
      case ColumnHeaders::Comment:
      {
        switch ( role )
        {
          case Qt::ItemDataRole::DisplayRole:
          {
            return field.comment();
          }
          default:
            return QVariant();
        }
      }
    }
  }
  return QVariant();
}

QVariant QgsNewVectorTableFieldModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Orientation::Horizontal && role == Qt::ItemDataRole::DisplayRole )
  {
    switch ( role )
    {
      case Qt::ItemDataRole::DisplayRole:
      {
        switch ( section )
        {
          case ColumnHeaders::Name:
          {
            return tr( "Name" );
          }
          case ColumnHeaders::Type:
          {
            return tr( "Type" );
          }
          case ColumnHeaders::Comment:
          {
            return tr( "Comment" );
          }
          default:
            return QVariant();
        }
        break;
      }
      default:
        return QVariant();
    }
  }
  return QVariant();
}

Qt::ItemFlags QgsNewVectorTableFieldModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;
  return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable;
}

QList<QgsVectorDataProvider::NativeType> QgsNewVectorTableFieldModel::typeList() const
{
  return mTypeList;
}

QString QgsNewVectorTableFieldModel::typeDesc( const QString &typeName ) const
{
  for ( const auto &t : qgis::as_const( mTypeList ) )
  {
    if ( t.mTypeName.compare( typeName, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      return t.mTypeDesc;
    }
  }
  return typeName;
}

QVariant::Type QgsNewVectorTableFieldModel::type( const QString &typeName ) const
{
  for ( const auto &t : qgis::as_const( mTypeList ) )
  {
    if ( t.mTypeName.compare( typeName, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      return t.mType;
    }
  }
  // This should never happen!
  QgsDebugMsg( QStringLiteral( "Cannot get field type for: %1" ).arg( typeName ) );
  return QVariant::Type::String;
}

bool QgsNewVectorTableFieldModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role == Qt::ItemDataRole::EditRole && mFields.exists( index.row() ) && index.column() < 3 )
  {
    const int fieldIdx { index.row() };
    QgsField field {mFields.at( fieldIdx )};
    switch ( static_cast<ColumnHeaders>( index.column() ) )
    {
      case ColumnHeaders::Name:
      {
        field.setName( value.toString() );
        break;
      }
      case ColumnHeaders::Type:
      {
        field.setTypeName( value.toString() );
        field.setType( type( value.toString() ) );
        break;
      }
      case ColumnHeaders::Comment:
      {
        field.setComment( value.toString() );
        break;
      }
    }
    QgsFields fields;
    for ( int i = 0; i < mFields.count(); ++i )
    {
      if ( i == fieldIdx )
      {
        fields.append( field );
      }
      else
      {
        fields.append( mFields.at( i ) );
      }
    }
    setFields( fields );
  }
  return QgsFieldModel::setData( index, value, role );
}

/// @endcond
