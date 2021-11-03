/***************************************************************************
    qgssymbollevelsdialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollevelsdialog.h"

#include "qgssymbollayerutils.h"
#include "qgssymbollayer.h"
#include "qgssymbol.h"
#include "qgssettings.h"
#include "qgsguiutils.h"

#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QSpinBox>
#include <QDialogButtonBox>


////////////////

QgsSymbolLevelsWidget::QgsSymbolLevelsWidget( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent )
  : QgsSymbolLevelsWidget( renderer->legendSymbolItems(), usingSymbolLevels, parent )
{
  mRenderer = renderer;
}

QgsSymbolLevelsWidget::QgsSymbolLevelsWidget( const QgsLegendSymbolList &symbols, bool usingSymbolLevels, QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  tableLevels->setItemDelegate( new SpinBoxDelegate( this ) );

  chkEnable->setChecked( usingSymbolLevels );

  connect( chkEnable, &QAbstractButton::clicked, this, &QgsSymbolLevelsWidget::updateUi );

  // only consider entries with symbols
  for ( const QgsLegendSymbolItem &item : symbols )
  {
    if ( item.symbol() )
      mLegendSymbols << item;
  }

  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  int maxLayers = 0;
  tableLevels->setRowCount( mLegendSymbols.count() );
  for ( int i = 0; i < mLegendSymbols.count(); i++ )
  {
    QgsSymbol *sym = mLegendSymbols.at( i ).symbol();

    // set icons for the rows
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( sym, QSize( iconSize, iconSize ) );
    tableLevels->setVerticalHeaderItem( i, new QTableWidgetItem( icon, QString() ) );

    // find out max. number of layers per symbol
    int layers = sym->symbolLayerCount();
    if ( layers > maxLayers )
      maxLayers = layers;
  }

  tableLevels->setColumnCount( maxLayers + 1 );
  tableLevels->setHorizontalHeaderItem( 0, new QTableWidgetItem( QString() ) );
  for ( int i = 0; i < maxLayers; i++ )
  {
    QString name = tr( "Layer %1" ).arg( i );
    tableLevels->setHorizontalHeaderItem( i + 1, new QTableWidgetItem( name ) );
  }

  mMaxLayers = maxLayers;

  updateUi();

  if ( !usingSymbolLevels )
    setDefaultLevels();

  populateTable();

  connect( tableLevels, &QTableWidget::cellChanged, this, &QgsSymbolLevelsWidget::renderingPassChanged );
}


void QgsSymbolLevelsWidget::populateTable()
{
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  for ( int row = 0; row < mLegendSymbols.count(); row++ )
  {
    QgsSymbol *sym = mLegendSymbols.at( row ).symbol();
    const QString label = mLegendSymbols.at( row ).label();
    QTableWidgetItem *itemLabel = new QTableWidgetItem( label );
    itemLabel->setFlags( itemLabel->flags() ^ Qt::ItemIsEditable );
    tableLevels->setItem( row, 0, itemLabel );
    for ( int layer = 0; layer < mMaxLayers; layer++ )
    {
      QTableWidgetItem *item = nullptr;
      if ( layer >= sym->symbolLayerCount() )
      {
        item = new QTableWidgetItem();
        item->setFlags( Qt::ItemFlags() );
      }
      else
      {
        const QgsSymbolLayer *sl = sym->symbolLayer( layer );
        const QIcon icon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( sl, QgsUnitTypes::RenderMillimeters, QSize( iconSize, iconSize ), QgsMapUnitScale(), sym->type() );
        item = new QTableWidgetItem( icon, QString::number( sl->renderingPass() ) );
      }
      tableLevels->setItem( row, layer + 1, item );
      tableLevels->resizeColumnToContents( 0 );
    }
  }
}

void QgsSymbolLevelsWidget::updateUi()
{
  tableLevels->setEnabled( chkEnable->isChecked() );
  emit widgetChanged();
}

void QgsSymbolLevelsWidget::apply()
{
  if ( !mRenderer )
    return;

  for ( const QgsLegendSymbolItem &legendSymbol : std::as_const( mLegendSymbols ) )
  {
    QgsSymbol *sym = legendSymbol.symbol();
    for ( int layer = 0; layer < sym->symbolLayerCount(); layer++ )
    {
      mRenderer->setLegendSymbolItem( legendSymbol.ruleKey(), sym->clone() );
    }
  }

  mRenderer->setUsingSymbolLevels( usingLevels() );
}

void QgsSymbolLevelsWidget::setDefaultLevels()
{
  for ( const QgsLegendSymbolItem &item : std::as_const( mLegendSymbols ) )
  {
    QgsSymbol *sym = item.symbol();
    for ( int layer = 0; layer < sym->symbolLayerCount(); layer++ )
    {
      sym->symbolLayer( layer )->setRenderingPass( layer );
    }
  }
}

bool QgsSymbolLevelsWidget::usingLevels() const
{
  return chkEnable->isChecked();
}

QgsLegendSymbolList QgsSymbolLevelsWidget::symbolLevels() const
{
  return mLegendSymbols;
}

void QgsSymbolLevelsWidget::renderingPassChanged( int row, int column )
{
  if ( row < 0 || row >= mLegendSymbols.count() )
    return;
  QgsSymbol *sym = mLegendSymbols.at( row ).symbol();
  if ( column < 0 || column > sym->symbolLayerCount() )
    return;
  sym->symbolLayer( column - 1 )->setRenderingPass( tableLevels->item( row, column )->text().toInt() );

  emit widgetChanged();
}

void QgsSymbolLevelsWidget::setForceOrderingEnabled( bool enabled )
{
  mForceOrderingEnabled = enabled;
  if ( enabled )
  {
    chkEnable->setChecked( true );
    chkEnable->hide();
  }
  else
    chkEnable->show();
}


//
// QgsSymbolLevelsDialog
//

QgsSymbolLevelsDialog::QgsSymbolLevelsDialog( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsSymbolLevelsWidget( renderer, usingSymbolLevels );
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsSymbolLevelsDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsSymbolLevelsDialog::reject );
  connect( bbox, &QDialogButtonBox::helpRequested, this, &QgsSymbolLevelsDialog::showHelp );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Symbol Levels" ) );
}

void QgsSymbolLevelsDialog::setForceOrderingEnabled( bool enabled )
{
  mWidget->setForceOrderingEnabled( enabled );
}

bool QgsSymbolLevelsDialog::usingLevels() const
{
  return mWidget->usingLevels();
}

QgsLegendSymbolList QgsSymbolLevelsDialog::symbolLevels() const
{
  return mWidget->symbolLevels();
}

void QgsSymbolLevelsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#symbols-levels" ) );
}

/// @cond PRIVATE

QWidget *SpinBoxDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QSpinBox *editor = new QSpinBox( parent );
  editor->setMinimum( 0 );
  editor->setMaximum( 999 );
  return editor;
}

void SpinBoxDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  int value = index.model()->data( index, Qt::EditRole ).toInt();
  QSpinBox *spinBox = static_cast<QSpinBox *>( editor );
  spinBox->setValue( value );
}

void SpinBoxDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QSpinBox *spinBox = static_cast<QSpinBox *>( editor );
  spinBox->interpretText();
  int value = spinBox->value();

  model->setData( index, value, Qt::EditRole );
}

void SpinBoxDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & ) const
{
  editor->setGeometry( option.rect );
}


///@endcond
