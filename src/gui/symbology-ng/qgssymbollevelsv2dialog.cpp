
#include "qgssymbollevelsv2dialog.h"

#include "qgssymbollayerv2utils.h"
#include "qgssymbollayerv2.h"
#include "qgssymbolv2.h"

#include <QTableWidgetItem>

QgsSymbolLevelsV2Dialog::QgsSymbolLevelsV2Dialog( QgsSymbolV2List symbols, bool usingSymbolLevels, QWidget* parent )
    : QDialog( parent ), mSymbols( symbols )
{
  setupUi( this );

  chkEnable->setChecked( usingSymbolLevels );

  connect( chkEnable, SIGNAL( clicked() ), this, SLOT( updateUi() ) );

  int maxLayers = 0;
  tableLevels->setRowCount( symbols.count() );
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];

    // set icons for the rows
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( sym, QSize( 16, 16 ) );
    tableLevels->setVerticalHeaderItem( i, new QTableWidgetItem( icon, QString() ) );

    // find out max. number of layers per symbol
    int layers = sym->symbolLayerCount();
    if ( layers > maxLayers )
      maxLayers = layers;
  }

  tableLevels->setColumnCount( maxLayers );
  for ( int i = 0; i < maxLayers; i++ )
  {
    QString name = tr( "Layer %1" ).arg( i );
    tableLevels->setHorizontalHeaderItem( i, new QTableWidgetItem( name ) );
  }

  mMaxLayers = maxLayers;

  updateUi();

  if ( !usingSymbolLevels )
    setDefaultLevels();

  populateTable();

  connect( tableLevels, SIGNAL( cellChanged( int, int ) ), this, SLOT( renderingPassChanged( int, int ) ) );
}

void QgsSymbolLevelsV2Dialog::populateTable()
{
  for ( int row = 0; row < mSymbols.count(); row++ )
  {
    QgsSymbolV2* sym = mSymbols[row];
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
        item = new QTableWidgetItem( QString::number( sym->symbolLayer( layer )->renderingPass() ) );
      }
      tableLevels->setItem( row, layer, item );
    }
  }

}

void QgsSymbolLevelsV2Dialog::updateUi()
{
  tableLevels->setEnabled( chkEnable->isChecked() );
}

void QgsSymbolLevelsV2Dialog::setDefaultLevels()
{
  for ( int i = 0; i < mSymbols.count(); i++ )
  {
    QgsSymbolV2* sym = mSymbols[i];
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
  if ( row < 0 || row >= mSymbols.count() )
    return;
  QgsSymbolV2* sym = mSymbols[row];
  if ( column < 0 || column >= sym->symbolLayerCount() )
    return;
  sym->symbolLayer( column )->setRenderingPass( tableLevels->item( row, column )->text().toInt() );
}
