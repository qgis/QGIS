
#include "qgssymbolv2selectordialog.h"

#include "qgssymbolv2propertiesdialog.h"
#include "qgsstylev2managerdialog.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsstylev2.h"

#include "qgsapplication.h"

#include <QColorDialog>
#include <QPainter>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>

QgsSymbolV2SelectorDialog::QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, QWidget* parent, bool embedded )
    : QDialog( parent ), mAdvancedMenu( NULL )
{
  mStyle = style;
  mSymbol = symbol;

  setupUi( this );

  btnAdvanced->hide(); // advanced button is hidden by default

  // can be embedded in renderer properties dialog
  if ( embedded )
  {
    buttonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }

  connect( btnSymbolProperties, SIGNAL( clicked() ), this, SLOT( changeSymbolProperties() ) );
  connect( btnStyleManager, SIGNAL( clicked() ), SLOT( openStyleManager() ) );

  QStandardItemModel* model = new QStandardItemModel( viewSymbols );
  viewSymbols->setModel( model );
  connect( viewSymbols, SIGNAL( clicked( const QModelIndex & ) ), this, SLOT( setSymbolFromStyle( const QModelIndex & ) ) );

  populateSymbolView();
  updateSymbolPreview();
  updateSymbolInfo();

  if ( mSymbol )
  {
    // output unit
    mSymbolUnitComboBox->blockSignals( true );
    mSymbolUnitComboBox->setCurrentIndex( mSymbol->outputUnit() );
    mSymbolUnitComboBox->blockSignals( false );

    mTransparencySlider->blockSignals( true );
    double transparency = 1 - symbol->alpha();
    mTransparencySlider->setValue( transparency * 255 );
    displayTransparency( symbol->alpha() );
    mTransparencySlider->blockSignals( false );
  }

  // select correct page in stacked widget
  // there's a correspondence between symbol type number and page numbering => exploit it!
  stackedWidget->setCurrentIndex( symbol->type() );

  connect( btnColor, SIGNAL( clicked() ), this, SLOT( setSymbolColor() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerAngle( double ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerSize( double ) ) );
  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( setLineWidth( double ) ) );

  connect( btnAddToStyle, SIGNAL( clicked() ), this, SLOT( addSymbolToStyle() ) );
  btnSymbolProperties->setIcon( QIcon( QgsApplication::defaultThemePath() + "mActionOptions.png" ) );
  btnAddToStyle->setIcon( QIcon( QgsApplication::defaultThemePath() + "symbologyAdd.png" ) );
}

void QgsSymbolV2SelectorDialog::populateSymbolView()
{
  QSize previewSize = viewSymbols->iconSize();
  QPixmap p( previewSize );
  QPainter painter;

  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( viewSymbols->model() );
  if ( !model )
    return;
  model->clear();

  QStringList names = mStyle->symbolNames();
  for ( int i = 0; i < names.count(); i++ )
  {
    QgsSymbolV2* s = mStyle->symbol( names[i] );
    if ( s->type() != mSymbol->type() )
    {
      delete s;
      continue;
    }
    QStandardItem* item = new QStandardItem( names[i] );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    // create preview icon
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( s, previewSize );
    item->setIcon( icon );
    // add to model
    model->appendRow( item );
    delete s;
  }
}

void QgsSymbolV2SelectorDialog::setSymbolFromStyle( const QModelIndex & index )
{
  QString symbolName = index.data().toString();
  // get new instance of symbol from style
  QgsSymbolV2* s = mStyle->symbol( symbolName );
  // remove all symbol layers from original symbol
  while ( mSymbol->symbolLayerCount() )
    mSymbol->deleteSymbolLayer( 0 );
  // move all symbol layers to our symbol
  while ( s->symbolLayerCount() )
  {
    QgsSymbolLayerV2* sl = s->takeSymbolLayer( 0 );
    mSymbol->appendSymbolLayer( sl );
  }
  // delete the temporary symbol
  delete s;

  updateSymbolPreview();
  updateSymbolInfo();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::updateSymbolPreview()
{
  QImage preview = mSymbol->bigSymbolPreviewImage();
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
}

void QgsSymbolV2SelectorDialog::updateSymbolColor()
{
  btnColor->setColor( mSymbol->color() );
}

void QgsSymbolV2SelectorDialog::updateSymbolInfo()
{
  updateSymbolColor();

  if ( mSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
    spinSize->setValue( markerSymbol->size() );
    spinAngle->setValue( markerSymbol->angle() );
  }
  else if ( mSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
    spinWidth->setValue( lineSymbol->width() );
  }
}

void QgsSymbolV2SelectorDialog::changeSymbolProperties()
{
  QgsSymbolV2PropertiesDialog dlg( mSymbol, this );
  if ( !dlg.exec() )
    return;

  updateSymbolPreview();
  updateSymbolInfo();
  emit symbolModified();
}


void QgsSymbolV2SelectorDialog::setSymbolColor()
{
  QColor color = QColorDialog::getColor( mSymbol->color(), this );
  if ( !color.isValid() )
    return;

  mSymbol->setColor( color );
  updateSymbolColor();
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::setMarkerAngle( double angle )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  if ( markerSymbol->angle() == angle )
    return;
  markerSymbol->setAngle( angle );
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::setMarkerSize( double size )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  if ( markerSymbol->size() == size )
    return;
  markerSymbol->setSize( size );
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::setLineWidth( double width )
{
  QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
  if ( lineSymbol->width() == width )
    return;
  lineSymbol->setWidth( width );
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::addSymbolToStyle()
{
  bool ok;
  QString name = QInputDialog::getText( this, tr( "Symbol name" ),
                                        tr( "Please enter name for the symbol:" ) , QLineEdit::Normal, tr( "New symbol" ), &ok );
  if ( !ok || name.isEmpty() )
    return;

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( name, mSymbol->clone() );

  // make sure the symbol is stored
  mStyle->save();

  populateSymbolView();
}

void QgsSymbolV2SelectorDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( !isWindow() && e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
  else
  {
    QDialog::keyPressEvent( e );
  }
}

void QgsSymbolV2SelectorDialog::on_mSymbolUnitComboBox_currentIndexChanged( const QString & text )
{
  if ( mSymbol )
  {
    mSymbol->setOutputUnit(( QgsSymbolV2::OutputUnit ) mSymbolUnitComboBox->currentIndex() );

    updateSymbolPreview();
    emit symbolModified();
  }
}

void QgsSymbolV2SelectorDialog::on_mTransparencySlider_valueChanged( int value )
{
  if ( mSymbol )
  {
    double alpha = 1 - ( value / 255.0 );
    mSymbol->setAlpha( alpha );
    displayTransparency( alpha );
    updateSymbolPreview();
    emit symbolModified();
  }
}

void QgsSymbolV2SelectorDialog::displayTransparency( double alpha )
{
  double transparencyPercent = ( 1 - alpha ) * 100;
  mTransparencyLabel->setText( tr( "Transparency: %1%" ).arg(( int ) transparencyPercent ) );
}

QMenu* QgsSymbolV2SelectorDialog::advancedMenu()
{
  if ( mAdvancedMenu == NULL )
  {
    mAdvancedMenu = new QMenu;
    btnAdvanced->setMenu( mAdvancedMenu );
    btnAdvanced->show();
  }
  return mAdvancedMenu;
}

void QgsSymbolV2SelectorDialog::openStyleManager()
{
  QgsStyleV2ManagerDialog dlg( mStyle, this );
  dlg.exec();

  populateSymbolView();
}
