#include "qgsgraduatedsymbolrendererv2widget.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"

#include "qgsvectorlayer.h"

#include "qgsgraduatedsymbolrendererv2.h"

#include "qgssymbolv2selectordialog.h"

#include "qgsludialog.h"

#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>


QgsRendererV2Widget* QgsGraduatedSymbolRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsGraduatedSymbolRendererV2Widget( layer, style, renderer );
}

QgsGraduatedSymbolRendererV2Widget::QgsGraduatedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( !renderer || renderer->type() != "graduatedSymbol" )
  {
    // we're not going to use it - so let's delete the renderer
    delete renderer;

    mRenderer = new QgsGraduatedSymbolRendererV2( "", QgsRangeList() );
  }
  else
  {
    mRenderer = static_cast<QgsGraduatedSymbolRendererV2*>( renderer );
  }

  // setup user interface
  setupUi( this );

  populateColumns();

  cboGraduatedColorRamp->populate( mStyle );

  QStandardItemModel* mg = new QStandardItemModel( this );
  QStringList labels;
  labels << tr( "Range" ) << tr( "Label" );
  mg->setHorizontalHeaderLabels( labels );
  viewGraduated->setModel( mg );

  mGraduatedSymbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

  connect( cboGraduatedColumn, SIGNAL( currentIndexChanged( int ) ), this, SLOT( graduatedColumnChanged() ) );
  connect( viewGraduated, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( rangesDoubleClicked( const QModelIndex & ) ) );
  connect( viewGraduated, SIGNAL( clicked( const QModelIndex & ) ), this, SLOT( rangesClicked( const QModelIndex & ) ) );
  connect( mg, SIGNAL( itemChanged( QStandardItem * ) ), this, SLOT( changeCurrentValue( QStandardItem * ) ) );
  connect( btnGraduatedClassify, SIGNAL( clicked() ), this, SLOT( classifyGraduated() ) );
  connect( btnChangeGraduatedSymbol, SIGNAL( clicked() ), this, SLOT( changeGraduatedSymbol() ) );
  connect( btnGraduatedDelete, SIGNAL( clicked() ), this, SLOT( deleteCurrentClass() ) );
  connect( btnGraduatedAdd, SIGNAL( clicked() ), this, SLOT( addClass() ) );



  // initialize from previously set renderer
  updateUiFromRenderer();

  // menus for data-defined rotation/size
  QMenu* advMenu = new QMenu;
  mDataDefinedMenus = new QgsRendererV2DataDefinedMenus( advMenu, mLayer->pendingFields(),
      mRenderer->rotationField(), mRenderer->sizeScaleField() );
  connect( mDataDefinedMenus, SIGNAL( rotationFieldChanged( QString ) ), this, SLOT( rotationFieldChanged( QString ) ) );
  connect( mDataDefinedMenus, SIGNAL( sizeScaleFieldChanged( QString ) ), this, SLOT( sizeScaleFieldChanged( QString ) ) );
  btnAdvanced->setMenu( advMenu );
}

QgsGraduatedSymbolRendererV2Widget::~QgsGraduatedSymbolRendererV2Widget()
{
  delete mRenderer;
}

QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}


void QgsGraduatedSymbolRendererV2Widget::updateUiFromRenderer()
{

  updateGraduatedSymbolIcon();
  populateRanges();

  // update UI from the graduated renderer (update combo boxes, view)
  if ( mRenderer->mode() < cboGraduatedMode->count() )
    cboGraduatedMode->setCurrentIndex( mRenderer->mode() );
  if ( mRenderer->ranges().count() )
    spinGraduatedClasses->setValue( mRenderer->ranges().count() );

  // set column
  disconnect( cboGraduatedColumn, SIGNAL( currentIndexChanged( int ) ), this, SLOT( graduatedColumnChanged() ) );
  QString attrName = mRenderer->classAttribute();
  int idx = cboGraduatedColumn->findText( attrName, Qt::MatchExactly );
  cboGraduatedColumn->setCurrentIndex( idx >= 0 ? idx : 0 );
  connect( cboGraduatedColumn, SIGNAL( currentIndexChanged( int ) ), this, SLOT( graduatedColumnChanged() ) );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    delete mGraduatedSymbol;
    mGraduatedSymbol = mRenderer->sourceSymbol()->clone();
    updateGraduatedSymbolIcon();
  }

  // set source color ramp
  if ( mRenderer->sourceColorRamp() )
  {
    cboGraduatedColorRamp->setSourceColorRamp( mRenderer->sourceColorRamp() );
  }

}



void QgsGraduatedSymbolRendererV2Widget::populateColumns()
{
  cboGraduatedColumn->clear();
  const QgsFieldMap& flds = mLayer->pendingFields();
  QgsFieldMap::ConstIterator it = flds.begin();
  for ( ; it != flds.end(); ++it )
  {
    if ( it->type() == QVariant::Double || it->type() == QVariant::Int )
      cboGraduatedColumn->addItem( it->name() );
  }
}

void QgsGraduatedSymbolRendererV2Widget::graduatedColumnChanged()
{
  mRenderer->setClassAttribute( cboGraduatedColumn->currentText() );
}


