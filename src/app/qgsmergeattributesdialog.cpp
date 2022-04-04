/***************************************************************************
                         qgsmergeattributesdialog.cpp
                         ----------------------------
    begin                : May 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmergeattributesdialog.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsstatisticalsummary.h"
#include "qgseditorwidgetregistry.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsfieldformatter.h"
#include "qgsfieldformatterregistry.h"

#include <limits>
#include <QComboBox>

const QList< QgsStatisticalSummary::Statistic > QgsMergeAttributesDialog::DISPLAY_STATS =
  QList< QgsStatisticalSummary::Statistic > () << QgsStatisticalSummary::Count
  << QgsStatisticalSummary::Sum
  << QgsStatisticalSummary::Mean
  << QgsStatisticalSummary::Median
  << QgsStatisticalSummary::StDev
  << QgsStatisticalSummary::StDevSample
  << QgsStatisticalSummary::Min
  << QgsStatisticalSummary::Max
  << QgsStatisticalSummary::Range
  << QgsStatisticalSummary::Minority
  << QgsStatisticalSummary::Majority
  << QgsStatisticalSummary::Variety
  << QgsStatisticalSummary::FirstQuartile
  << QgsStatisticalSummary::ThirdQuartile
  << QgsStatisticalSummary::InterQuartileRange;

QgsMergeAttributesDialog::QgsMergeAttributesDialog( const QgsFeatureList &features, QgsVectorLayer *vl, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mFeatureList( features )
  , mVectorLayer( vl )
  , mMapCanvas( canvas )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mFromSelectedPushButton, &QPushButton::clicked, this, &QgsMergeAttributesDialog::mFromSelectedPushButton_clicked );
  connect( mFromLargestPushButton, &QPushButton::clicked, this, &QgsMergeAttributesDialog::mFromLargestPushButton_clicked );
  connect( mRemoveFeatureFromSelectionButton, &QPushButton::clicked, this, &QgsMergeAttributesDialog::mRemoveFeatureFromSelectionButton_clicked );
  createTableWidgetContents();

  QHeaderView *verticalHeader = mTableWidget->verticalHeader();
  if ( verticalHeader )
  {
    connect( mTableWidget, &QTableWidget::itemSelectionChanged, this, &QgsMergeAttributesDialog::selectedRowChanged );
  }
  mTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  mFromSelectedPushButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFromSelectedFeature.svg" ) ) );
  mFromLargestPushButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFromLargestFeature.svg" ) ) );
  mRemoveFeatureFromSelectionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRemoveSelectedFeature.svg" ) ) );

  switch ( mVectorLayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
      mTakeLargestAttributesLabel->setText( tr( "Take attributes from feature with the most points" ) );
      mFromLargestPushButton->setToolTip( tr( "Take all attributes from the MultiPoint feature with the most parts" ) );
      if ( !QgsWkbTypes::isMultiType( mVectorLayer->wkbType() ) )
      {
        mTakeLargestAttributesLabel->setEnabled( false );
        mFromLargestPushButton->setEnabled( false );
      }
      break;

    case QgsWkbTypes::LineGeometry:
      mTakeLargestAttributesLabel->setText( tr( "Take attributes from feature with the longest length" ) );
      mFromLargestPushButton->setToolTip( tr( "Take all attributes from the Line feature with the longest length" ) );
      break;

    case QgsWkbTypes::PolygonGeometry:
      mTakeLargestAttributesLabel->setText( tr( "Take attributes from feature with the largest area" ) );
      mFromLargestPushButton->setToolTip( tr( "Take all attributes from the Polygon feature with the largest area" ) );
      break;

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      mTakeLargestAttributesLabel->setEnabled( false );
      mFromLargestPushButton->setEnabled( false );
      break;
  }

  connect( mSkipAllButton, &QAbstractButton::clicked, this, &QgsMergeAttributesDialog::setAllToSkip );
  connect( mTableWidget, &QTableWidget::cellChanged, this, &QgsMergeAttributesDialog::tableWidgetCellChanged );

  setAttributeTableConfig( mVectorLayer->attributeTableConfig() );
}

QgsMergeAttributesDialog::QgsMergeAttributesDialog()
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mFromSelectedPushButton, &QPushButton::clicked, this, &QgsMergeAttributesDialog::mFromSelectedPushButton_clicked );
  connect( mRemoveFeatureFromSelectionButton, &QPushButton::clicked, this, &QgsMergeAttributesDialog::mRemoveFeatureFromSelectionButton_clicked );
}

QgsMergeAttributesDialog::~QgsMergeAttributesDialog()
{
  delete mSelectionRubberBand;
}

void QgsMergeAttributesDialog::setAttributeTableConfig( const QgsAttributeTableConfig &config )
{
  const QVector< QgsAttributeTableConfig::ColumnConfig > columns = config.columns();
  for ( const QgsAttributeTableConfig::ColumnConfig &columnConfig : columns )
  {
    if ( columnConfig.hidden )
      continue;

    const int col = mFieldToColumnMap.value( columnConfig.name, -1 );
    if ( col < 0 )
      continue;

    if ( columnConfig.width >= 0 )
    {
      mTableWidget->setColumnWidth( col, columnConfig.width );
    }
    else
    {
      mTableWidget->setColumnWidth( col, mTableWidget->horizontalHeader()->defaultSectionSize() );
    }
  }
}

void QgsMergeAttributesDialog::createTableWidgetContents()
{
  //get information about attributes from vector layer
  if ( !mVectorLayer )
  {
    return;
  }

  //combo box row, attributes titles, feature values and current merge results
  mTableWidget->setRowCount( mFeatureList.size() + 2 );

  //create combo boxes and insert attribute names
  mFields = mVectorLayer->fields();

  int col = 0;
  mHiddenAttributes.clear();
  for ( int idx = 0; idx < mFields.count(); ++idx )
  {
    const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mVectorLayer, mFields.at( idx ).name() );
    if ( setup.type() == QLatin1String( "Hidden" ) || setup.type() == QLatin1String( "Immutable" ) )
    {
      mHiddenAttributes.insert( idx );
      continue;
    }

    mTableWidget->setColumnCount( col + 1 );
    mFieldToColumnMap[ mFields.at( idx ).name() ] = col;

    QComboBox *cb = createMergeComboBox( mFields.at( idx ).type() );
    if ( ! mVectorLayer->dataProvider()->pkAttributeIndexes().contains( mFields.fieldOriginIndex( idx ) ) &&
         mFields.at( idx ).constraints().constraints() & QgsFieldConstraints::ConstraintUnique )
    {
      cb->setCurrentIndex( cb->findData( "skip" ) );
    }
    mTableWidget->setCellWidget( 0, col, cb );

    QTableWidgetItem *item = new QTableWidgetItem( mFields.at( idx ).name() );
    item->setData( FieldIndex, idx );
    mTableWidget->setHorizontalHeaderItem( col++, item );
  }

  //insert the attribute values
  QStringList verticalHeaderLabels; //the id column is in the
  verticalHeaderLabels << tr( "Id" );

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    verticalHeaderLabels << FID_TO_STRING( mFeatureList[i].id() );

    QgsAttributes attrs = mFeatureList.at( i ).attributes();

    for ( int j = 0; j < mTableWidget->columnCount(); j++ )
    {
      int idx = mTableWidget->horizontalHeaderItem( j )->data( FieldIndex ).toInt();

      const QgsEditorWidgetSetup setup = mFields.at( idx ).editorWidgetSetup();
      const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
      QString stringVal = formatter->representValue( mVectorLayer, idx, setup.config(), QVariant(), attrs.at( idx ) );

      QTableWidgetItem *attributeValItem = new QTableWidgetItem( stringVal );
      attributeValItem->setData( Qt::UserRole, attrs.at( idx ) );
      attributeValItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mTableWidget->setItem( i + 1, j, attributeValItem );
    }
  }

  //merge
  verticalHeaderLabels << tr( "Merge" );
  mTableWidget->setVerticalHeaderLabels( verticalHeaderLabels );

  for ( int j = 0; j < mTableWidget->columnCount(); j++ )
  {
    QTableWidgetItem *mergedItem = new QTableWidgetItem();
    mergedItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, j, mergedItem );
  }

  //initially set any fields with default values/default value clauses to that value
  for ( int j = 0; j < mTableWidget->columnCount(); j++ )
  {
    int idx = mTableWidget->horizontalHeaderItem( j )->data( FieldIndex ).toInt();
    bool setToManual = false;
    if ( !mVectorLayer->dataProvider()->defaultValueClause( idx ).isEmpty() )
    {
      mTableWidget->item( mTableWidget->rowCount() - 1, j )->setData( Qt::DisplayRole, mVectorLayer->dataProvider()->defaultValueClause( idx ) );
      setToManual = true;
    }
    else
    {
      QVariant v = mVectorLayer->dataProvider()->defaultValue( idx );
      if ( v.isValid() )
      {
        mTableWidget->item( mTableWidget->rowCount() - 1, j )->setData( Qt::DisplayRole, v );
        mTableWidget->item( mTableWidget->rowCount() - 1, j )->setData( Qt::UserRole, v );
        setToManual = true;
      }
    }
    if ( setToManual )
    {
      QComboBox *currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, j ) );
      if ( currentComboBox )
      {
        currentComboBox->blockSignals( true );
        currentComboBox->setCurrentIndex( currentComboBox->findData( QStringLiteral( "manual" ) ) );
        currentComboBox->blockSignals( false );
      }
    }
  }

  //insert currently merged values
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    refreshMergedValue( i );
  }

}

QComboBox *QgsMergeAttributesDialog::createMergeComboBox( QVariant::Type columnType ) const
{
  QComboBox *newComboBox = new QComboBox();
  //add items for feature
  QgsFeatureList::const_iterator f_it = mFeatureList.constBegin();
  for ( ; f_it != mFeatureList.constEnd(); ++f_it )
  {
    newComboBox->addItem( tr( "Feature %1" ).arg( f_it->id() ), QStringLiteral( "f%1" ).arg( FID_TO_STRING( f_it->id() ) ) );
  }

  switch ( columnType )
  {
    case QVariant::Double:
    case QVariant::Int:
    case QVariant::LongLong:
    {
      for ( QgsStatisticalSummary::Statistic stat : std::as_const( DISPLAY_STATS ) )
      {
        newComboBox->addItem( QgsStatisticalSummary::displayName( stat ), stat );
      }
      break;
    }
    case QVariant::String:
      newComboBox->addItem( tr( "Concatenation" ), QStringLiteral( "concat" ) );
      break;

    //TODO - add date/time/datetime handling
    default:
      break;
  }

  newComboBox->addItem( tr( "Skip Attribute" ), QStringLiteral( "skip" ) );
  newComboBox->addItem( tr( "Manual Value" ), QStringLiteral( "manual" ) );

  connect( newComboBox, &QComboBox::currentTextChanged,
           this, &QgsMergeAttributesDialog::comboValueChanged );
  return newComboBox;
}

int QgsMergeAttributesDialog::findComboColumn( QComboBox *c ) const
{
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    if ( mTableWidget->cellWidget( 0, i ) == c )
    {
      return i;
    }
  }
  return -1;
}

void QgsMergeAttributesDialog::comboValueChanged( const QString &text )
{
  Q_UNUSED( text )
  QComboBox *senderComboBox = qobject_cast<QComboBox *>( sender() );
  if ( !senderComboBox )
  {
    return;
  }
  int column = findComboColumn( senderComboBox );
  if ( column < 0 )
    return;

  refreshMergedValue( column );
}

void QgsMergeAttributesDialog::selectedRowChanged()
{
  //find out selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.isEmpty() )
  {
    delete mSelectionRubberBand;
    mSelectionRubberBand = nullptr;
    return;
  }

  int row = selectionList[0]->row();

  if ( !mTableWidget || !mMapCanvas || !mVectorLayer || row < 1 || row >= mTableWidget->rowCount() )
  {
    return;
  }

  //read the feature id
  QTableWidgetItem *idItem = mTableWidget->verticalHeaderItem( row );
  if ( !idItem )
  {
    return;
  }

  bool conversionSuccess = false;
  QgsFeatureId featureIdToSelect = idItem->text().toLongLong( &conversionSuccess );
  if ( !conversionSuccess )
  {
    //the merge result row was selected
    delete mSelectionRubberBand;
    mSelectionRubberBand = nullptr;
    return;
  }
  createRubberBandForFeature( featureIdToSelect );
}

void QgsMergeAttributesDialog::refreshMergedValue( int col )
{
  QComboBox *comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, col ) );
  if ( !comboBox )
  {
    return;
  }

  const int fieldIdx = mTableWidget->horizontalHeaderItem( col )->data( FieldIndex ).toInt();

  //evaluate behavior (feature value or min / max / mean )
  QTableWidgetItem *item = mTableWidget->item( mTableWidget->rowCount() - 1, col );
  const QString mergeBehaviorString = comboBox->currentData().toString();
  QVariant mergeResult; // result to show in the merge result field
  if ( mergeBehaviorString == QLatin1String( "concat" ) )
  {
    mergeResult = concatenationAttribute( col );
  }
  else if ( mergeBehaviorString == QLatin1String( "skip" ) )
  {
    mergeResult = tr( "Skipped" );
  }
  else if ( mergeBehaviorString == QLatin1String( "manual" ) )
  {
    mergeResult = item->data( Qt::DisplayRole );
  }
  else if ( mergeBehaviorString.startsWith( 'f' ) )
  {
    //an existing feature value
    QgsFeatureId featureId = STRING_TO_FID( mergeBehaviorString.mid( 1 ) );
    mergeResult = featureAttribute( featureId, fieldIdx );
  }
  else
  {
    //numerical statistic
    QgsStatisticalSummary::Statistic stat = static_cast< QgsStatisticalSummary::Statistic >( comboBox->currentData().toInt() );
    mergeResult = calcStatistic( fieldIdx, stat );
  }

  //insert string into table widget
  mUpdating = true; // prevent combobox changing to "manual" value

  // Result formatting
  QString stringVal;
  if ( mergeBehaviorString != QLatin1String( "skip" ) &&  mergeBehaviorString != QLatin1String( "manual" ) )
  {
    const QgsEditorWidgetSetup setup = mFields.at( fieldIdx ).editorWidgetSetup();
    const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );
    stringVal = formatter->representValue( mVectorLayer, fieldIdx, setup.config(), QVariant(), mergeResult );
  }
  else
  {
    stringVal = mergeResult.toString();
  }

  item->setData( Qt::DisplayRole, stringVal );
  item->setData( Qt::UserRole, mergeResult );

  mUpdating = false;
}

QVariant QgsMergeAttributesDialog::featureAttribute( QgsFeatureId featureId, int fieldIdx )
{
  int i;
  for ( i = 0; i < mFeatureList.size() && mFeatureList.at( i ).id() != featureId; i++ )
    ;

  if ( i < mFeatureList.size() )
  {
    const QgsFeature f = mFeatureList.at( i );
    return f.attributes().at( fieldIdx );
  }

  return QVariant( mVectorLayer->fields().at( fieldIdx ).type() );
}

void QgsMergeAttributesDialog::setAllAttributesFromFeature( QgsFeatureId featureId )
{
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox *currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( !currentComboBox )
      continue;

    if ( ! mVectorLayer->dataProvider()->pkAttributeIndexes().contains( mVectorLayer->fields().fieldOriginIndex( i ) ) &&
         mVectorLayer->fields().at( i ).constraints().constraints() & QgsFieldConstraints::ConstraintUnique )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( QStringLiteral( "skip" ) ) );
    }
    else
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( QStringLiteral( "f%1" ).arg( FID_TO_STRING( featureId ) ) ) );
    }
  }
}

QVariant QgsMergeAttributesDialog::calcStatistic( int col, QgsStatisticalSummary::Statistic stat )
{
  QgsStatisticalSummary summary( stat );

  bool conversion = false;
  QList<double> values;
  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    QTableWidgetItem *currentItem = mTableWidget->item( i + 1, col );
    const QVariant currentData = currentItem->data( Qt::UserRole );
    const double currentValue = currentData.toDouble( &conversion );
    if ( conversion )
    {
      values << currentValue;
    }
  }

  if ( values.isEmpty() )
  {
    return QVariant( mVectorLayer->fields().at( col ).type() );
  }

  summary.calculate( values );

  double val = summary.statistic( stat );
  return std::isnan( val ) ? QVariant( QVariant::Double ) : val;
}

QVariant QgsMergeAttributesDialog::concatenationAttribute( int col )
{
  QStringList concatString;
  concatString.reserve( mFeatureList.size() );
  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    concatString << mTableWidget->item( i + 1, col )->text();
  }
  return concatString.join( QLatin1Char( ',' ) ); //todo: make separator user configurable
}

void QgsMergeAttributesDialog::mFromSelectedPushButton_clicked()
{
  //find the selected feature
  if ( !mVectorLayer )
  {
    return;
  }

  //find out feature id of selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.isEmpty() )
  {
    return;
  }

  //assume all selected items to be in the same row
  QTableWidgetItem *selectedItem = selectionList[0];
  int selectedRow = selectedItem->row();
  QTableWidgetItem *selectedHeaderItem = mTableWidget->verticalHeaderItem( selectedRow );
  if ( !selectedHeaderItem )
  {
    return;
  }

  bool conversionSuccess;
  QgsFeatureId featureId = selectedHeaderItem->text().toLongLong( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return;
  }

  setAllAttributesFromFeature( featureId );
}

void QgsMergeAttributesDialog::mFromLargestPushButton_clicked()
{
  QgsFeatureId featureId = FID_NULL;
  double maxValue = 0;

  switch ( mVectorLayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      QgsFeatureList::const_iterator f_it = mFeatureList.constBegin();
      for ( ; f_it != mFeatureList.constEnd(); ++f_it )
      {
        const QgsAbstractGeometry *geom = f_it->geometry().constGet();
        int partCount = geom ? geom->partCount() : 0;
        if ( partCount > maxValue )
        {
          featureId = f_it->id();
          maxValue = partCount;
        }
      }
      break;
    }
    case QgsWkbTypes::LineGeometry:
    {
      QgsFeatureList::const_iterator f_it = mFeatureList.constBegin();
      for ( ; f_it != mFeatureList.constEnd(); ++f_it )
      {
        double featureLength = f_it->geometry().length();
        if ( featureLength > maxValue )
        {
          featureId = f_it->id();
          maxValue = featureLength;
        }
      }
      break;
    }
    case QgsWkbTypes::PolygonGeometry:
    {
      QgsFeatureList::const_iterator f_it = mFeatureList.constBegin();
      for ( ; f_it != mFeatureList.constEnd(); ++f_it )
      {
        double featureArea = f_it->geometry().area();
        if ( featureArea > maxValue )
        {
          featureId = f_it->id();
          maxValue = featureArea;
        }
      }
      break;
    }
    default:
      return;
  }
  // Only proceed if we do have a largest geometry
  if ( maxValue > 0 )
  {
    setAllAttributesFromFeature( featureId );
    // Also select the appropriate row so the feature gets highlighted
    for ( int row = mTableWidget->rowCount() - 1; row >= 0; --row )
    {
      if ( mTableWidget->verticalHeaderItem( row )->text() == FID_TO_STRING( featureId ) )
      {
        mTableWidget->selectRow( row );
        return;
      }
    }
  }
}

void QgsMergeAttributesDialog::mRemoveFeatureFromSelectionButton_clicked()
{
  if ( !mVectorLayer )
  {
    return;
  }

  //find out feature id of selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.isEmpty() )
  {
    return;
  }

  //assume all selected items to be in the same row
  QTableWidgetItem *selectedItem = selectionList[0];
  int selectedRow = selectedItem->row();
  QTableWidgetItem *selectedHeaderItem = mTableWidget->verticalHeaderItem( selectedRow );
  if ( !selectedHeaderItem )
  {
    return;
  }

  bool conversionSuccess;
  QgsFeatureId featureId = selectedHeaderItem->text().toLongLong( &conversionSuccess );
  if ( !conversionSuccess )
  {
    selectedRowChanged();
    return;
  }

  mTableWidget->removeRow( selectedRow );
  selectedRowChanged();

  //remove feature from the vector layer selection
  QgsFeatureIds selectedIds = mVectorLayer->selectedFeatureIds();
  selectedIds.remove( featureId );
  mVectorLayer->selectByIds( selectedIds );
  mMapCanvas->repaint();

  //remove feature option from the combo box (without altering the current merge values)
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox *currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( !currentComboBox )
      continue;

    currentComboBox->blockSignals( true );
    currentComboBox->removeItem( currentComboBox->findData( QStringLiteral( "f%1" ).arg( FID_TO_STRING( featureId ) ) ) );
    currentComboBox->blockSignals( false );
  }

  //finally remove the feature from mFeatureList
  for ( QgsFeatureList::iterator f_it = mFeatureList.begin();
        f_it != mFeatureList.end();
        ++f_it )
  {
    if ( f_it->id() == featureId )
    {
      mFeatureList.erase( f_it );
      break;
    }
  }
}

void QgsMergeAttributesDialog::tableWidgetCellChanged( int row, int column )
{
  if ( mUpdating )
    return;

  if ( row < mTableWidget->rowCount() - 1 )
  {
    //only looking for edits in the final row
    return;
  }

  QComboBox *currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, column ) );
  if ( currentComboBox )
  {
    currentComboBox->blockSignals( true );
    currentComboBox->setCurrentIndex( currentComboBox->findData( QStringLiteral( "manual" ) ) );
    currentComboBox->blockSignals( false );
    refreshMergedValue( column );
  }
}

void QgsMergeAttributesDialog::createRubberBandForFeature( QgsFeatureId featureId )
{
  //create rubber band to highlight the feature
  delete mSelectionRubberBand;
  mSelectionRubberBand = new QgsRubberBand( mMapCanvas, mVectorLayer->geometryType() );
  mSelectionRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
  QgsFeature featureToSelect;
  mVectorLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setNoAttributes() ).nextFeature( featureToSelect );
  mSelectionRubberBand->setToGeometry( featureToSelect.geometry(), mVectorLayer );
}

QgsAttributes QgsMergeAttributesDialog::mergedAttributes() const
{
  if ( mFeatureList.empty() )
  {
    return QgsAttributes();
  }

  int widgetIndex = 0;
  QgsAttributes results( mFields.count() );
  for ( int fieldIdx = 0; fieldIdx < mFields.count(); ++fieldIdx )
  {
    if ( mHiddenAttributes.contains( fieldIdx ) )
    {
      //hidden attribute, set to default value
      results[fieldIdx] = QVariant();
      continue;
    }

    QComboBox *comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, widgetIndex ) );
    if ( !comboBox )
      continue;

    QTableWidgetItem *currentItem = mTableWidget->item( mFeatureList.size() + 1, widgetIndex );
    if ( !currentItem )
      continue;

    if ( fieldIdx >= results.count() )
      results.resize( fieldIdx + 1 ); // make sure the results vector is long enough (maybe not necessary)

    if ( comboBox->currentData().toString() != QLatin1String( "skip" ) )
    {
      results[fieldIdx] = currentItem->data( Qt::UserRole );
    }
    widgetIndex++;
  }

  return results;
}

QSet<int> QgsMergeAttributesDialog::skippedAttributeIndexes() const
{
  QSet<int> skipped;
  int widgetIndex = 0;
  for ( int i = 0; i < mFields.count(); ++i )
  {
    if ( mHiddenAttributes.contains( i ) )
    {
      skipped << i;
      continue;
    }

    QComboBox *comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, widgetIndex ) );
    if ( !comboBox )
    {
      //something went wrong, better skip this attribute
      skipped << i;
      continue;
    }

    if ( comboBox->currentData().toString() == QLatin1String( "skip" ) )
    {
      skipped << i;
    }
    widgetIndex++;
  }

  return skipped;
}

void QgsMergeAttributesDialog::setAllToSkip()
{
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox *currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( QStringLiteral( "skip" ) ) );
    }
  }
}
