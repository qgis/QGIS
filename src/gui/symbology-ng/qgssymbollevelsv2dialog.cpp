/***************************************************************************
    qgssymbollevelsv2dialog.cpp
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

#include "qgssymbollevelsv2dialog.h"

#include "qgssymbollayerv2utils.h"
#include "qgssymbollayerv2.h"
#include "qgssymbolv2.h"

#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QSpinBox>

// delegate used from Qt Spin Box example
class SpinBoxDelegate : public QItemDelegate
{
  public:
    SpinBoxDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &/*index*/ ) const
    {
      QSpinBox *editor = new QSpinBox( parent );
      editor->setMinimum( 0 );
      editor->setMaximum( 999 );
      return editor;
    }

    void setEditorData( QWidget *editor, const QModelIndex &index ) const
    {
      int value = index.model()->data( index, Qt::EditRole ).toInt();
      QSpinBox *spinBox = static_cast<QSpinBox*>( editor );
      spinBox->setValue( value );
    }

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
    {
      QSpinBox *spinBox = static_cast<QSpinBox*>( editor );
      spinBox->interpretText();
      int value = spinBox->value();

      model->setData( index, value, Qt::EditRole );
    }

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/ ) const
    {
      editor->setGeometry( option.rect );
    }

};

////////////////

QgsSymbolLevelsV2Dialog::QgsSymbolLevelsV2Dialog( QgsLegendSymbolList list, bool usingSymbolLevels, QWidget* parent )
    : QDialog( parent ), mList( list ), mForceOrderingEnabled( false )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/symbolLevelsDlg/geometry" ).toByteArray() );

  tableLevels->setItemDelegate( new SpinBoxDelegate( this ) );

  chkEnable->setChecked( usingSymbolLevels );

  connect( chkEnable, SIGNAL( clicked() ), this, SLOT( updateUi() ) );

  if ( mList.count() > 0 && !mList[0].second )
  {
    // remove symbolless entry (probably classifier of categorized renderer)
    mList.removeFirst();
  }

  int maxLayers = 0;
  tableLevels->setRowCount( mList.count() );
  for ( int i = 0; i < mList.count(); i++ )
  {
    QgsSymbolV2* sym = mList[i].second;

    // set icons for the rows
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( sym, QSize( 16, 16 ) );
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

  connect( tableLevels, SIGNAL( cellChanged( int, int ) ), this, SLOT( renderingPassChanged( int, int ) ) );
}

QgsSymbolLevelsV2Dialog::~QgsSymbolLevelsV2Dialog()
{
  QSettings settings;
  settings.setValue( "/Windows/symbolLevelsDlg/geometry", saveGeometry() );
}

void QgsSymbolLevelsV2Dialog::populateTable()
{
  for ( int row = 0; row < mList.count(); row++ )
  {
    QgsSymbolV2* sym = mList[row].second;
    QString label = mList[row].first;
    QTableWidgetItem *itemLabel = new QTableWidgetItem( label );
    itemLabel->setFlags( itemLabel->flags() ^ Qt::ItemIsEditable );
    tableLevels->setItem( row, 0, itemLabel );
    for ( int layer = 0; layer < mMaxLayers; layer++ )
    {
      QTableWidgetItem* item;
      if ( layer >= sym->symbolLayerCount() )
      {
        item = new QTableWidgetItem();
        item->setFlags( Qt::ItemFlags() );
      }
      else
      {
        QgsSymbolLayerV2* sl = sym->symbolLayer( layer );
        QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( sl, QgsSymbolV2::MM, QSize( 16, 16 ) );
        item = new QTableWidgetItem( icon, QString::number( sl->renderingPass() ) );
      }
      tableLevels->setItem( row, layer + 1, item );
      tableLevels->resizeColumnToContents( 0 );
    }
  }

}

void QgsSymbolLevelsV2Dialog::updateUi()
{
  tableLevels->setEnabled( chkEnable->isChecked() );
}

void QgsSymbolLevelsV2Dialog::setDefaultLevels()
{
  for ( int i = 0; i < mList.count(); i++ )
  {
    QgsSymbolV2* sym = mList[i].second;
    for ( int layer = 0; layer < sym->symbolLayerCount(); layer++ )
    {
      sym->symbolLayer( layer )->setRenderingPass( layer );
    }
  }
}

bool QgsSymbolLevelsV2Dialog::usingLevels() const
{
  return chkEnable->isChecked();
}

void QgsSymbolLevelsV2Dialog::renderingPassChanged( int row, int column )
{
  if ( row < 0 || row >= mList.count() )
    return;
  QgsSymbolV2* sym = mList[row].second;
  if ( column < 0 || column > sym->symbolLayerCount() )
    return;
  sym->symbolLayer( column - 1 )->setRenderingPass( tableLevels->item( row, column )->text().toInt() );
}

void QgsSymbolLevelsV2Dialog::setForceOrderingEnabled( bool enabled )
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
