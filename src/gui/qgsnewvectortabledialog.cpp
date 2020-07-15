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
#include "qgsapplication.h"

QgsNewVectorTableDialog::QgsNewVectorTableDialog( QgsAbstractDatabaseProviderConnection *conn, QWidget *parent )
  : QDialog( parent )
  , mConnection( conn )
  , mFieldModel( new QgsNewVectorTableFieldModel( mConnection->nativeTypes(), this ) )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  setWindowTitle( tr( "New Table" ) );

  auto updateTableNames = [ = ]( const QString &schema = QString( ) )
  {
    mTableNames.clear();
    try
    {
      const auto constTables { conn->tables( schema ) };
      for ( const auto &tp : constTables )
      {
        mTableNames.push_back( tp.tableName() );
      }
    }
    catch ( QgsProviderConnectionException &ex )
    {
      QgsDebugMsg( QStringLiteral( "Error retrieving tables from connection: %1" ).arg( ex.what() ) );
    }
  };

  mTableName->setText( QStringLiteral( "new_table_name" ) );
  mFieldsTableView->setModel( mFieldModel );
  QgsNewVectorTableDialogFieldsDelegate *delegate { new QgsNewVectorTableDialogFieldsDelegate( mConnection->nativeTypes(), this )};
  mFieldsTableView->setItemDelegate( delegate );
  mFieldsTableView->setSelectionBehavior( QAbstractItemView::SelectionBehavior::SelectRows );
  mFieldsTableView->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );

  // Cosmetics
  mFieldsTableView->horizontalHeader()->setStretchLastSection( true );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Type, 300 );
  /*mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Name, 200 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Type, 400 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::ProviderType, 300 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Length, 100 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Precision, 150 );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Comment, 300 );*/

  // Schema is not supported by all providers
  if ( mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) )
  {
    mSchemaCbo->addItems( mConnection->schemas() );
    connect( mSchemaCbo, &QComboBox::currentTextChanged, this, updateTableNames );
  }
  else
  {
    updateTableNames();
    mSchemaCbo->hide();
    mSchemaLabel->hide();
  }

  // Validators
  connect( mTableName, &QLineEdit::textChanged, this, [ = ]( const QString & )
  {
    validate();
  } );

  connect( mGeomColumn, &QLineEdit::textChanged, this, [ = ]( const QString & )
  {
    validate();
  } );

  connect( mGeomTypeCbo, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int index )
  {
    mGeomColumn->setEnabled( index != 0 );
    mSpatialIndexChk->setEnabled( index != 0 );
  } );

  // Hardcode geometry types
  // TODO: this information should really come from the connection through the provider
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconTableLayer.svg" ) ), tr( "No Geometry" ), QString() );
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconPointLayer.svg" ) ), tr( "Point" ), QgsWkbTypes::Type::Point );
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconLineLayer.svg" ) ), tr( "Line" ), QgsWkbTypes::Type::LineString );
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconPolygonLayer.svg" ) ), tr( "Polygon" ), QgsWkbTypes::Type::Polygon );
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconPointLayer.svg" ) ), tr( "MultiPoint" ), QgsWkbTypes::Type::MultiPoint );
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconLineLayer.svg" ) ), tr( "MultiLine" ), QgsWkbTypes::Type::MultiLineString );
  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconPolygonLayer.svg" ) ), tr( "MultiPolygon" ), QgsWkbTypes::Type::MultiPolygon );
  mGeomTypeCbo->setCurrentIndex( 0 );

  connect( mFieldsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, mFieldsTableView, [ = ]( const QItemSelection & selected, const QItemSelection & )
  {
    if ( ! selected.isEmpty() )
    {
      mCurrentRow = selected.indexes().first().row();
    }
    updateButtons();
  } );

  // Default type for new fields. The ternary operator is for extra safety,
  // all providers should always return a non-empty list of native types.
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
  validate();
}

void QgsNewVectorTableDialog::setSchemaName( const QString &name )
{
  mSchemaCbo->setCurrentText( name );
}

void QgsNewVectorTableDialog::setTableName( const QString &name )
{
  mTableName->setText( name );
}

void QgsNewVectorTableDialog::setGeometryType( QgsWkbTypes::Type type )
{
  mGeomTypeCbo->setCurrentIndex( mGeomTypeCbo->findData( type ) );
}

QString QgsNewVectorTableDialog::tableName() const
{
  return mTableName->text();
}

QString QgsNewVectorTableDialog::schemaName() const
{
  return mSchemaCbo->currentText();
}

QString QgsNewVectorTableDialog::geometryColumnName() const
{
  return mGeomColumn->text();
}

QgsFields QgsNewVectorTableDialog::fields() const
{
  return mFieldModel->fields();
}

QgsWkbTypes::Type QgsNewVectorTableDialog::geometryType() const
{
  return static_cast<QgsWkbTypes::Type>( mGeomTypeCbo->currentData( ).toInt() );
}

