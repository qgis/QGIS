#include "qgssinglesymbolrendererv2widget.h"

#include "qgssinglesymbolrendererv2.h"
#include "qgssymbolv2.h"

#include "qgsvectorlayer.h"

#include "qgssymbolv2selectordialog.h"

QgsRendererV2Widget* QgsSingleSymbolRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsSingleSymbolRendererV2Widget( layer, style, renderer );
}

QgsSingleSymbolRendererV2Widget::QgsSingleSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{
  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( !renderer || renderer->type() != "singleSymbol" )
  {
    // we're not going to use it - so let's delete the renderer
    delete renderer;

    // some default options
    QgsSymbolV2* symbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

    mRenderer = new QgsSingleSymbolRendererV2( symbol );
  }
  else
  {
    mRenderer = static_cast<QgsSingleSymbolRendererV2*>( renderer );
  }

  // load symbol from it
  mSingleSymbol = mRenderer->symbol()->clone();

  // setup ui
  mSelector = new QgsSymbolV2SelectorDialog( mSingleSymbol, mStyle, NULL, true );
  connect( mSelector, SIGNAL( symbolModified() ), this, SLOT( changeSingleSymbol() ) );

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget( mSelector );
  setLayout( layout );

}

QgsSingleSymbolRendererV2Widget::~QgsSingleSymbolRendererV2Widget()
{
  delete mSingleSymbol;

  delete mRenderer;

  delete mSelector;
}


QgsFeatureRendererV2* QgsSingleSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}

void QgsSingleSymbolRendererV2Widget::changeSingleSymbol()
{
  // update symbol from the GUI
  mRenderer->setSymbol( mSingleSymbol->clone() );
}

