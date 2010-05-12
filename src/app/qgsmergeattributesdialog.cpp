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
#include "qgsfield.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsattributeeditor.h"
#include <limits>
#include <QComboBox>

QgsMergeAttributesDialog::QgsMergeAttributesDialog( const QgsFeatureList& features, QgsVectorLayer* vl, QgsMapCanvas* canvas, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f ), mFeatureList( features ), mVectorLayer( vl ), mMapCanvas( canvas ), mSelectionRubberBand( 0 )
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

  mFromSelectedPushButton->setIcon( QgisApp::getThemeIcon( "mActionFromSelectedFeature.png" ) );
  mRemoveFeatureFromSelectionButton->setIcon( QgisApp::getThemeIcon( "mActionRemoveSelectedFeature.png" ) );
}

QgsMergeAttributesDialog::QgsMergeAttributesDialog(): QDialog()
{
  setupUi( this );
}

QgsMergeAttributesDialog::~QgsMergeAttributesDialog()
{
  delete mSelectionRubberBand;
}

void QgsMergeAttributesDialog::createTableWidgetContents()
{
  //get information about attributes from vector layer
  if ( !mVectorLayer )
  {
    return;
  }
  const QgsFieldMap& fieldMap = mVectorLayer->pendingFields();

  //combo box row, attributes titles, feature values and current merge results
  mTableWidget->setRowCount( mFeatureList.size() + 2 );
  mTableWidget->setColumnCount( fieldMap.size() );

  //create combo boxes
  for ( int i = 0; i < fieldMap.size(); ++i )
  {
    mTableWidget->setCellWidget( 0, i, createMergeComboBox( fieldMap[i].type() ) );
  }

  QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();

  //insert attribute names
  int col = 0;
  for ( ; fieldIt != fieldMap.constEnd(); ++fieldIt )
  {
    QTableWidgetItem *item = new QTableWidgetItem( fieldIt.value().name() );
    item->setData( Qt::UserRole, fieldIt.key() );
    mTableWidget->setHorizontalHeaderItem( col++, item );
  }

  //insert the attribute values
  int currentRow = 1;
  QStringList verticalHeaderLabels; //the id column is in the
  verticalHeaderLabels << tr( "Id" );

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    verticalHeaderLabels << QString::number( mFeatureList[i].id() );
    QgsAttributeMap currentAttributeMap = mFeatureList[i].attributeMap();
    QgsAttributeMap::const_iterator currentMapIt = currentAttributeMap.constBegin();
    int col = 0;
    for ( ; currentMapIt != currentAttributeMap.constEnd(); ++currentMapIt )
    {
      QTableWidgetItem* attributeValItem = new QTableWidgetItem( currentMapIt.value().toString() );
      attributeValItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mTableWidget->setItem( currentRow, col, attributeValItem );
      mTableWidget->setCellWidget( currentRow, col++, QgsAttributeEditor::createAttributeEditor( mTableWidget, NULL, mVectorLayer, currentMapIt.key(), currentMapIt.value() ) );
    }
    ++currentRow;
  }

  //merge
  verticalHeaderLabels << tr( "Merge" );
  mTableWidget->setVerticalHeaderLabels( verticalHeaderLabels );

  //insert currently merged values
  for ( int i = 0; i < fieldMap.size(); ++i )
  {
    refreshMergedValue( i );
  }
}

