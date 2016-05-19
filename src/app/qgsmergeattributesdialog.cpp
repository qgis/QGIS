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
#include "qgsfield.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeeditor.h"
#include "qgsstatisticalsummary.h"

#include <limits>
#include <QComboBox>
#include <QSettings>

QList< QgsStatisticalSummary::Statistic > QgsMergeAttributesDialog::mDisplayStats =
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
    , mSelectionRubberBand( nullptr )
{
  setupUi( this );
  createTableWidgetContents();

  QHeaderView* verticalHeader = mTableWidget->verticalHeader();
  if ( verticalHeader )
  {
    QObject::connect( mTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( selectedRowChanged() ) );
  }
  mTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  mFromSelectedPushButton->setIcon( QgsApplication::getThemeIcon( "mActionFromSelectedFeature.png" ) );
  mRemoveFeatureFromSelectionButton->setIcon( QgsApplication::getThemeIcon( "mActionRemoveSelectedFeature.png" ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/MergeAttributes/geometry" ).toByteArray() );

  connect( mSkipAllButton, SIGNAL( clicked() ), this, SLOT( setAllToSkip() ) );
  connect( mTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( tableWidgetCellChanged( int, int ) ) );
}

QgsMergeAttributesDialog::QgsMergeAttributesDialog()
    : QDialog()
    , mVectorLayer( nullptr )
    , mMapCanvas( nullptr )
    , mSelectionRubberBand( nullptr )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/MergeAttributes/geometry" ).toByteArray() );
}

QgsMergeAttributesDialog::~QgsMergeAttributesDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/MergeAttributes/geometry", saveGeometry() );

  delete mSelectionRubberBand;
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
  QSet<int> pkAttrList = mVectorLayer->pkAttributeList().toSet();

  int col = 0;
  mHiddenAttributes.clear();
  for ( int idx = 0; idx < mFields.count(); ++idx )
  {
    if ( mVectorLayer->editFormConfig()->widgetType( idx ) == "Hidden" ||
         mVectorLayer->editFormConfig()->widgetType( idx ) == "Immutable" )
    {
      mHiddenAttributes.insert( idx );
      continue;
    }

    mTableWidget->setColumnCount( col + 1 );

    QComboBox *cb = createMergeComboBox( mFields.at( idx ) .type() );
    if ( pkAttrList.contains( idx ) )
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

      QTableWidgetItem* attributeValItem = new QTableWidgetItem( attrs.at( idx ).toString() );
      attributeValItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mTableWidget->setItem( i + 1, j, attributeValItem );
      QWidget* attributeWidget = QgsAttributeEditor::createAttributeEditor( mTableWidget, nullptr, mVectorLayer, idx, attrs.at( idx ) );
      mTableWidget->setCellWidget( i + 1, j, attributeWidget );
    }
  }

  //merge
  verticalHeaderLabels << tr( "Merge" );
  mTableWidget->setVerticalHeaderLabels( verticalHeaderLabels );

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
    newComboBox->addItem( tr( "Feature %1" ).arg( f_it->id() ), QString( "f%1" ).arg( FID_TO_STRING( f_it->id() ) ) );
  }

  switch ( columnType )
  {
    case QVariant::Double:
    case QVariant::Int:
    case QVariant::LongLong:
    {
      Q_FOREACH ( QgsStatisticalSummary::Statistic stat, mDisplayStats )
      {
        newComboBox->addItem( QgsStatisticalSummary::displayName( stat ) , stat );
      }
      break;
    }
    case QVariant::String:
      newComboBox->addItem( tr( "Concatenation" ), "concat" );
      break;

      //TODO - add date/time/datetime handling
    default:
      break;
  }

  newComboBox->addItem( tr( "Skip attribute" ), "skip" );
  newComboBox->addItem( tr( "Manual value" ), "manual" );

  QObject::connect( newComboBox, SIGNAL( currentIndexChanged( const QString& ) ),
                    this, SLOT( comboValueChanged( const QString& ) ) );
  return newComboBox;
}

int QgsMergeAttributesDialog::findComboColumn( QComboBox* c ) const
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
  Q_UNUSED( text );
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
  QTableWidgetItem* idItem = mTableWidget->verticalHeaderItem( row );
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
  QComboBox* comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, col ) );
  if ( !comboBox )
  {
    return;
  }

  //evaluate behaviour (feature value or min / max / mean )
  QString mergeBehaviourString = comboBox->itemData( comboBox->currentIndex() ).toString();
  QVariant mergeResult; // result to show in the merge result field
  if ( mergeBehaviourString == "concat" )
  {
    mergeResult = concatenationAttribute( col );
  }
  else if ( mergeBehaviourString == "skip" )
  {
    mergeResult = tr( "Skipped" );
  }
  else if ( mergeBehaviourString == "manual" )
  {
    return; //nothing to do
  }
  else if ( mergeBehaviourString.startsWith( 'f' ) )
  {
    //an existing feature value
    QgsFeatureId featureId = STRING_TO_FID( mergeBehaviourString.mid( 1 ) );
    mergeResult = featureAttribute( featureId, col );
  }
  else
  {
    //numerical statistic
    QgsStatisticalSummary::Statistic stat = ( QgsStatisticalSummary::Statistic )( comboBox->itemData( comboBox->currentIndex() ).toInt() );
    mergeResult = calcStatistic( col, stat );
  }

  //insert string into table widget
  QTableWidgetItem* newTotalItem = new QTableWidgetItem();
  newTotalItem->setData( Qt::DisplayRole, mergeResult );
  newTotalItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );

  //block signals to prevent table widget switching combo box to "manual" entry
  mTableWidget->blockSignals( true );
  mTableWidget->setItem( mTableWidget->rowCount() - 1, col, newTotalItem );
  mTableWidget->blockSignals( false );
}