void QgsGraduatedSymbolRendererV2Widget::classifyGraduated()
{
  QString attrName = cboGraduatedColumn->currentText();

  int classes = spinGraduatedClasses->value();

  QgsVectorColorRampV2* ramp = cboGraduatedColorRamp->currentColorRamp();

  if ( ramp == NULL )
  {
    if ( cboGraduatedColorRamp->count() == 0 )
      QMessageBox::critical( this, tr( "Error" ), tr( "There are no available color ramps. You can add them in Style Manager." ) );
    else
      QMessageBox::critical( this, tr( "Error" ), tr( "The selected color ramp is not available." ) );
    return;
  }

  QgsGraduatedSymbolRendererV2::Mode mode;
  if ( cboGraduatedMode->currentIndex() == 0 )
    mode = QgsGraduatedSymbolRendererV2::EqualInterval;
  else if ( cboGraduatedMode->currentIndex() == 2 )
    mode = QgsGraduatedSymbolRendererV2::Jenks;
  else if ( cboGraduatedMode->currentIndex() == 3 )
    mode = QgsGraduatedSymbolRendererV2::StdDev;
  else if ( cboGraduatedMode->currentIndex() == 4 )
    mode = QgsGraduatedSymbolRendererV2::Pretty;
  else // default should be quantile for now
    mode = QgsGraduatedSymbolRendererV2::Quantile;

  // create and set new renderer
  QgsGraduatedSymbolRendererV2* r = QgsGraduatedSymbolRendererV2::createRenderer(
                                      mLayer, attrName, classes, mode, mGraduatedSymbol, ramp );
  if ( !r )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Renderer creation has failed." ) );
    return;
  }

  delete mRenderer;
  mRenderer = r;

  populateRanges();
}

void QgsGraduatedSymbolRendererV2Widget::changeGraduatedSymbol()
{
  QgsSymbolV2SelectorDialog dlg( mGraduatedSymbol, mStyle, this );
  if ( !dlg.exec() )
    return;

  updateGraduatedSymbolIcon();
}

void QgsGraduatedSymbolRendererV2Widget::updateGraduatedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mGraduatedSymbol, btnChangeGraduatedSymbol->iconSize() );
  btnChangeGraduatedSymbol->setIcon( icon );
}


void QgsGraduatedSymbolRendererV2Widget::populateRanges()
{

  QStandardItemModel* m = qobject_cast<QStandardItemModel*>( viewGraduated->model() );
  m->clear();
  mRowSelected = -1;

  QStringList labels;
  labels << tr( "Symbol" ) << tr( "Range" ) << tr( "Label" );
  m->setHorizontalHeaderLabels( labels );

  QSize iconSize( 16, 16 );

  int i, count = mRenderer->ranges().count();

  for ( i = 0; i < count; i++ )
  {
    const QgsRendererRangeV2& range = mRenderer->ranges()[i];
    QString rangeStr = QString::number( range.lowerValue(), 'f', 4 ) + " - " + QString::number( range.upperValue(), 'f', 4 );

    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( range.symbol(), iconSize );
    QStandardItem* item = new QStandardItem( icon, "" );
    //item->setData(k); // set attribute value as user role
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    QStandardItem* item2 = new QStandardItem( rangeStr );
    item2->setEditable( 0 );

    QList<QStandardItem *> list;
    list << item << item2 << new QStandardItem( range.label() );

    m->appendRow( list );
  }

  // make sure that the "range" column has visible context
  viewGraduated->resizeColumnToContents( 0 );
}


#if 0
int QgsRendererV2PropertiesDialog::currentRangeRow()
{
  QModelIndex idx = viewGraduated->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}
#endif

void QgsGraduatedSymbolRendererV2Widget::rangesDoubleClicked( const QModelIndex & idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeRangeSymbol( idx.row() );
  if ( idx.isValid() && idx.column() == 1 )
    changeRange( idx.row() );
}

void QgsGraduatedSymbolRendererV2Widget::rangesClicked( const QModelIndex & idx )
{
  if ( !idx.isValid() )
    mRowSelected = -1;
  else
    mRowSelected = idx.row();
}

void QgsGraduatedSymbolRendererV2Widget::changeRangeSymbol( int rangeIdx )
{
  QgsSymbolV2* newSymbol = mRenderer->ranges()[rangeIdx].symbol()->clone();

  QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, this );
  if ( !dlg.exec() )
  {
    delete newSymbol;
    return;
  }

  mRenderer->updateRangeSymbol( rangeIdx, newSymbol );

  populateRanges();
}

void QgsGraduatedSymbolRendererV2Widget::changeRange( int rangeIdx )
{
  QgsLUDialog dialog( this );

  const QgsRendererRangeV2& range = mRenderer->ranges()[rangeIdx];
  dialog.setLowerValue( QString( "%1" ).arg( range.lowerValue() ) );
  dialog.setUpperValue( QString( "%1" ).arg( range.upperValue() ) );

  if ( dialog.exec() == QDialog::Accepted )
  {
    double lowerValue = dialog.lowerValue().toDouble();
    double upperValue = dialog.upperValue().toDouble();
    mRenderer->updateRangeUpperValue( rangeIdx, upperValue );
    mRenderer->updateRangeLowerValue( rangeIdx, lowerValue );
    populateRanges();

  }

}

void QgsGraduatedSymbolRendererV2Widget::addClass()
{
  mRenderer->addClass( mGraduatedSymbol );
  populateRanges();
}

void QgsGraduatedSymbolRendererV2Widget::deleteCurrentClass()
{

  mRenderer->deleteClass( mRowSelected );
  populateRanges();
}

void QgsGraduatedSymbolRendererV2Widget::changeCurrentValue( QStandardItem * item )
{
  if ( item->column() == 2 )
  {
    QString label = item->text();
    int idx = item->row();
    mRenderer->updateRangeLabel( idx, label );
  }
}

void QgsGraduatedSymbolRendererV2Widget::rotationFieldChanged( QString fldName )
{
  mRenderer->setRotationField( fldName );
}

void QgsGraduatedSymbolRendererV2Widget::sizeScaleFieldChanged( QString fldName )
{
  mRenderer->setSizeScaleField( fldName );
}