QComboBox* QgsMergeAttributesDialog::createMergeComboBox( QVariant::Type columnType ) const
{
  QComboBox* newComboBox = new QComboBox();
  //add items for feature
  QgsFeatureList::const_iterator f_it = mFeatureList.constBegin();
  for ( ; f_it != mFeatureList.constEnd(); ++f_it )
  {
    newComboBox->addItem( tr( "feature %1" ).arg( f_it->id() ) );
  }

  if ( columnType == QVariant::Double || columnType == QVariant::Int )
  {
    newComboBox->addItem( tr( "Minimum" ) );
    newComboBox->addItem( tr( "Maximum" ) );
    newComboBox->addItem( tr( "Median" ) );
  }
  else if ( columnType == QVariant::String )
  {
    newComboBox->addItem( tr( "Concatenation" ) );
  }
  if ( columnType == QVariant::Double )
  {
    newComboBox->addItem( tr( "Mean" ) );
  }

  QObject::connect( newComboBox, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( comboValueChanged( const QString& ) ) );
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

void QgsMergeAttributesDialog::comboValueChanged( const QString & text )
{
  QComboBox* senderComboBox = qobject_cast<QComboBox *>( sender() );
  if ( !senderComboBox )
  {
    return;
  }
  int column = findComboColumn( senderComboBox );
  refreshMergedValue( column );
}

void QgsMergeAttributesDialog::selectedRowChanged()
{
  //find out selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.size() < 1 )
  {
    delete mSelectionRubberBand;
    mSelectionRubberBand = 0;
    return;
  }

  int row = selectionList[0]->row();

  if ( !mTableWidget || !mMapCanvas || !mVectorLayer || row < 1 || row >= ( mTableWidget->rowCount() ) )
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
  int featureIdToSelect = idItem->text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    //the merge result row was selected
    delete mSelectionRubberBand;
    mSelectionRubberBand = 0;
    return;
  }
  createRubberBandForFeature( featureIdToSelect );
}

void QgsMergeAttributesDialog::refreshMergedValue( int col )
{
  //get QComboBox
  QWidget* cellWidget = mTableWidget->cellWidget( 0, col );
  if ( !cellWidget )
  {
    return;
  }
  QComboBox* comboBox = qobject_cast<QComboBox *>( cellWidget );
  if ( !comboBox )
  {
    return;
  }

  //evaluate behaviour (feature value or min / max / mean )
  QString mergeBehaviourString = comboBox->currentText();
  QString evalText; //text that has to be inserted into merge result field
  if ( mergeBehaviourString == tr( "Minimum" ) )
  {
    evalText = minimumAttributeString( col );
  }
  else if ( mergeBehaviourString == tr( "Maximum" ) )
  {
    evalText = maximumAttributeString( col );
  }
  else if ( mergeBehaviourString == tr( "Mean" ) )
  {
    evalText = meanAttributeString( col );
  }
  else if ( mergeBehaviourString == tr( "Median" ) )
  {
    evalText = medianAttributeString( col );
  }
  else if ( mergeBehaviourString == tr( "Concatenation" ) )
  {
    evalText = concatenationAttributeString( col );
  }
  else //an existing feature value
  {
    int featureId = mergeBehaviourString.split( " " ).at( 1 ).toInt(); //probably not very robust for translations...
    evalText = featureAttributeString( featureId, col );
  }

  //insert string into table widget
  QTableWidgetItem* newTotalItem = new QTableWidgetItem( evalText );
  newTotalItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mTableWidget->setItem( mTableWidget->rowCount() - 1, col, newTotalItem );
}

QString QgsMergeAttributesDialog::featureAttributeString( int featureId, int col )
{
  int idx = mTableWidget->horizontalHeaderItem( col )->data( Qt::UserRole ).toInt();

  int i;
  for ( i = 0; i < mFeatureList.size() && mFeatureList[i].id() != featureId; i++ )
    ;

  QVariant value;
  if ( i < mFeatureList.size() &&
       QgsAttributeEditor::retrieveValue( mTableWidget->cellWidget( i + 1, col ), mVectorLayer, idx, value ) )
  {
    return value.toString();
  }
  else
  {
    return "";
  }
}

QString QgsMergeAttributesDialog::minimumAttributeString( int col )
{
  double minimumValue = std::numeric_limits<double>::max();
  double currentValue;
  bool conversion = false;
  int numberOfConsideredFeatures = 0;

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    currentValue = mTableWidget->item( i + 1, col )->text().toDouble( &conversion );
    if ( conversion )
    {
      if ( currentValue < minimumValue )
      {
        minimumValue = currentValue;
        ++numberOfConsideredFeatures;
      }
    }
  }

  if ( numberOfConsideredFeatures < 1 )
  {
    return QString();
  }

  return QString::number( minimumValue, 'f' );
}