QgsCoordinateReferenceSystem QgsNewVectorTableDialog::crs() const
{
  bool ok;
  QgsCoordinateReferenceSystem srs{ QgsCoordinateReferenceSystem::fromEpsgId( mSrid->text().toLong( &ok ) ) };
  if ( ok && srs.isValid() )
  {
    return srs;
  }
  return QgsCoordinateReferenceSystem();
}

void QgsNewVectorTableDialog::setFields( const QgsFields &fields )
{
  mFieldModel->setFields( fields );
  validate();
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

void QgsNewVectorTableDialog::validate()
{
  mValidationErrors.clear();
  if ( mTableNames.contains( mTableName->text() ) )
  {
    mValidationErrors.push_back( tr( "Table '%1' already exists!" ).arg( mTableName->text() ) );
  }
  // Check for field names and geom col name
  if ( mGeomTypeCbo->currentIndex() != 0 && ! mGeomColumn->text().isEmpty() && fields().names().contains( mGeomColumn->text() ) )
  {
    mValidationErrors.push_back( tr( "Geometry column name cannot be equal to an existing field name!" ) );
  }
  const bool isValid { mValidationErrors.isEmpty() };
  if ( ! isValid )
  {
    mValidationResults->setText( mValidationErrors.join( '\n' ) );
  }
  mValidationFrame->setVisible( ! isValid );
  mButtonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isValid && fields().count() > 0 );
}

void QgsNewVectorTableDialog::showEvent( QShowEvent *event )
{
  QDialog::showEvent( event );
  mTableName->setFocus();
  mTableName->selectAll();
}


/// @cond private


QgsNewVectorTableDialogFieldsDelegate::QgsNewVectorTableDialogFieldsDelegate( const QList<QgsVectorDataProvider::NativeType> &typeList, QObject *parent )
  : QStyledItemDelegate( parent )
  , mTypeList( typeList )
{

}

QWidget *QgsNewVectorTableDialogFieldsDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == QgsNewVectorTableFieldModel::ColumnHeaders::Type )
  {
    QComboBox *cbo = new QComboBox { parent };
    cbo->setEditable( false );
    cbo->setFrame( false );
    connect( cbo, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsNewVectorTableDialogFieldsDelegate::onCurrentTypeChanged );
    for ( const auto &f : qgis::as_const( mTypeList ) )
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

void QgsNewVectorTableDialogFieldsDelegate::onCurrentTypeChanged( int index )
{
  Q_UNUSED( index )
  QComboBox *cb = static_cast<QComboBox *>( sender() );
  if ( cb )
  {
    emit commitData( cb );
  }
}

QgsNewVectorTableFieldModel::QgsNewVectorTableFieldModel( const QList<QgsVectorDataProvider::NativeType> &typeList, QObject *parent )
  : QgsFieldModel( parent )
  , mTypeList( typeList )
{

}

int QgsNewVectorTableFieldModel::columnCount( const QModelIndex & ) const
{
  return 6;
}

QVariant QgsNewVectorTableFieldModel::data( const QModelIndex &index, int role ) const
{
  if ( mFields.exists( index.row() ) )
  {
    const QgsField field { mFields.at( index.row() ) };
    switch ( role )
    {
      case Qt::ItemDataRole::DisplayRole:
      {
        switch ( static_cast<ColumnHeaders>( index.column() ) )
        {
          case ColumnHeaders::Name:
          {
            return QgsFieldModel::data( index, role );
          }
          case ColumnHeaders::Type:
          {
            return typeDesc( field.typeName() );
          }
          case ColumnHeaders::ProviderType:
          {
            return field.typeName();
          }
          case ColumnHeaders::Comment:
          {
            return field.comment();
          }
          case ColumnHeaders::Precision:
          {
            return field.precision();
          }
          case ColumnHeaders::Length:
          {
            return field.length();
          }
        }
      }
      default:
      {
        if ( static_cast<ColumnHeaders>( index.column() ) == ColumnHeaders::Name )
        {
          return QgsFieldModel::data( index, role );
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
        switch ( static_cast<QgsNewVectorTableFieldModel::ColumnHeaders>( section ) )
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
          case ColumnHeaders::ProviderType:
          {
            return tr( "Provider type" );
          }
          case ColumnHeaders::Length:
          {
            return tr( "Length" );
          }
          case ColumnHeaders::Precision:
          {
            return tr( "Precision" );
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
  // TODO: define what is editable according to type
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
  if ( role == Qt::ItemDataRole::EditRole && mFields.exists( index.row() ) && index.column() < 6 )
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
      case ColumnHeaders::ProviderType:
      {
        field.setTypeName( value.toString() );
        break;
      }
      case ColumnHeaders::Length:
      {
        field.setLength( value.toInt() );
        break;
      }
      case ColumnHeaders::Precision:
      {
        field.setPrecision( value.toInt() );
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
