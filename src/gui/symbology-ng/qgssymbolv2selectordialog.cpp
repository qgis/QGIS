
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
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenu>

QgsSymbolV2SelectorDialog::QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent, bool embedded ):
  QDialog( parent ), mStyle( style ), mAdvancedMenu( NULL ), mVectorLayer( vl )
{
  mStyle = style;
  mSymbols.append( symbol );
  init( embedded );
}

QgsSymbolV2SelectorDialog::QgsSymbolV2SelectorDialog( QList<QgsSymbolV2*> symbols, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent, bool embedded )
  : QDialog( parent), mStyle( style ), mSymbols( symbols ), mAdvancedMenu( NULL ), mVectorLayer( vl )
{
  init( embedded );
}

void QgsSymbolV2SelectorDialog::init( bool embedded )
{
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
  lblSymbolName->setText( "" );
  populateSymbolView();
  updateSymbolPreview();
  updateSymbolInfo();

  //set dialog elements based upon the first symbol
  if( mSymbols.size() > 0 )
  {
    QgsSymbolV2* sym = mSymbols.at( 0 );
    if( sym )
    {
      // output unit
      mSymbolUnitComboBox->blockSignals( true );
      mSymbolUnitComboBox->setCurrentIndex( sym->outputUnit() );
      mSymbolUnitComboBox->blockSignals( false );

      mTransparencySlider->blockSignals( true );
      double transparency = 1 - sym->alpha();
      mTransparencySlider->setValue( transparency * 255 );
      displayTransparency( sym->alpha() );
      mTransparencySlider->blockSignals( false );

      stackedWidget->setCurrentIndex( sym->type() );
    }
  }

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

  QgsSymbolV2* firstSymbol = 0;
  if( mSymbols.size() > 0 )
  {
    firstSymbol = mSymbols.at( 0 );
  }

  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( viewSymbols->model() );
  if ( !model )
  {
    return;
  }
  model->clear();

  QStringList names = mStyle->symbolNames();
  for ( int i = 0; i < names.count(); i++ )
  {
    QgsSymbolV2* s = mStyle->symbol( names[i] );
    if ( firstSymbol && s->type() != firstSymbol->type() )
    {
      delete s;
      continue;
    }
    QStandardItem* item = new QStandardItem( names[i] );
    item->setData( names[i], Qt::UserRole ); //so we can show a label when it is clicked
    item->setText( "" ); //set the text to nothing and show in label when clicked rather
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
  QString symbolName = index.data( Qt::UserRole ).toString();
  lblSymbolName->setText( symbolName );
  // get new instance of symbol from style
  QgsSymbolV2* s = mStyle->symbol( symbolName );

  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    if( !(*symbolIt) )
    {
      return;
    }

    // remove all symbol layers from original symbol
    while ( (*symbolIt)->symbolLayerCount() )
      (*symbolIt)->deleteSymbolLayer( 0 );
    // move all symbol layers to our symbol
    while ( s->symbolLayerCount() )
    {
      QgsSymbolLayerV2* sl = s->takeSymbolLayer( 0 );
      (*symbolIt)->appendSymbolLayer( sl );
    }
  }

  // delete the temporary symbol
  delete s;

  updateSymbolPreview();
  updateSymbolInfo();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::updateSymbolPreview()
{
  if( mSymbols.size() < 1 )
  {
    return;
  }

  QImage preview = mSymbols.at(0)->bigSymbolPreviewImage();
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
}

void QgsSymbolV2SelectorDialog::updateSymbolColor()
{
  if( mSymbols.size() < 1 )
  {
    return;
  }
  btnColor->setColor( mSymbols.at(0)->color() );
}

void QgsSymbolV2SelectorDialog::updateSymbolInfo()
{
  if( mSymbols.size() < 1 )
  {
    return;
  }
  QgsSymbolV2* symbol = mSymbols.at( 0 );

  updateSymbolColor();

  if ( symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( symbol );
    spinSize->setValue( markerSymbol->size() );
    spinAngle->setValue( markerSymbol->angle() );
  }
  else if ( symbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( symbol );
    spinWidth->setValue( lineSymbol->width() );
  }
}

void QgsSymbolV2SelectorDialog::changeSymbolProperties()
{
  if( mSymbols.size() != 1 ) //makes only sense for one symbol
  {
    return;
  }
  QgsSymbolV2* symbol = mSymbols.at(0);

  QgsSymbolV2PropertiesDialog dlg( symbol, mVectorLayer, this );
  if ( !dlg.exec() )
    return;

  updateSymbolPreview();
  updateSymbolInfo();
  emit symbolModified();
}


void QgsSymbolV2SelectorDialog::setSymbolColor()
{
  if( mSymbols.size() < 1 )
  {
    return;
  }

#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mSymbol->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mSymbols.at(0)->color(), this );
#endif
  if ( !color.isValid() )
    return;

  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    (*symbolIt)->setColor( color );
  }
  updateSymbolColor();
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::setMarkerAngle( double angle )
{
  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( *symbolIt );
    if ( markerSymbol && markerSymbol->angle() == angle )
      return;
    markerSymbol->setAngle( angle );
  }
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::setMarkerSize( double size )
{
  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( *symbolIt );
    if ( markerSymbol->size() == size )
      return;
    markerSymbol->setSize( size );
  }
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::setLineWidth( double width )
{
  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( *symbolIt );
    if ( lineSymbol->width() == width )
      return;
    lineSymbol->setWidth( width );
  }
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::addSymbolToStyle()
{
  if( mSymbols.size() < 1 )
  {
    return;
  }

  QgsSymbolV2* symbol = mSymbols.at( 0 );
  bool ok;
  QString name = QInputDialog::getText( this, tr( "Symbol name" ),
                                        tr( "Please enter name for the symbol:" ) , QLineEdit::Normal, tr( "New symbol" ), &ok );
  if ( !ok || name.isEmpty() )
    return;

  // check if there is no symbol with same name
  if ( mStyle->symbolNames().contains( name ) )
  {
    int res = QMessageBox::warning( this, tr( "Save symbol" ),
                                    tr( "Symbol with name '%1' already exists. Overwrite?" )
                                    .arg( name ),
                                    QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
  }

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( name, symbol->clone() );

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
  Q_UNUSED( text );
  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    (*symbolIt)->setOutputUnit(( QgsSymbolV2::OutputUnit ) mSymbolUnitComboBox->currentIndex() );
  }

  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::on_mTransparencySlider_valueChanged( int value )
{
  double alpha = 1 - ( value / 255.0 );

  QList<QgsSymbolV2*>::iterator symbolIt = mSymbols.begin();
  for(; symbolIt != mSymbols.end(); ++symbolIt )
  {
    (*symbolIt)->setAlpha( alpha );
  }
  displayTransparency( alpha );
  updateSymbolPreview();
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::displayTransparency( double alpha )
{
  double transparencyPercent = ( 1 - alpha ) * 100;
  mTransparencyLabel->setText( tr( "Transparency %1%" ).arg(( int ) transparencyPercent ) );
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