QString QgsMergeAttributesDialog::maximumAttributeString( int col )
{
  double maximumValue = -std::numeric_limits<double>::max();
  double currentValue;
  bool conversion = false;
  int numberOfConsideredFeatures = 0;

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    currentValue = mTableWidget->item( i + 1, col )->text().toDouble( &conversion );
    if ( conversion )
    {
      if ( currentValue > maximumValue )
      {
        maximumValue = currentValue;
        ++numberOfConsideredFeatures;
      }
    }
  }

  if ( numberOfConsideredFeatures < 1 )
  {
    return QString();
  }

  return QString::number( maximumValue, 'f' );
}

QString QgsMergeAttributesDialog::meanAttributeString( int col )
{
  int numberOfConsideredFeatures = 0;
  double currentValue;
  double sum = 0;
  bool conversion = false;

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    currentValue = mTableWidget->item( i + 1, col )->text().toDouble( &conversion );
    if ( conversion )
    {
      sum += currentValue;
      ++numberOfConsideredFeatures;
    }
  }
  double mean = sum / numberOfConsideredFeatures;
  return QString::number( mean, 'f' );
}

QString QgsMergeAttributesDialog::medianAttributeString( int col )
{
  //bring all values into a list and sort
  QList<double> valueList;
  double currentValue;
  bool conversionSuccess;

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    currentValue = mTableWidget->item( i + 1, col )->text().toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    valueList.push_back( currentValue );
  }
  qSort( valueList );

  double medianValue;
  int size = valueList.size();
  bool even = ( size % 2 ) < 1;
  if ( even )
  {
    medianValue = ( valueList[size / 2 - 1] + valueList[size / 2] ) / 2;
  }
  else //odd
  {
    medianValue = valueList[( size + 1 ) / 2 - 1];
  }
  return QString::number( medianValue, 'f' );
}

QString QgsMergeAttributesDialog::concatenationAttributeString( int col )
{
  QStringList concatString;
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
  if ( selectionList.size() < 1 )
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
  int featureId = selectedHeaderItem->text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return;
  }

  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findText( tr( "feature %1" ).arg( featureId ) ) );
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
  if ( selectionList.size() < 1 )
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
  int featureId = selectedHeaderItem->text().toInt( &conversionSuccess );
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
  mVectorLayer->setSelectedFeatures( selectedIds );
  mMapCanvas->repaint();

  //remove feature option from the combo box (without altering the current merge values)
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->blockSignals( true );
      currentComboBox->removeItem( currentComboBox->findText( tr( "feature %1" ).arg( featureId ) ) );
      currentComboBox->blockSignals( false );
    }
  }

  //finally remove the feature from mFeatureList
  QgsFeatureList::iterator f_it = mFeatureList.begin();
  for ( ; f_it != mFeatureList.end(); ++f_it )
  {
    if ( f_it->id() == featureId )
    {
      mFeatureList.erase( f_it );
      break;
    }
  }
}

void QgsMergeAttributesDialog::createRubberBandForFeature( int featureId )
{
  //create rubber band to highlight the feature
  delete mSelectionRubberBand;
  mSelectionRubberBand = new QgsRubberBand( mMapCanvas, mVectorLayer->geometryType() == QGis::Polygon );
  mSelectionRubberBand->setColor( QColor( 255, 0, 0 ) );
  QgsFeature featureToSelect;
  mVectorLayer->featureAtId( featureId, featureToSelect, true, false );
  mSelectionRubberBand->setToGeometry( featureToSelect.geometry(), mVectorLayer );
}

QgsAttributeMap QgsMergeAttributesDialog::mergedAttributesMap() const
{
  QgsAttributeMap resultMap;
  if ( mFeatureList.size() < 1 )
  {
    return resultMap; //return empty map
  }

  resultMap = mFeatureList[0].attributeMap();
  int index = 0;
  QgsAttributeMap::iterator it = resultMap.begin();

  for ( ; it != resultMap.end(); ++it )
  {
    QTableWidgetItem* currentItem = mTableWidget->item( mFeatureList.size() + 1, index );
    if ( !currentItem )
    {
      continue;
    }
    QString mergedString = currentItem->text();
    QVariant newValue( mergedString );
    resultMap.insert( it.key(), newValue );
    ++index;
  }

  return resultMap;
}

