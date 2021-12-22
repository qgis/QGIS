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
#include "qgsiconutils.h"
#include <QSpinBox>
#include <QMessageBox>
#include <QTimer>

QgsNewVectorTableDialog::QgsNewVectorTableDialog( QgsAbstractDatabaseProviderConnection *conn, QWidget *parent )
  : QDialog( parent )
  , mConnection( conn )
{

  setupUi( this );

  // This is a precondition for the dialog to work correctly
  try
  {
    mFieldModel = new QgsNewVectorTableFieldModel( mConnection->nativeTypes(), this );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QMessageBox::critical( nullptr, tr( "Cannot Create New Tables" ), tr( "Error retrieving native types from the data provider: creation of new tables is not possible.\n"
                           "Error message: %1" ).arg( ex.what() ) );
    QTimer::singleShot( 0, [ = ] { reject(); } );
    return;
  }

  Q_ASSERT( ! mFieldModel->nativeTypes().isEmpty() );

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
      validate();
    }
    catch ( QgsProviderConnectionException &ex )
    {
      // This should never happen but it's not critical, we can safely continue.
      QgsDebugMsg( QStringLiteral( "Error retrieving tables from connection: %1" ).arg( ex.what() ) );
    }
  };

  // Validate on data changed
  connect( mFieldModel, &QgsNewVectorTableFieldModel::modelReset, this, [ = ]()
  {
    validate();
  } );

  mTableName->setText( QStringLiteral( "new_table_name" ) );
  mFieldsTableView->setModel( mFieldModel );
  QgsNewVectorTableDialogFieldsDelegate *delegate { new QgsNewVectorTableDialogFieldsDelegate( mConnection->nativeTypes(), this )};
  mFieldsTableView->setItemDelegate( delegate );
  mFieldsTableView->setSelectionBehavior( QAbstractItemView::SelectionBehavior::SelectRows );
  mFieldsTableView->setSelectionMode( QAbstractItemView::SelectionMode::SingleSelection );
  mFieldsTableView->setVerticalHeader( nullptr );

  // Cosmetics
  mFieldsTableView->horizontalHeader()->setStretchLastSection( true );
  mFieldsTableView->setColumnWidth( QgsNewVectorTableFieldModel::ColumnHeaders::Type, 300 );

  // Schema is not supported by all providers
  if ( mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) )
  {
    mSchemaCbo->addItems( mConnection->schemas() );
    connect( mSchemaCbo, &QComboBox::currentTextChanged, this, [ = ]( const QString & schema )
    {
      updateTableNames( schema );
    } );
  }
  else
  {
    mSchemaCbo->hide();
    mSchemaLabel->hide();
  }

  if ( ! mConnection->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::CreateSpatialIndex ) )
  {
    mSpatialIndexChk->setChecked( false );
    mSpatialIndexChk->hide();
    mSpatialIndexLabel->hide();
  }

  // Initial load of table names
  updateTableNames( mSchemaCbo->currentText() );

  // Validators
  connect( mTableName, &QLineEdit::textChanged, this, [ = ]( const QString & )
  {
    validate();
  } );

  connect( mGeomColumn, &QLineEdit::textChanged, this, [ = ]( const QString & )
  {
    validate();
  } );

  // Enable/disable geometry options and call validate
  connect( mGeomTypeCbo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int index )
  {
    const bool hasGeom { index != 0 };
    mGeomColumn->setEnabled( hasGeom );
    mGeomColumnLabel->setEnabled( hasGeom );
    mSpatialIndexChk->setEnabled( hasGeom );
    mSpatialIndexLabel->setEnabled( hasGeom );
    mCrs->setEnabled( hasGeom );
    mCrsLabel->setEnabled( hasGeom );
    mDimensionsLabel->setEnabled( hasGeom );
    mHasMChk->setEnabled( hasGeom );
    mHasZChk->setEnabled( hasGeom );
    validate();
  } );

  mCrs->setShowAccuracyWarnings( true );

  // geometry types
  const bool hasSinglePart { conn->geometryColumnCapabilities().testFlag( QgsAbstractDatabaseProviderConnection::GeometryColumnCapability::SinglePart ) };

  const auto addGeomItem = [this]( QgsWkbTypes::Type type )
  {
    mGeomTypeCbo->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), type );
  };

  mGeomTypeCbo->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconTableLayer.svg" ) ), tr( "No Geometry" ), QgsWkbTypes::Type::NoGeometry );
  if ( hasSinglePart )
    addGeomItem( QgsWkbTypes::Type::Point );
  addGeomItem( QgsWkbTypes::Type::MultiPoint );
  if ( hasSinglePart )
    addGeomItem( QgsWkbTypes::Type::LineString );
  addGeomItem( QgsWkbTypes::Type::MultiLineString );
  if ( hasSinglePart )
    addGeomItem( QgsWkbTypes::Type::Polygon );
  addGeomItem( QgsWkbTypes::Type::MultiPolygon );

  if ( conn->geometryColumnCapabilities().testFlag( QgsAbstractDatabaseProviderConnection::GeometryColumnCapability::Curves ) )
  {
    addGeomItem( QgsWkbTypes::Type::CompoundCurve );
    addGeomItem( QgsWkbTypes::Type::CurvePolygon );
    addGeomItem( QgsWkbTypes::Type::MultiCurve );
    addGeomItem( QgsWkbTypes::Type::MultiSurface );
  }

  mGeomTypeCbo->setCurrentIndex( 0 );

  const bool hasZ { conn->geometryColumnCapabilities().testFlag( QgsAbstractDatabaseProviderConnection::GeometryColumnCapability::Z ) };
  const bool hasM { conn->geometryColumnCapabilities().testFlag( QgsAbstractDatabaseProviderConnection::GeometryColumnCapability::M ) };
  if ( ! hasM )
  {
    mHasMChk->setEnabled( false );
    mHasMChk->setChecked( false );
  }
  if ( ! hasZ )
  {
    mHasZChk->setEnabled( false );
    mHasZChk->setChecked( false );
  }
  if ( ! hasM && ! hasM )
  {
    mHasZChk->setVisible( false );
    mHasMChk->setVisible( false );
    mDimensionsLabel->setVisible( false );
  }

  connect( mFieldsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, mFieldsTableView, [ = ]( const QItemSelection & selected, const QItemSelection & )
  {
    if ( ! selected.isEmpty() )
    {
      mCurrentRow = selected.indexes().first().row();
    }
    updateButtons();
  } );

  // Get a default type for new fields
  const QVariant::Type defaultFieldType { mFieldModel->nativeTypes().first().mType };
  const QString defaultFieldTypeName { mFieldModel->nativeTypes().first().mTypeName };

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

void QgsNewVectorTableDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs->setCrs( crs );
}

QgsCoordinateReferenceSystem QgsNewVectorTableDialog::crs() const
{
  return mCrs->crs( );
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
  return mFieldModel ? mFieldModel->fields() : QgsFields();
}

QgsWkbTypes::Type QgsNewVectorTableDialog::geometryType() const
{
  QgsWkbTypes::Type type { static_cast<QgsWkbTypes::Type>( mGeomTypeCbo->currentData( ).toInt() ) };
  if ( mHasMChk->isChecked() )
  {
    type = QgsWkbTypes::addM( type );
  }
  if ( mHasZChk->isChecked() )
  {
    type = QgsWkbTypes::addZ( type );
  }
  return type;
}


void QgsNewVectorTableDialog::setFields( const QgsFields &fields )
{
  if ( mFieldModel )
  {
    mFieldModel->setFields( fields );
  }
}

bool QgsNewVectorTableDialog::createSpatialIndex()
{
  return mSpatialIndexChk->isChecked();
}

QStringList QgsNewVectorTableDialog::validationErrors() const
{
  return mValidationErrors;
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

  const bool isSpatial { mGeomTypeCbo->currentIndex() > 0 };
  if ( mTableNames.contains( mTableName->text(), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    mValidationErrors.push_back( tr( "Table <b>%1</b> already exists!" ).arg( mTableName->text() ) );
  }
  // Check for field names and geom col name
  if ( isSpatial && fields().names().contains( mGeomColumn->text(), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    mValidationErrors.push_back( tr( "Geometry column name <b>%1</b> cannot be equal to an existing field name!" ).arg( mGeomColumn->text() ) );
  }
  // No geometry and no fields? No party!
  if ( ! isSpatial && fields().count() == 0 )
  {
    mValidationErrors.push_back( tr( "The table has no geometry column and no fields!" ) );
  }
  // Check if precision is <= length
  const auto cFields { fields() };
  for ( const auto &f : cFields )
  {
    if ( f.isNumeric() && f.length() >= 0 && f.precision() >= 0 && f.precision() > f.length() )
    {
      mValidationErrors.push_back( tr( "Field <b>%1</b>: precision cannot be greater than length!" ).arg( f.name() ) );
    }
  }

  const bool isValid { mValidationErrors.isEmpty() };
  if ( ! isValid )
  {
    mValidationResults->setText( mValidationErrors.join( QLatin1String( "<br>" ) ) );
  }

  mValidationFrame->setVisible( ! isValid );
  mButtonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isValid );
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
  switch ( index.column() )
  {
    case QgsNewVectorTableFieldModel::ColumnHeaders::Type:
    {
      QComboBox *cbo = new QComboBox { parent };
      cbo->setEditable( false );
      cbo->setFrame( false );
      connect( cbo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsNewVectorTableDialogFieldsDelegate::onFieldTypeChanged );
      for ( const auto &f : std::as_const( mTypeList ) )
      {
        cbo->addItem( f.mTypeDesc, f.mTypeName );
      }
      return cbo;
    }
    case QgsNewVectorTableFieldModel::ColumnHeaders::Precision:
    {
      QSpinBox *sp { new QSpinBox { parent } };
      const QgsNewVectorTableFieldModel *model { static_cast<const QgsNewVectorTableFieldModel *>( index.model() )};
      if ( model )
      {
        const QgsVectorDataProvider::NativeType nt { model->nativeType( index.row() ) };
        sp->setRange( nt.mMinPrec, std::min<int>( nt.mMaxPrec, index.model()->data( index.model()->index( index.row(), index.column() - 1 ) ).toInt() ) );
      }
      return sp;
    }
    case QgsNewVectorTableFieldModel::ColumnHeaders::Length:
    {
      QSpinBox *sp { new QSpinBox { parent } };
      const QgsNewVectorTableFieldModel *model { static_cast<const QgsNewVectorTableFieldModel *>( index.model() )};
      if ( model )
      {
        const QgsVectorDataProvider::NativeType nt { model->nativeType( index.row() ) };
        sp->setRange( std::max<int>( nt.mMinLen, index.model()->data( index.model()->index( index.row(), index.column() + 1 ) ).toInt() ), nt.mMaxLen );
      }
      return sp;
    }
    default:
    {
      return QStyledItemDelegate::createEditor( parent, option, index );
    }
  }
}

void QgsNewVectorTableDialogFieldsDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const auto m { index.model() };
  switch ( index.column() )
  {
    case QgsNewVectorTableFieldModel::ColumnHeaders::Type:
    {
      const QString txt = m->data( index, Qt::DisplayRole ).toString();
      QComboBox *cbo{ qobject_cast<QComboBox *>( editor ) };
      if ( cbo )
      {
        cbo->setCurrentIndex( cbo->findText( txt ) );
      }
      break;
    }
    case QgsNewVectorTableFieldModel::ColumnHeaders::Precision:
    case QgsNewVectorTableFieldModel::ColumnHeaders::Length:
    {
      const int value = m->data( index, Qt::DisplayRole ).toInt();
      QSpinBox *sp{ qobject_cast<QSpinBox *>( editor ) };
      if ( sp )
      {
        sp->setValue( value );
      }
      break;
    }
    default:
    {
      QStyledItemDelegate::setEditorData( editor, index );
    }
  }
}

void QgsNewVectorTableDialogFieldsDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsNewVectorTableFieldModel::ColumnHeaders::Type:
    {
      QComboBox *cbo { qobject_cast<QComboBox *>( editor ) };
      if ( cbo )
      {
        model->setData( index, cbo->currentData() );
      }
      break;
    }
    default:
    {
      QStyledItemDelegate::setModelData( editor, model, index );
    }
  }
}

void QgsNewVectorTableDialogFieldsDelegate::onFieldTypeChanged( int index )
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
  , mNativeTypes( typeList )
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
          default:
            break;
        }
        return QgsFieldModel::data( index, role );
      }
      case Qt::ItemDataRole::TextAlignmentRole:
      {
        switch ( static_cast<ColumnHeaders>( index.column() ) )
        {
          case ColumnHeaders::Precision:
          case ColumnHeaders::Length:
          {
            return static_cast<Qt::Alignment::Int>( Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignHCenter );
          }
          default:
            break;
        }
        return QgsFieldModel::data( index, role );
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
  if ( orientation == Qt::Orientation::Horizontal )
  {
    switch ( role )
    {
      case Qt::ItemDataRole::DisplayRole:
      {
        switch ( static_cast<ColumnHeaders>( section ) )
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
      case Qt::ItemDataRole::TextAlignmentRole:
      {
        switch ( static_cast<ColumnHeaders>( section ) )
        {
          case ColumnHeaders::Name:
          case ColumnHeaders::Comment:
          case ColumnHeaders::Type:
          case ColumnHeaders::ProviderType:
          {
            return static_cast<Qt::Alignment::Int>( Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignLeft );
          }
          default:
          {
            return static_cast<Qt::Alignment::Int>( Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignHCenter );
          }
        }
        break;
      }
      default:
      {
        QgsFieldModel::headerData( section, orientation, role );
      }
    }
  }
  return QVariant();
}

Qt::ItemFlags QgsNewVectorTableFieldModel::flags( const QModelIndex &index ) const
{
  switch ( static_cast<ColumnHeaders>( index.column() ) )
  {
    case ColumnHeaders::Name:
    case ColumnHeaders::Comment:
    case ColumnHeaders::Type:
    {
      return QgsFieldModel::flags( index ) | Qt::ItemIsEditable;
    }
    case ColumnHeaders::Length:
    {
      if ( mFields.exists( index.row( ) ) )
      {
        const QgsVectorDataProvider::NativeType nt { nativeType( mFields.at( index.row( ) ).typeName() ) };
        if ( nt.mMinLen < nt.mMaxLen )
        {
          return QgsFieldModel::flags( index ) | Qt::ItemIsEditable;
        }
      }
      break;
    }
    case ColumnHeaders::Precision:
    {
      if ( mFields.exists( index.row( ) ) )
      {
        const QgsVectorDataProvider::NativeType nt { nativeType( mFields.at( index.row( ) ).typeName() ) };
        if ( nt.mMinPrec < nt.mMaxPrec )
        {
          return QgsFieldModel::flags( index ) | Qt::ItemIsEditable;
        }
      }
      break;
    }
    case ColumnHeaders::ProviderType:
    {
      return QgsFieldModel::flags( index );
    }
  }
  return QgsFieldModel::flags( index );
}

QList<QgsVectorDataProvider::NativeType> QgsNewVectorTableFieldModel::nativeTypes() const
{
  return mNativeTypes;
}

QString QgsNewVectorTableFieldModel::typeDesc( const QString &typeName ) const
{
  for ( const auto &t : std::as_const( mNativeTypes ) )
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
  return nativeType( typeName ).mType;
}

QgsVectorDataProvider::NativeType QgsNewVectorTableFieldModel::nativeType( const QString &typeName ) const
{
  for ( const auto &t : std::as_const( mNativeTypes ) )
  {
    if ( t.mTypeName.compare( typeName, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      return t;
    }
  }
  // This should never happen!
  QgsDebugMsg( QStringLiteral( "Cannot get field native type for: %1" ).arg( typeName ) );
  return mNativeTypes.first();
}

QgsVectorDataProvider::NativeType QgsNewVectorTableFieldModel::nativeType( int row ) const
{
  if ( mFields.exists( row ) )
  {
    return nativeType( mFields.at( row ).typeName() );
  }
  // This should never happen!
  QgsDebugMsg( QStringLiteral( "Cannot get field for row: %1" ).arg( row ) );
  return mNativeTypes.first();
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
        const auto tp { nativeType( value.toString() ) };
        field.setType( tp.mType );
        field.setLength( std::max( std::min<int>( field.length(), tp.mMaxLen ), tp.mMinLen ) );
        field.setPrecision( std::max( std::min<int>( field.precision(), tp.mMaxPrec ), tp.mMinPrec ) );
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
