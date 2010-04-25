#include "qgssinglesymbolrendererv2widget.h"

#include "qgssinglesymbolrendererv2.h"
#include "qgssymbolv2.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgssymbolv2selectordialog.h"

#include <QMenu>

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

  // advanced actions - data defined rendering
  QMenu* advMenu = mSelector->advancedMenu();

  mRotationMenu = new QMenu( tr( "Rotation field" ) );
  mSizeScaleMenu = new QMenu( tr( "Size scale field" ) );

  populateMenu( mRotationMenu, SLOT( rotationFieldSelected() ), mRenderer->rotationField() );
  populateMenu( mSizeScaleMenu, SLOT( sizeScaleFieldSelected() ), mRenderer->sizeScaleField() );

  advMenu->addMenu( mRotationMenu );
  advMenu->addMenu( mSizeScaleMenu );
}

void QgsSingleSymbolRendererV2Widget::populateMenu( QMenu* menu, const char* slot, QString fieldName )
{
  QAction* aNo = menu->addAction( tr( "- no field -" ), this, slot );
  aNo->setCheckable( true );
  menu->addSeparator();

  bool hasField = false;
  const QgsFieldMap& flds = mLayer->pendingFields();
  for ( QgsFieldMap::const_iterator it = flds.begin(); it != flds.end(); ++it )
  {
    const QgsField& fld = it.value();
    if ( fld.type() == QVariant::Int || fld.type() == QVariant::Double )
    {
      QAction* a = menu->addAction( fld.name(), this, slot );
      a->setCheckable( true );
      if ( fieldName == fld.name() )
      {
        a->setChecked( true );
        hasField = true;
      }
    }
  }

  if ( !hasField )
    aNo->setChecked( true );
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

void QgsSingleSymbolRendererV2Widget::rotationFieldSelected()
{
  QObject* s = sender();
  if ( s == NULL )
    return;

  QAction* a = qobject_cast<QAction*>( s );
  if ( a == NULL )
    return;

  QString fldName = a->text();

  updateMenu( mRotationMenu, fldName );

  if ( fldName == tr( "- no field -" ) )
    fldName = QString();

  mRenderer->setRotationField( fldName );
}

void QgsSingleSymbolRendererV2Widget::sizeScaleFieldSelected()
{
  QObject* s = sender();
  if ( s == NULL )
    return;

  QAction* a = qobject_cast<QAction*>( s );
  if ( a == NULL )
    return;

  QString fldName = a->text();

  updateMenu( mSizeScaleMenu, fldName );

  if ( fldName == tr( "- no field -" ) )
    fldName = QString();

  mRenderer->setSizeScaleField( fldName );
}

void QgsSingleSymbolRendererV2Widget::updateMenu( QMenu* menu, QString fieldName )
{
  foreach( QAction* a, menu->actions() )
  {
    a->setChecked( a->text() == fieldName );
  }
}
