/***************************************************************************
 qgssizescalewidget.cpp - continuous size scale assistant

 ---------------------
 begin                : March 2015
 copyright            : (C) 2015 by Vincent Mora
 email                : vincent dot mora at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssizescalewidget.h"

#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgslayertreelayer.h>
#include <qgslayertreemodellegendnode.h>
#include <qgssymbollayerv2utils.h>

#include <QMenu>
#include <QAction>
#include <QItemDelegate>

#include <limits>



struct ItemDelegate : QItemDelegate
{
  ItemDelegate( QStandardItemModel * model ): mModel( model ) {}

  QSize sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
  {
    return mModel->item( index.row() )->icon().actualSize( QSize( 512, 512 ) );
  }
private:
  QStandardItemModel * mModel;

};

QgsSizeScaleWidget::QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsMarkerSymbolV2 * symbol )
    : mSymbol( symbol )
    // we just use the minimumValue and maximumValue from the layer, unfortunately they are
    // non const, so we get the layer from the registry instead
    , mLayer( dynamic_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( layer->id() ) ) )
//  , mModel( &mRoot, this )
{
  setupUi( this );
  setWindowFlags( Qt::WindowStaysOnTopHint );

  mLayerTreeLayer = new QgsLayerTreeLayer( mLayer );
  mRoot.addChildNode( mLayerTreeLayer ); // takes ownership

  treeView->setModel( &mPreviewList );
  treeView->setItemDelegate( new ItemDelegate( &mPreviewList ) );
  treeView->setHeaderHidden( true );
  treeView->expandAll();

  QScopedPointer<QAction> computeFromLayer( new QAction( tr( "Compute from layer" ), this ) );
  connect( computeFromLayer.data(), SIGNAL( triggered() ), this, SLOT( computeFromLayerTriggered() ) );

  QScopedPointer<QMenu> menu( new QMenu );
  menu->addAction( computeFromLayer.take() );
  computeValuesButton->setMenu( menu.take() );
  connect( computeValuesButton, SIGNAL( clicked() ), computeValuesButton, SLOT( showMenu() ) );

  fieldComboBox->addItem( tr( "[choose field]" ) );
  const QgsFields& fields = mLayer->pendingFields();
  for ( int i = 0; i < fields.count(); ++i )
  {
    fieldComboBox->addItem( fields.at( i ).name() );
  }

  scaleMethodComboBox->addItem( tr( "Flannery" ) );
  scaleMethodComboBox->addItem( tr( "Surface" ) );
  scaleMethodComboBox->addItem( tr( "Radius" ) );

  // parse the current size expression of the symbol if it exists
  QString expr = mSymbol->sizeExpression();
  if ( expr.length() )
  {
    QStringList args;
    if ( expr.indexOf( "scale_linear(" ) == 0 )
    {
      args = expr.mid( 13, expr.length() - 14 ).split( "," );
      if ( args.length() == 5 )
        scaleMethodComboBox->setCurrentIndex( 2 );
      else
        args = QStringList();
    }
    else if ( expr.indexOf( "scale_exp(" ) == 0 )
    {
      args = expr.mid( 10, expr.length() - 11 ).split( "," );
      if ( args.length() == 6 )
      {
        if ( qAbs( QVariant( args[5] ).toDouble() - .57 ) < .001 )
          scaleMethodComboBox->setCurrentIndex( 1 );
        else
          scaleMethodComboBox->setCurrentIndex( 0 );
      }
      else
        args = QStringList();
    }

    if ( args.length() >= 5 )
    {
      for ( int i = 0; i < fieldComboBox->count(); i++ )
      {
        if ( args[0].trimmed().replace( "\"", "" ) == fieldComboBox->itemText( i ) )
        {
          fieldComboBox->setCurrentIndex( i );
          break;
        }
      }
      minValueSpinBox->setValue( QVariant( args[1] ).toDouble() );
      maxValueSpinBox->setValue( QVariant( args[2] ).toDouble() );
      minSizeSpinBox->setValue( QVariant( args[3] ).toDouble() );
      maxSizeSpinBox->setValue( QVariant( args[4] ).toDouble() );
      updatePreview();
    }
  }

  connect( minSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( maxSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( minValueSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( maxValueSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( fieldComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( computeFromLayerTriggered() ) );
  connect( scaleMethodComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
}

QString QgsSizeScaleWidget::expressionText() const
{
  return expressionTextImpl();
}

QString QgsSizeScaleWidget::expressionTextImpl( const QString & value ) const
{
  const QString args =
    ( value.length() ? value : QgsExpression::quotedColumnRef( fieldComboBox->currentText() ) )
    + QString( ", " )
    + QString::number( minValueSpinBox->value() ) + ", "
    + QString::number( maxValueSpinBox->value() ) + ", "
    + QString::number( minSizeSpinBox->value() ) + ", "
    + QString::number( maxSizeSpinBox->value() );
  switch ( scaleMethodComboBox->currentIndex() )
  {
    case 0: return "scale_exp("+args+", .57)";
    case 1: return "scale_exp("+args+", .5)";
    case 2: return "scale_linear("+args+")";
  }
  return "";
}

void QgsSizeScaleWidget::updatePreview()
{
  const double min = minValueSpinBox->value();
  const double max = maxValueSpinBox->value();
  QList<double> breaks = QgsSymbolLayerV2Utils::prettyBreaks( min, max, 5 );

  treeView->setIconSize( QSize( 512, 512 ) );
  mPreviewList.clear();
  int widthMax = 0;
  for ( int i = 0; i < breaks.length(); i++ )
  {
    QScopedPointer< QgsMarkerSymbolV2 > symbol( dynamic_cast<QgsMarkerSymbolV2*>( mSymbol->clone() ) );
    QgsExpression ex( expressionTextImpl( QString::number( breaks[i] ) ) );
    symbol->setSizeExpression( "" );
    symbol->setAngleExpression( "" ); // to avoid symbol not beeing drawn
    symbol->setSize( ex.evaluate().toDouble() );
    QgsSymbolV2LegendNode node( mLayerTreeLayer, QgsLegendSymbolItemV2( symbol.data(), QString::number( i ), 0 ) );
    QScopedPointer< QStandardItem > item( new QStandardItem( node.data( Qt::DecorationRole ).value<QPixmap>(), QString::number( breaks[i] ) ) );
    widthMax = qMax( item->icon().actualSize( QSize( 512, 512 ) ).rwidth(), widthMax );
    mPreviewList.appendRow( item.take() );
  }

  // center icon and align text left by givinq icons the same width
  // @todo maybe add some space so that icons don't touch
  for ( int i = 0; i < breaks.length(); i++ )
  {
    QPixmap img( mPreviewList.item( i )->icon().pixmap( mPreviewList.item( i )->icon().actualSize( QSize( 512, 512 ) ) ) );
    QPixmap enlarged( widthMax, img.height() );
    // fill transparent and add original image
    {
      enlarged.fill( Qt::transparent );
      QPainter p( &enlarged );
      p.setCompositionMode( QPainter::CompositionMode_DestinationOver );
      p.drawPixmap( QPoint(( widthMax - img.width() ) / 2, 0 ), img );
    }
    mPreviewList.item( i )->setIcon( enlarged );
  }

  //mModel.refreshLayerLegend( mLayerTreeLayer );
}

void QgsSizeScaleWidget::computeFromLayerTriggered()
{
  const int idx = mLayer->fieldNameIndex( fieldComboBox->currentText() );
  bool minOk, maxOk;
  double min = mLayer->minimumValue( idx ).toDouble( &minOk );
  double max = mLayer->maximumValue( idx ).toDouble( &maxOk );
  if ( !( minOk && maxOk ) )
  {
    min = 0;
    max = 0;
  }
  minValueSpinBox->setValue( min );
  maxValueSpinBox->setValue( max );
  updatePreview();
}

