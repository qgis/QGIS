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

#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgssymbolv2.h"
#include "qgslayertreelayer.h"
#include "qgslayertreemodellegendnode.h"
#include "qgssymbollayerv2utils.h"
#include "qgsscaleexpression.h"
#include "qgsdatadefined.h"

#include <QMenu>
#include <QAction>
#include <QItemDelegate>

#include <limits>


class ItemDelegate : public QItemDelegate
{
  public:
    ItemDelegate( QStandardItemModel* model ): mModel( model ) {}

    QSize sizeHint( const QStyleOptionViewItem& /*option*/, const QModelIndex & index ) const override
    {
      return mModel->item( index.row() )->icon().actualSize( QSize( 512, 512 ) );
    }

  private:
    QStandardItemModel* mModel;

};

void QgsSizeScaleWidget::setFromSymbol()
{
  if ( !mSymbol )
  {
    return;
  }

  QgsDataDefined ddSize = mSymbol->dataDefinedSize();
  QgsScaleExpression expr( ddSize.expressionString() );
  if ( expr )
  {
    for ( int i = 0; i < scaleMethodComboBox->count(); i++ )
    {
      if ( scaleMethodComboBox->itemData( i ).toInt() == int( expr.type() ) )
      {
        scaleMethodComboBox->setCurrentIndex( i );
        break;
      }
    }

    mExpressionWidget->setField( expr.baseExpression() );

    minValueSpinBox->setValue( expr.minValue() );
    maxValueSpinBox->setValue( expr.maxValue() );
    minSizeSpinBox->setValue( expr.minSize() );
    maxSizeSpinBox->setValue( expr.maxSize() );
    nullSizeSpinBox->setValue( expr.nullSize() );
  }
  updatePreview();
}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope();

  const QgsVectorLayer* layer = ( const QgsVectorLayer* ) context;
  if ( layer )
    expContext << QgsExpressionContextUtils::layerScope( layer );

  return expContext;
}