QVariant QgsMergeAttributesDialog::featureAttribute( QgsFeatureId featureId, int col )
{
  int fieldIdx = mTableWidget->horizontalHeaderItem( col )->data( FieldIndex ).toInt();

  int i;
  for ( i = 0; i < mFeatureList.size() && mFeatureList.at( i ).id() != featureId; i++ )
    ;

  QVariant value;
  if ( i < mFeatureList.size() &&
       QgsAttributeEditor::retrieveValue( mTableWidget->cellWidget( i + 1, col ), mVectorLayer, fieldIdx, value ) )
  {
    return value;
  }
  else
  {
    return QVariant( mVectorLayer->fields().at( fieldIdx ).type() );
  }
}


QVariant QgsMergeAttributesDialog::calcStatistic( int col, QgsStatisticalSummary::Statistic stat )
{
  QgsStatisticalSummary summary( stat );

  bool conversion = false;
  QList<double> values;
  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    double currentValue = mTableWidget->item( i + 1, col )->text().toDouble( &conversion );
    if ( conversion )
    {
      values << currentValue;
    }
  }

  if ( values.isEmpty() )
  {
    return QVariant( mVectorLayer->fields()[col].type() );
  }

  summary.calculate( values );

  return QVariant::fromValue( summary.statistic( stat ) );
}

QVariant QgsMergeAttributesDialog::concatenationAttribute( int col )
{
  QStringList concatString;
  concatString.reserve( mFeatureList.size() );
  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    concatString << mTableWidget->item( i + 1, col )->text();
  }
  return concatString.join( "," ); //todo: make separator user configurable
}

void QgsMergeAttributesDialog::on_mFromSelectedPushButton_clicked()
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
  QTableWidgetItem* selectedItem = selectionList[0];
  int selectedRow = selectedItem->row();
  QTableWidgetItem* selectedHeaderItem = mTableWidget->verticalHeaderItem( selectedRow );
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

  QSet<int> pkAttributes = mVectorLayer->pkAttributeList().toSet();
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    if ( pkAttributes.contains( i ) )
    {
      continue;
    }
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( QString( "f%1" ).arg( FID_TO_STRING( featureId ) ) ) );
    }
  }
}

void QgsMergeAttributesDialog::on_mRemoveFeatureFromSelectionButton_clicked()
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
  QTableWidgetItem* selectedItem = selectionList[0];
  int selectedRow = selectedItem->row();
  QTableWidgetItem* selectedHeaderItem = mTableWidget->verticalHeaderItem( selectedRow );
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
  QgsFeatureIds selectedIds = mVectorLayer->selectedFeaturesIds();
  selectedIds.remove( featureId );
  mVectorLayer->selectByIds( selectedIds );
  mMapCanvas->repaint();

  //remove feature option from the combo box (without altering the current merge values)
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( !currentComboBox )
      continue;

    currentComboBox->blockSignals( true );
    currentComboBox->removeItem( currentComboBox->findData( QString( "f%1" ).arg( FID_TO_STRING( featureId ) ) ) );
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
  if ( row < mTableWidget->rowCount() - 1 )
  {
    //only looking for edits in the final row
    return;
  }

  QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, column ) );
  if ( currentComboBox )
  {
    currentComboBox->blockSignals( true );
    currentComboBox->setCurrentIndex( currentComboBox->findData( "manual" ) );
    currentComboBox->blockSignals( false );
  }
}

void QgsMergeAttributesDialog::createRubberBandForFeature( QgsFeatureId featureId )
{
  //create rubber band to highlight the feature
  delete mSelectionRubberBand;
  mSelectionRubberBand = new QgsRubberBand( mMapCanvas, mVectorLayer->geometryType() );
  mSelectionRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
  QgsFeature featureToSelect;
  mVectorLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( featureToSelect );
  mSelectionRubberBand->setToGeometry( featureToSelect.constGeometry(), mVectorLayer );
}

QgsAttributes QgsMergeAttributesDialog::mergedAttributes() const
{
  if ( mFeatureList.size() < 1 )
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
      if ( mVectorLayer->dataProvider() )
        results[fieldIdx] = mVectorLayer->dataProvider()->defaultValue( fieldIdx );
      else
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

    if ( comboBox->itemData( comboBox->currentIndex() ).toString() != "skip" )
    {
      results[fieldIdx] = currentItem->data( Qt::DisplayRole );
    }
    else if ( mVectorLayer->dataProvider() )
    {
      results[fieldIdx] = mVectorLayer->dataProvider()->defaultValue( fieldIdx );
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

    if ( comboBox->itemData( comboBox->currentIndex() ).toString() == "skip" )
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
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( "skip" ) );
    }
  }
}
