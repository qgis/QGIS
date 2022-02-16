/***************************************************************************
    qgsfielddomainwidget.cpp
    ------------------
    Date                 : February 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfielddomainwidget.h"
#include "qgsfielddomain.h"
#include "qgsvariantutils.h"
#include "qgsgui.h"
#include <QDialogButtonBox>
#include <QPushButton>

//
// QgsAbstractFieldDomainWidget
//

QgsAbstractFieldDomainWidget::QgsAbstractFieldDomainWidget( QWidget *parent )
  : QWidget( parent )
{

}

QgsAbstractFieldDomainWidget::~QgsAbstractFieldDomainWidget() = default;


//
// QgsRangeDomainWidget
//

QgsRangeDomainWidget::QgsRangeDomainWidget( QWidget *parent )
  : QgsAbstractFieldDomainWidget( parent )
{
  setupUi( this );


  mMinSpinBox->setMinimum( std::numeric_limits<double>::lowest() );
  mMinSpinBox->setMaximum( std::numeric_limits<double>::max() );
  mMinSpinBox->setValue( 0 );
  mMinSpinBox->setDecimals( 6 );

  mMaxSpinBox->setMinimum( std::numeric_limits<double>::lowest() );
  mMaxSpinBox->setMaximum( std::numeric_limits<double>::max() );
  // any value we pick here is wrong!
  mMaxSpinBox->setValue( 100 );
  mMaxSpinBox->setDecimals( 6 );

  mMinInclusiveCheckBox->setChecked( true );
  mMaxInclusiveCheckBox->setChecked( true );

  connect( mMinSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsAbstractFieldDomainWidget::changed );
  connect( mMaxSpinBox, qOverload< double >( &QDoubleSpinBox::valueChanged ), this, &QgsAbstractFieldDomainWidget::changed );
  connect( mMinInclusiveCheckBox, &QCheckBox::toggled, this, &QgsAbstractFieldDomainWidget::changed );
  connect( mMaxInclusiveCheckBox, &QCheckBox::toggled, this, &QgsAbstractFieldDomainWidget::changed );
}

void QgsRangeDomainWidget::setFieldDomain( const QgsFieldDomain *domain )
{
  const QgsRangeFieldDomain *rangeDomain = dynamic_cast< const QgsRangeFieldDomain *>( domain );
  if ( !rangeDomain )
    return;

  // currently only supported data type is double, but in future we *may* need to handle dates/etc here
  mMinSpinBox->setValue( rangeDomain->minimum().toDouble() );
  mMaxSpinBox->setValue( rangeDomain->maximum().toDouble() );

  mMinInclusiveCheckBox->setChecked( rangeDomain->minimumIsInclusive() );
  mMaxInclusiveCheckBox->setChecked( rangeDomain->maximumIsInclusive() );
}

QgsFieldDomain *QgsRangeDomainWidget::createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const
{
  return new QgsRangeFieldDomain( name, description, fieldType,
                                  mMinSpinBox->value(), mMinInclusiveCheckBox->isChecked(),
                                  mMaxSpinBox->value(), mMaxInclusiveCheckBox->isChecked() );
}

bool QgsRangeDomainWidget::isValid() const
{
  return mMinSpinBox->value() <= mMaxSpinBox->value();
}

//
// QgsGlobDomainWidget
//

QgsGlobDomainWidget::QgsGlobDomainWidget( QWidget *parent )
  : QgsAbstractFieldDomainWidget( parent )
{
  setupUi( this );

  connect( mEditGlob, &QLineEdit::textChanged, this, &QgsAbstractFieldDomainWidget::changed );
}

void QgsGlobDomainWidget::setFieldDomain( const QgsFieldDomain *domain )
{
  const QgsGlobFieldDomain *globDomain = dynamic_cast< const QgsGlobFieldDomain *>( domain );
  if ( !globDomain )
    return;

  mEditGlob->setText( globDomain->glob() );
}

QgsFieldDomain *QgsGlobDomainWidget::createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const
{
  return new QgsGlobFieldDomain( name, description, fieldType, mEditGlob->text() );
}

bool QgsGlobDomainWidget::isValid() const
{
  return !mEditGlob->text().trimmed().isEmpty();
}

//
// QgsCodedFieldDomainWidget
//

QgsCodedFieldDomainWidget::QgsCodedFieldDomainWidget( QWidget *parent )
  : QgsAbstractFieldDomainWidget( parent )
{
  setupUi( this );

  mModel = new QgsCodedValueTableModel( this );
  mValuesTable->setModel( mModel );

  connect( mButtonAddRow, &QToolButton::clicked, this, [ = ]
  {
    mModel->insertRow( mModel->rowCount() );
  } );
  connect( mButtonRemoveRow, &QToolButton::clicked, this, [ = ]
  {
    QItemSelectionModel *selectionModel = mValuesTable->selectionModel();
    const QModelIndexList selectedRows = selectionModel->selectedIndexes();
    if ( !selectedRows.empty() )
    {
      mModel->removeRow( selectedRows.first().row() );
    }
  } );

  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsAbstractFieldDomainWidget::changed );
  connect( mModel, &QAbstractItemModel::rowsInserted, this, &QgsAbstractFieldDomainWidget::changed );
  connect( mModel, &QAbstractItemModel::rowsRemoved, this, &QgsAbstractFieldDomainWidget::changed );
}

void QgsCodedFieldDomainWidget::setFieldDomain( const QgsFieldDomain *domain )
{
  const QgsCodedFieldDomain *codedDomain = dynamic_cast< const QgsCodedFieldDomain *>( domain );
  if ( !codedDomain )
    return;

  mModel->setValues( codedDomain->values() );
}

QgsFieldDomain *QgsCodedFieldDomainWidget::createFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType ) const
{
  return new QgsCodedFieldDomain( name, description, fieldType, mModel->values() );
}

bool QgsCodedFieldDomainWidget::isValid() const
{
  return true;
}


//
// QgsCodedValueTableModel
//

QgsCodedValueTableModel::QgsCodedValueTableModel( QObject *parent )
  : QAbstractTableModel( parent )
{

}

int QgsCodedValueTableModel::rowCount( const QModelIndex & ) const
{
  return mValues.count();
}

int QgsCodedValueTableModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

QVariant QgsCodedValueTableModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= mValues.count()
       || index.column() < 0 || index.column() >= 2 )
    return QVariant();

  const QgsCodedValue &value = mValues[ index.row() ];
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case 0:
          return value.code();
        case 1:
          return value.value();
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}

bool QgsCodedValueTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.row() < 0 || index.row() >= mValues.count()
       || index.column() < 0 || index.column() >= 2 )
    return false;

  const QgsCodedValue codedValue = mValues.at( index.row() );
  switch ( role )
  {
    case Qt::EditRole:

      switch ( index.column() )
      {
        case 0:
        {
          const QgsCodedValue newValue( value.toString(), codedValue.value() );
          mValues.replace( index.row(), newValue );
          emit dataChanged( index, index );
          return true;
        }

        case 1:
        {
          const QgsCodedValue newValue( codedValue.code(), value.toString() );
          mValues.replace( index.row(), newValue );
          emit dataChanged( index, index );
          return true;
        }

        default:
          break;
      }

      break;
  }
  return false;
}

Qt::ItemFlags QgsCodedValueTableModel::flags( const QModelIndex &index ) const
{
  if ( index.row() < 0
       || index.row() >= mValues.size()
       || index.column() < 0
       || index.column() >= columnCount() )
    return QAbstractTableModel::flags( index );

  return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
}

QVariant QgsCodedValueTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( orientation )
  {
    case Qt::Horizontal:
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
          switch ( section )
          {
            case 0:
              return tr( "Code" );
            case 1:
              return tr( "Value" );
            default:
              break;
          }
        }
        break;
        default:
          break;
      }

      break;
    case Qt::Vertical:
      break;
  }
  return QVariant();
}

bool QgsCodedValueTableModel::insertRows( int row, int count, const QModelIndex &parent )
{
  if ( parent.isValid() )
    return false;

  beginInsertRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row; i < row + count; ++i )
  {
    mValues.insert( i, QgsCodedValue( QString(), QString() ) );
  }
  endInsertRows();
  return true;
}

bool QgsCodedValueTableModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || row >= mValues.count() )
    return false;

  if ( parent.isValid() )
    return false;

  for ( int i = row + count - 1; i >= row; --i )
  {
    beginRemoveRows( parent, i, i );
    mValues.removeAt( i );
    endRemoveRows();
  }
  return true;
}

void QgsCodedValueTableModel::setValues( const QList<QgsCodedValue> &values )
{
  beginResetModel();
  mValues = values;
  endResetModel();
}

//
// QgsFieldDomainWidget
//

QgsFieldDomainWidget::QgsFieldDomainWidget( Qgis::FieldDomainType type, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mComboSplitPolicy->addItem( tr( "Default Value" ), static_cast< int >( Qgis::FieldDomainSplitPolicy::DefaultValue ) );
  mComboSplitPolicy->addItem( tr( "Duplicate" ), static_cast< int >( Qgis::FieldDomainSplitPolicy::Duplicate ) );
  mComboSplitPolicy->addItem( tr( "Ratio of Geometries" ), static_cast< int >( Qgis::FieldDomainSplitPolicy::GeometryRatio ) );

  mComboMergePolicy->addItem( tr( "Default Value" ), static_cast< int >( Qgis::FieldDomainMergePolicy::DefaultValue ) );
  mComboMergePolicy->addItem( tr( "Sum" ), static_cast< int >( Qgis::FieldDomainMergePolicy::Sum ) );
  mComboMergePolicy->addItem( tr( "Geometry Weighted Average" ), static_cast< int >( Qgis::FieldDomainMergePolicy::GeometryWeighted ) );

  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::Bool ), static_cast< int >( QVariant::Bool ) );
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::String ), static_cast< int >( QVariant::String ) );
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::Int ), static_cast< int >( QVariant::Int ) );
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::LongLong ), static_cast< int >( QVariant::LongLong ) );
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::Double ), static_cast< int >( QVariant::Double ) );
#if 0 // not supported by any formats yet...
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::Date ), static_cast< int >( QVariant::Date ) );
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::Time ), static_cast< int >( QVariant::Time ) );
  mFieldTypeCombo->addItem( QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), static_cast< int >( QVariant::DateTime ) );
#endif

  switch ( type )
  {
    case Qgis::FieldDomainType::Coded:
      mDomainWidget = new QgsCodedFieldDomainWidget();
      mFieldTypeCombo->setCurrentIndex( mFieldTypeCombo->findData( static_cast< int >( QVariant::String ) ) );
      break;

    case Qgis::FieldDomainType::Range:
      mDomainWidget = new QgsRangeDomainWidget();
      mFieldTypeCombo->setCurrentIndex( mFieldTypeCombo->findData( static_cast< int >( QVariant::Double ) ) );
      break;

    case Qgis::FieldDomainType::Glob:
      mDomainWidget = new QgsGlobDomainWidget();
      mFieldTypeCombo->setCurrentIndex( mFieldTypeCombo->findData( static_cast< int >( QVariant::String ) ) );
      break;
  }

  mStackedWidget->addWidget( mDomainWidget );
  mStackedWidget->setCurrentWidget( mDomainWidget );

  connect( mNameEdit, &QLineEdit::textChanged, this, [ = ]
  {
    emit validityChanged( isValid() );
  } );

  connect( mDomainWidget, &QgsAbstractFieldDomainWidget::changed, this, [ = ]
  {
    emit validityChanged( isValid() );
  } );
}

void QgsFieldDomainWidget::setFieldDomain( const QgsFieldDomain *domain )
{
  if ( !domain )
    return;

  mNameEdit->setText( domain->name() );
  mDescriptionEdit->setText( domain->description() );
  mComboMergePolicy->setCurrentIndex( mComboMergePolicy->findData( static_cast< int >( domain->mergePolicy() ) ) );
  mComboSplitPolicy->setCurrentIndex( mComboSplitPolicy->findData( static_cast< int >( domain->splitPolicy() ) ) );
  mFieldTypeCombo->setCurrentIndex( mFieldTypeCombo->findData( static_cast< int >( domain->fieldType() ) ) );

  if ( mDomainWidget )
    mDomainWidget->setFieldDomain( domain );
}

QgsFieldDomain *QgsFieldDomainWidget::createFieldDomain() const
{
  if ( !mDomainWidget )
    return nullptr;

  std::unique_ptr< QgsFieldDomain > res( mDomainWidget->createFieldDomain( mNameEdit->text(),
                                         mDescriptionEdit->text(),
                                         static_cast< QVariant::Type >( mFieldTypeCombo->currentData().toInt() ) ) );

  res->setMergePolicy( static_cast< Qgis::FieldDomainMergePolicy >( mComboMergePolicy->currentData().toInt() ) );
  res->setSplitPolicy( static_cast< Qgis::FieldDomainSplitPolicy >( mComboSplitPolicy->currentData().toInt() ) );
  return res.release();
}

bool QgsFieldDomainWidget::isValid() const
{
  if ( mNameEdit->text().trimmed().isEmpty() )
    return false;

  return mDomainWidget && mDomainWidget->isValid();
}

//
// QgsFieldDomainDialog
//

QgsFieldDomainDialog::QgsFieldDomainDialog( Qgis::FieldDomainType type, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setObjectName( QStringLiteral( "QgsFieldDomainDialog" ) );

  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsFieldDomainWidget( type );
  vLayout->addWidget( mWidget, 1 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );

  setLayout( vLayout );
  connect( mWidget, &QgsFieldDomainWidget::validityChanged, this, &QgsFieldDomainDialog::validityChanged );
  validityChanged( mWidget->isValid() );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsFieldDomainDialog::setFieldDomain( const QgsFieldDomain *domain )
{
  mWidget->setFieldDomain( domain );
}

QgsFieldDomain *QgsFieldDomainDialog::createFieldDomain() const
{
  return mWidget->createFieldDomain();
}

void QgsFieldDomainDialog::accept()
{
  if ( !mWidget->isValid() )
    return;

  QDialog::accept();
}

void QgsFieldDomainDialog::validityChanged( bool isValid )
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
}
