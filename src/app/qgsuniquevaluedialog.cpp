/***************************************************************************
                         qgsuniquevaluedialog.cpp  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsuniquevaluedialog.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgssymbol.h"
#include "qgsuniquevaluerenderer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include "qgslogger.h"

#include <QMessageBox>
#include <QKeyEvent>

QgsUniqueValueDialog::QgsUniqueValueDialog( QgsVectorLayer* vl ): QDialog(), mVectorLayer( vl ), sydialog( vl, true )
{
  setupUi( this );
  setOrientation( Qt::Vertical );

  //find out the fields of mVectorLayer
  if ( mVectorLayer )
  {
    //we cannot use unique values for not-commited fields because QgsVectorLayer has no 'unique values' method...
    QgsVectorDataProvider* provider = mVectorLayer->dataProvider();
    if ( provider )
    {
      const QgsFieldMap & fields = provider->fields();
      QString str;

      for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
      {
        str = ( *it ).name();
        str = mVectorLayer->attributeDisplayName( it.key() );
        mClassificationComboBox->addItem( str, it.key() );
      }
    }
  }


  mClassListWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mClassListWidget->setEditTriggers( QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed );
  mClassListWidget->setSortingEnabled( true );

  if ( mVectorLayer )
  {
    const QgsUniqueValueRenderer* renderer = dynamic_cast<const QgsUniqueValueRenderer *>( mVectorLayer->renderer() );

    if ( renderer )
    {
      mClassListWidget->clear();
      QString field = mVectorLayer->attributeDisplayName( renderer->classificationField() );
      mOldClassificationAttribute = field;
      mClassificationComboBox->setCurrentIndex( mClassificationComboBox->findText( field ) );

      const QList<QgsSymbol*> list = renderer->symbols();
      //fill the items of the renderer into mValues
      for ( QList<QgsSymbol*>::const_iterator iter = list.begin(); iter != list.end(); ++iter )
      {
        QgsSymbol* symbol = *iter;
        QString symbolvalue = symbol->lowerValue();
        QgsSymbol* sym = new QgsSymbol( mVectorLayer->geometryType(), symbol->lowerValue(), symbol->upperValue(), symbol->label() );
        sym->setPen( symbol->pen() );
        sym->setCustomTexture( symbol->customTexture() );
        sym->setBrush( symbol->brush() );
        sym->setNamedPointSymbol( symbol->pointSymbolName() );
        sym->setPointSize( symbol->pointSize() );
        sym->setPointSizeUnits( symbol->pointSizeUnits() );
        sym->setScaleClassificationField( symbol->scaleClassificationField() );
        sym->setRotationClassificationField( symbol->rotationClassificationField() );
        mValues.insert( symbolvalue, sym );

        QListWidgetItem *item = new QListWidgetItem( symbolvalue );
        mClassListWidget->addItem( item );
        updateEntryIcon( symbol, item );
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        item->setData( Qt::UserRole, symbol->lowerValue() );
        item->setToolTip( symbol->label() );
      }
    }
  }

  mDeletePushButton->setEnabled( false );

  connect( mClassifyButton, SIGNAL( clicked() ), this, SLOT( changeClassificationAttribute() ) );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addClass() ) );
  connect( mDeletePushButton, SIGNAL( clicked() ), this, SLOT( deleteSelectedClasses() ) );
  connect( mRandomizeColors, SIGNAL( clicked() ), this, SLOT( randomizeColors() ) );
  connect( mResetColors, SIGNAL( clicked() ), this, SLOT( resetColors() ) );
  connect( mClassListWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( selectionChanged() ) );
  connect( mCommonPropertyLock, SIGNAL( clicked() ), this, SLOT( selectionChanged() ) );
  connect( mClassListWidget, SIGNAL( itemChanged( QListWidgetItem * ) ), this, SLOT( itemChanged( QListWidgetItem * ) ) );
  connect( &sydialog, SIGNAL( settingsChanged() ), this, SLOT( applySymbologyChanges() ) );
  mSymbolWidgetStack->addWidget( &sydialog );
  mSymbolWidgetStack->setCurrentWidget( &sydialog );
}

QgsUniqueValueDialog::~QgsUniqueValueDialog()
{
  QgsDebugMsg( "called." );
  QMap<QString, QgsSymbol *>::iterator myValueIterator = mValues.begin();
  while ( myValueIterator != mValues.end() )
  {
    delete myValueIterator.value();

    mValues.erase( myValueIterator );

    myValueIterator = mValues.begin(); // since iterator invalidated due to
    // erase(), reset to new first element
  }
  mClassListWidget->setCurrentItem( 0 );
}

void QgsUniqueValueDialog::apply()
{
  QgsDebugMsg( "called." );
  QgsUniqueValueRenderer *renderer = new QgsUniqueValueRenderer( mVectorLayer->geometryType() );

  //go through mValues and add the entries to the renderer
  for ( QMap<QString, QgsSymbol*>::iterator it = mValues.begin(); it != mValues.end(); ++it )
  {
    QgsSymbol* symbol = it.value();
    QgsSymbol* newsymbol = new QgsSymbol( mVectorLayer->geometryType(), symbol->lowerValue(), symbol->upperValue(), symbol->label() );
    newsymbol->setPen( symbol->pen() );
    newsymbol->setCustomTexture( symbol->customTexture() );
    newsymbol->setBrush( symbol->brush() );
    newsymbol->setNamedPointSymbol( symbol->pointSymbolName() );
    newsymbol->setPointSize( symbol->pointSize() );
    newsymbol->setPointSizeUnits( symbol->pointSizeUnits() );
    newsymbol->setScaleClassificationField( symbol->scaleClassificationField() );
    newsymbol->setRotationClassificationField( symbol->rotationClassificationField() );
    renderer->insertValue( it.key(), newsymbol );
  }
  renderer->updateSymbolAttributes();

  QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider *>( mVectorLayer->dataProvider() );
  if ( provider )
  {
    int fieldIndex = mClassificationComboBox->itemData( mClassificationComboBox->currentIndex() ).toInt();
    if ( fieldIndex != -1 )
    {
      renderer->setClassificationField( fieldIndex );
      mVectorLayer->setRenderer( renderer );
      return;
    }
  }

  delete renderer; //something went wrong
}


QColor QgsUniqueValueDialog::randomColor()
{
  QColor thecolor;

  //insert a random color
  int red = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int green = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  int blue = 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) );
  thecolor.setRgb( red, green, blue );

  return thecolor;
}

void QgsUniqueValueDialog::setSymbolColor( QgsSymbol *symbol, QColor thecolor )
{
  QPen pen;
  QBrush brush;
  if ( mVectorLayer->geometryType() == QGis::Line )
  {
    pen.setColor( thecolor );
    pen.setStyle( Qt::SolidLine );
    pen.setWidthF( symbol->lineWidth() );
  }
  else
  {
    brush.setColor( thecolor );
    brush.setStyle( Qt::SolidPattern );
    pen.setColor( Qt::black );
    pen.setStyle( Qt::SolidLine );
    pen.setWidthF( symbol->lineWidth() );
  }
  symbol->setPen( pen );
  symbol->setBrush( brush );
}

void QgsUniqueValueDialog::addClass( QString value )
{
  QgsDebugMsg( "called." );
  if ( mValues.contains( value ) )
  {
    int i;
    for ( i = 0; mValues.contains( value + QString::number( i ) ); i++ )
      ;
    value += QString::number( i );
  }

  QgsSymbol *symbol = new QgsSymbol( mVectorLayer->geometryType(), value, value, value.isNull() ? tr( "default" ) : "" );
  mValues.insert( value, symbol );

  QListWidgetItem *item = new QListWidgetItem( value );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
  item->setData( Qt::UserRole, value );
  item->setToolTip( symbol->label() );
  mClassListWidget->addItem( item );

  setSymbolColor( symbol, randomColor() );
  updateEntryIcon( symbol, item );
}

void QgsUniqueValueDialog::randomizeColors()
{
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  if ( selection.size() == 0 )
    selection = mClassListWidget->findItems( "", Qt::MatchContains );

  for ( int i = 0; i < selection.size(); i++ )
  {
    QListWidgetItem *item = selection[i];
    if ( !item )
      continue;

    if ( !mValues.contains( item->text() ) )
      continue;

    QgsSymbol *symbol = mValues[ item->text()];
    setSymbolColor( symbol, randomColor() );
    updateEntryIcon( symbol, item );
  }

  selectionChanged();
}

void QgsUniqueValueDialog::resetColors()
{
  QColor white;
  white.setRgb( 255, 255, 255 );

  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  if ( selection.size() == 0 )
    selection = mClassListWidget->findItems( "", Qt::MatchContains );

  for ( int i = 0; i < selection.size(); i++ )
  {
    QListWidgetItem *item = selection[i];
    if ( !item )
      continue;

    if ( !mValues.contains( item->text() ) )
      continue;

    QgsSymbol *symbol = mValues[ item->text()];
    setSymbolColor( symbol, white );
    updateEntryIcon( symbol, item );
  }

  selectionChanged();
}


void QgsUniqueValueDialog::changeClassificationAttribute()
{
  QgsDebugMsg( "called." );
  QString attributeName = mClassificationComboBox->currentText();

  if ( !mOldClassificationAttribute.isEmpty() &&
       attributeName != mOldClassificationAttribute &&
       QMessageBox::question( this,
                              tr( "Confirm Delete" ),
                              tr( "The classification field was changed from '%1' to '%2'.\n"
                                  "Should the existing classes be deleted before classification?" )
                              .arg( mOldClassificationAttribute ).arg( attributeName ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {
    mClassListWidget->clearSelection();
    deleteSelectedClasses();
  }
  mOldClassificationAttribute = attributeName;

  if ( mVectorLayer )
  {
    int nr = mClassificationComboBox->itemData( mClassificationComboBox->currentIndex() ).toInt();
    if ( nr == -1 )
    {
      return;
    }

    QList<QVariant> values;
    mVectorLayer->uniqueValues( nr, values );

    for ( int i = 0; i < values.size(); i++ )
    {
      if ( !mValues.contains( values[i].toString() ) )
        addClass( values[i].toString() );
    }

    if ( !mValues.contains( QString::null ) )
      addClass( QString::null );
  }
}

void QgsUniqueValueDialog::itemChanged( QListWidgetItem *item )
{
  QString oldValue = item->data( Qt::UserRole ).toString();
  QString newValue = item->text();

  if ( oldValue == newValue )
    return;

  if ( !mValues.contains( newValue ) )
  {
    QgsSymbol *sy = mValues[oldValue];
    mValues.remove( oldValue );
    mValues.insert( newValue, sy );
    sy->setLowerValue( newValue );
    item->setData( Qt::UserRole, newValue );
    updateEntryIcon( sy, item );
  }
  else
    item->setText( oldValue );
}


void QgsUniqueValueDialog::selectionChanged()
{
  QgsDebugMsg( "called." );
  sydialog.blockSignals( true );//block signal to prevent sydialog from changing the current QgsRenderItem
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();

  if ( selection.size() == 0 )
  {
    mDeletePushButton->setEnabled( false );
    sydialog.unset();
  }
  else
  {
    mDeletePushButton->setEnabled( true );

    if ( selection.size() == 1 || !mCommonPropertyLock->isChecked() )
    {
      QListWidgetItem *item = selection[0];
      if ( !item )
        return;

      if ( !mValues.contains( item->text() ) )
        return;

      QgsSymbol *symbol = mValues[ item->text()];
      sydialog.set( symbol );
      sydialog.setLabel( symbol->label() );
    }
    else if ( selection.size() > 1 && mCommonPropertyLock->isChecked() )
    {
      if ( !mValues.contains( selection[0]->text() ) )
        return;

      sydialog.set( mValues[ selection[0]->text()] );

      for ( int i = 1; i < selection.size(); i++ )
      {
        if ( !mValues.contains( selection[i]->text() ) )
          continue;

        sydialog.updateSet( mValues[ selection[i]->text()] );
      }
    }
  }
  sydialog.blockSignals( false );
}

void QgsUniqueValueDialog::deleteSelectedClasses()
{
  QgsDebugMsg( "called." );
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  if ( selection.size() == 0 )
    selection = mClassListWidget->findItems( "", Qt::MatchContains );

  for ( int i = 0; i < selection.size(); i++ )
  {
    QListWidgetItem* currentItem = selection[i];
    if ( !currentItem )
      continue;

    mValues.remove( currentItem->text() );

    mClassListWidget->removeItemWidget( currentItem );
    delete currentItem;
  }

  QgsDebugMsg( QString( "numRows: %1" ).arg( mClassListWidget->count() ) );
}

void QgsUniqueValueDialog::applySymbologyChanges()
{
  QgsDebugMsg( "called." );
  QList<QListWidgetItem *> selection = mClassListWidget->selectedItems();
  for ( int i = 0; i < selection.size(); i++ )
  {
    QListWidgetItem* item = selection[i];
    if ( !item )
    {
      QgsDebugMsg( QString( "selected item %1 not found" ).arg( i ) );
      continue;
    }

    QString value = item->text();
    if ( !mValues.contains( value ) )
    {
      QgsDebugMsg( QString( "value %1 not found" ).arg( value ) );
      continue;
    }

    QgsSymbol *symbol = mValues[ value ];
    symbol->setLowerValue( value );
    sydialog.apply( symbol );

    item->setToolTip( symbol->label() );
    item->setData( Qt::UserRole, value );
    updateEntryIcon( symbol, item );
  }
}

void QgsUniqueValueDialog::updateEntryIcon( QgsSymbol *thepSymbol, QListWidgetItem *thepItem )
{
  QGis::GeometryType myType = mVectorLayer->geometryType();
  switch ( myType )
  {
    case QGis::Point:
    {
      double size = thepSymbol->pointSize();
      if ( size > 50.0 )
        thepSymbol->setPointSize( 50.0 );
      thepItem->setIcon( QIcon( QPixmap::fromImage( thepSymbol->getPointSymbolAsImage( 4.0, false, Qt::yellow, 1.0, 0.0001 ) ) ) );
      if ( size > 50.0 )
        thepSymbol->setPointSize( size );
    }
    break;
    case QGis::Line:
      thepItem->setIcon( QIcon( QPixmap::fromImage( thepSymbol->getLineSymbolAsImage() ) ) );
      break;
    case QGis::Polygon:
      thepItem->setIcon( QIcon( QPixmap::fromImage( thepSymbol->getPolygonSymbolAsImage() ) ) );
      break;
    default: //unknown
      QgsDebugMsg( "Vector layer type unknown" );
      //do nothing
  }
}

void QgsUniqueValueDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
}
