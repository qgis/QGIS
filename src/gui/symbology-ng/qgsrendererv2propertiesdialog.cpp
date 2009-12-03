
#include "qgsrendererv2propertiesdialog.h"

#include "qgsrendererv2.h"
#include "qgsrendererv2registry.h"

#include "qgsrendererv2widget.h"
#include "qgssinglesymbolrendererv2widget.h"
#include "qgscategorizedsymbolrendererv2widget.h"
#include "qgsgraduatedsymbolrendererv2widget.h"

#include "qgssymbollevelsv2dialog.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <QKeyEvent>
#include <QMessageBox>

QgsRendererV2PropertiesDialog::QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded )
    : mLayer( layer ), mStyle( style ), mActiveWidget( NULL )
{
  setupUi( this );

  // can be embedded in vector layer properties
  if ( embedded )
  {
    buttonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );
  connect( btnSymbolLevels, SIGNAL( clicked() ), this, SLOT( showSymbolLevels() ) );
  connect( btnOldSymbology, SIGNAL( clicked() ), this, SLOT( useOldSymbology() ) );

  // initialize registry's widget functions
  QgsRendererV2Registry* reg = QgsRendererV2Registry::instance();
  if ( reg->rendererMetadata( "singleSymbol" ).widgetFunction() == NULL )
  {
    reg->setRendererWidgetFunction( "singleSymbol", QgsSingleSymbolRendererV2Widget::create );
    reg->setRendererWidgetFunction( "categorizedSymbol", QgsCategorizedSymbolRendererV2Widget::create );
    reg->setRendererWidgetFunction( "graduatedSymbol", QgsGraduatedSymbolRendererV2Widget::create );
  }

  QPixmap pix;
  QStringList renderers = reg->renderersList();
  foreach( QString name, renderers )
  {
    QgsRendererV2Metadata m = reg->rendererMetadata( name );

    QString iconPath = QgsApplication::defaultThemePath() + m.iconName();
    if ( !pix.load( iconPath, "png" ) )
      pix = QPixmap();

    cboRenderers->addItem( QIcon( pix ), m.visibleName(), name );
  }

  cboRenderers->setCurrentIndex( -1 ); // set no current renderer

  // if the layer doesn't use renderer V2, let's start using it!
  if ( !mLayer->isUsingRendererV2() )
  {
    mLayer->setRendererV2( QgsFeatureRendererV2::defaultRenderer( mLayer->geometryType() ) );
    mLayer->setUsingRendererV2( true );
  }

  // setup slot rendererChanged()
  connect( cboRenderers, SIGNAL( currentIndexChanged( int ) ), this, SLOT( rendererChanged() ) );

  // set current renderer from layer
  QString rendererName = mLayer->rendererV2()->type();
  for ( int i = 0; i < cboRenderers->count(); i++ )
  {
    if ( cboRenderers->itemData( i ).toString() == rendererName )
    {
      cboRenderers->setCurrentIndex( i );
      return;
    }
  }

  // no renderer found... this mustn't happen
  Q_ASSERT( false && "there must be a renderer!" );

}


void QgsRendererV2PropertiesDialog::rendererChanged()
{

  if ( cboRenderers->currentIndex() == -1 )
  {
    QgsDebugMsg( "No current item -- this should never happen!" );
    return;
  }

  QString rendererName = cboRenderers->itemData( cboRenderers->currentIndex() ).toString();

  // get rid of old active widget (if any)
  if ( mActiveWidget )
  {
    stackedWidget->removeWidget( mActiveWidget );

    delete mActiveWidget;
    mActiveWidget = NULL;
  }

  QgsRendererV2Metadata m = QgsRendererV2Registry::instance()->rendererMetadata( rendererName );
  QgsRendererV2WidgetFunc fWidget = m.widgetFunction();
  if ( fWidget != NULL )
  {
    // instantiate the widget and set as active
    mActiveWidget = fWidget( mLayer, mStyle, mLayer->rendererV2()->clone() );
    stackedWidget->addWidget( mActiveWidget );
    stackedWidget->setCurrentWidget( mActiveWidget );

    btnSymbolLevels->setEnabled( true );
  }
  else
  {
    // set default "no edit widget available" page
    stackedWidget->setCurrentWidget( pageNoWidget );

    btnSymbolLevels->setEnabled( false );
  }

}

void QgsRendererV2PropertiesDialog::apply()
{
  if ( mActiveWidget != NULL )
  {
    mLayer->setRendererV2( mActiveWidget->renderer()->clone() );
  }
}

void QgsRendererV2PropertiesDialog::onOK()
{
  apply();
  accept();
}


void QgsRendererV2PropertiesDialog::keyPressEvent( QKeyEvent * e )
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


void QgsRendererV2PropertiesDialog::showSymbolLevels()
{
  if ( !mActiveWidget )
    return;

  QgsFeatureRendererV2* r = mActiveWidget->renderer();
  QgsSymbolV2List symbols = r->symbols();

  QgsSymbolLevelsV2Dialog dlg( symbols, r->usingSymbolLevels(), this );
  if ( dlg.exec() )
  {
    r->setUsingSymbolLevels( dlg.usingLevels() );
  }
}

void QgsRendererV2PropertiesDialog::useOldSymbology()
{
  int res = QMessageBox::question( this, tr( "Symbology" ),
                                   tr( "Do you wish to use the original symbology implementation for this layer?" ),
                                   QMessageBox::Yes | QMessageBox::No );

  if ( res != QMessageBox::Yes )
    return;

  emit useNewSymbology( false );
}