QgsSizeScaleWidget::QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsMarkerSymbolV2 * symbol )
    : mSymbol( symbol )
    // we just use the minimumValue and maximumValue from the layer, unfortunately they are
    // non const, so we get the layer from the registry instead
    , mLayer( layer ? dynamic_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( layer->id() ) ) : 0 )
{
  setupUi( this );
  setWindowFlags( Qt::WindowStaysOnTopHint );

  mExpressionWidget->registerGetExpressionContextCallback( &_getExpressionContext, mLayer );

  if ( mLayer )
  {
    mLayerTreeLayer = new QgsLayerTreeLayer( mLayer );
    mRoot.addChildNode( mLayerTreeLayer ); // takes ownership
  }
  else
  {
    mLayerTreeLayer = 0;
  }

  treeView->setModel( &mPreviewList );
  treeView->setItemDelegate( new ItemDelegate( &mPreviewList ) );
  treeView->setHeaderHidden( true );
  treeView->expandAll();

  QAction* computeFromLayer = new QAction( tr( "Compute from layer" ), this );
  connect( computeFromLayer, SIGNAL( triggered() ), this, SLOT( computeFromLayerTriggered() ) );

  QMenu* menu = new QMenu();
  menu->addAction( computeFromLayer );
  computeValuesButton->setMenu( menu );
  connect( computeValuesButton, SIGNAL( clicked() ), computeValuesButton, SLOT( showMenu() ) );

  //mExpressionWidget->setFilters( QgsFieldProxyModel::Numeric | QgsFieldProxyModel::Date );
  if ( mLayer )
  {
    mExpressionWidget->setLayer( mLayer );
  }

  scaleMethodComboBox->addItem( tr( "Flannery" ), int( QgsScaleExpression::Flannery ) );
  scaleMethodComboBox->addItem( tr( "Surface" ), int( QgsScaleExpression::Area ) );
  scaleMethodComboBox->addItem( tr( "Radius" ), int( QgsScaleExpression::Linear ) );

  minSizeSpinBox->setShowClearButton( false );
  maxSizeSpinBox->setShowClearButton( false );
  minValueSpinBox->setShowClearButton( false );
  maxValueSpinBox->setShowClearButton( false );
  nullSizeSpinBox->setShowClearButton( false );

  // setup ui from expression if any
  setFromSymbol();

  connect( minSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( maxSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( minValueSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( maxValueSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( nullSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  //potentially very expensive for large layers:
  connect( mExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SLOT( computeFromLayerTriggered() ) );
  connect( scaleMethodComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
}

QgsDataDefined QgsSizeScaleWidget::dataDefined() const
{
  QScopedPointer<QgsScaleExpression> exp( createExpression() );
  return QgsDataDefined( exp.data() );
}

void QgsSizeScaleWidget::showEvent( QShowEvent* )
{
  setFromSymbol();
}

QgsScaleExpression *QgsSizeScaleWidget::createExpression() const
{
  return new QgsScaleExpression( QgsScaleExpression::Type( scaleMethodComboBox->itemData( scaleMethodComboBox->currentIndex() ).toInt() ),
                                 mExpressionWidget->currentField(),
                                 minValueSpinBox->value(),
                                 maxValueSpinBox->value(),
                                 minSizeSpinBox->value(),
                                 maxSizeSpinBox->value(),
                                 nullSizeSpinBox->value() );
}

void QgsSizeScaleWidget::updatePreview()
{
  if ( !mSymbol || !mLayer )
    return;

  QScopedPointer<QgsScaleExpression> expr( createExpression() );
  QList<double> breaks = QgsSymbolLayerV2Utils::prettyBreaks( expr->minValue(), expr->maxValue(), 4 );

  treeView->setIconSize( QSize( 512, 512 ) );
  mPreviewList.clear();
  int widthMax = 0;
  for ( int i = 0; i < breaks.length(); i++ )
  {
    QScopedPointer< QgsMarkerSymbolV2 > symbol( dynamic_cast<QgsMarkerSymbolV2*>( mSymbol->clone() ) );
    symbol->setDataDefinedSize( QgsDataDefined() );
    symbol->setDataDefinedAngle( QgsDataDefined() ); // to avoid symbol not beeing drawn
    symbol->setSize( expr->size( breaks[i] ) );
    QgsSymbolV2LegendNode node( mLayerTreeLayer, QgsLegendSymbolItemV2( symbol.data(), QString::number( i ), 0 ) );
    const QSize sz( node.minimumIconSize() );
    node.setIconSize( sz );
    QScopedPointer< QStandardItem > item( new QStandardItem( node.data( Qt::DecorationRole ).value<QPixmap>(), QString::number( breaks[i] ) ) );
    widthMax = qMax( sz.width(), widthMax );
    mPreviewList.appendRow( item.take() );
  }

  // center icon and align text left by giving icons the same width
  // @todo maybe add some space so that icons don't touch
  for ( int i = 0; i < breaks.length(); i++ )
  {
    QPixmap img( mPreviewList.item( i )->icon().pixmap( mPreviewList.item( i )->icon().actualSize( QSize( 512, 512 ) ) ) );
    QPixmap enlarged( widthMax, img.height() );
    // fill transparent and add original image
    enlarged.fill( Qt::transparent );
    QPainter p( &enlarged );
    p.drawPixmap( QPoint(( widthMax - img.width() ) / 2, 0 ), img );
    p.end();
    mPreviewList.item( i )->setIcon( enlarged );
  }
}

void QgsSizeScaleWidget::computeFromLayerTriggered()
{
  if ( !mLayer )
    return;

  QgsExpression expression( mExpressionWidget->currentField() );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  if ( ! expression.prepare( &context ) )
    return;

  QStringList lst( expression.referencedColumns() );

  QgsFeatureIterator fit = mLayer->getFeatures(
                             QgsFeatureRequest().setFlags( expression.needsGeometry()
                                                           ? QgsFeatureRequest::NoFlags
                                                           : QgsFeatureRequest::NoGeometry )
                             .setSubsetOfAttributes( lst, mLayer->fields() ) );

  // create list of non-null attribute values
  double min = DBL_MAX;
  double max = -DBL_MAX;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    bool ok;
    context.setFeature( f );
    const double value = expression.evaluate( &context ).toDouble( &ok );
    if ( ok )
    {
      max = qMax( max, value );
      min = qMin( min, value );
    }
  }
  minValueSpinBox->setValue( min );
  maxValueSpinBox->setValue( max );
  updatePreview();
}

