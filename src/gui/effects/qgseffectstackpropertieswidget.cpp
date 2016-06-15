/***************************************************************************
    qgseffectstackpropertieswidget.h
    --------------------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseffectstackpropertieswidget.h"
#include "qgspainteffectregistry.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgspainteffectpropertieswidget.h"
#include "qgspainteffectwidget.h"
#include "qgsapplication.h"
#include "qgssymbollayerv2utils.h"
#include "qgspanelwidget.h"

#include <QPicture>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QCheckBox>
#include <QToolButton>

///@cond PRIVATE

static const int EffectItemType = QStandardItem::UserType + 1;

class EffectItem : public QStandardItem
{
  public:
    EffectItem( QgsPaintEffect* effect, QgsEffectStackPropertiesWidget* propertiesWidget )
    {
      setEffect( effect );
      setCheckable( true );
      mWidget = propertiesWidget;
    }

    void setEffect( QgsPaintEffect* effect )
    {
      mEffect = effect;
      emitDataChanged();
    }

    int type() const override { return EffectItemType; }

    QgsPaintEffect* effect()
    {
      return mEffect;
    }

    QVariant data( int role ) const override
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        return QgsPaintEffectRegistry::instance()->effectMetadata( mEffect->type() )->visibleName();
      }
      if ( role == Qt::CheckStateRole )
      {
        return mEffect->enabled() ? Qt::Checked : Qt::Unchecked;
      }
      return QStandardItem::data( role );
    }

    void setData( const QVariant & value, int role ) override
    {
      if ( role == Qt::CheckStateRole )
      {
        mEffect->setEnabled( value.toBool() );
        mWidget->updatePreview();
      }
      else
      {
        QStandardItem::setData( value, role );
      }
    }

  protected:
    QgsPaintEffect* mEffect;
    QgsEffectStackPropertiesWidget* mWidget;
};
///@endcond

//
// QgsEffectStackPropertiesWidget
//

QgsEffectStackPropertiesWidget::QgsEffectStackPropertiesWidget( QgsEffectStack *stack, QWidget *parent )
    : QgsPanelWidget( parent )
    , mStack( stack )
    , mPreviewPicture( nullptr )
{

// TODO
#ifdef Q_OS_MAC
  //setWindowModality( Qt::WindowModal );
#endif

  mPresentWidget = nullptr;

  setupUi( this );

  mAddButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mRemoveButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  mUpButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.svg" ) ) );
  mDownButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.svg" ) ) );

  mModel = new QStandardItemModel();
  // Set the effect
  mEffectsList->setModel( mModel );

  QItemSelectionModel* selModel = mEffectsList->selectionModel();
  connect( selModel, SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( effectChanged() ) );

  loadStack( stack );
  updatePreview();

  connect( mUpButton, SIGNAL( clicked() ), this, SLOT( moveEffectUp() ) );
  connect( mDownButton, SIGNAL( clicked() ), this, SLOT( moveEffectDown() ) );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addEffect() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), this, SLOT( removeEffect() ) );

  updateUi();

  // set effect as active item in the tree
  QModelIndex newIndex = mEffectsList->model()->index( 0, 0 );
  mEffectsList->setCurrentIndex( newIndex );

  setPanelTitle( tr( "Effects Properties" ) );
}

QgsEffectStackPropertiesWidget::~QgsEffectStackPropertiesWidget()
{
  delete mPreviewPicture;
}

void QgsEffectStackPropertiesWidget::setPreviewPicture( const QPicture &picture )
{
  if ( mPreviewPicture )
  {
    delete mPreviewPicture;
  }

  mPreviewPicture = new QPicture( picture );
  updatePreview();
}

void QgsEffectStackPropertiesWidget::loadStack( QgsEffectStack* stack )
{
  if ( !stack )
  {
    return;
  }

  EffectItem* parent = static_cast<EffectItem*>( mModel->invisibleRootItem() );

  int count = stack->count();
  for ( int i = count - 1; i >= 0; i-- )
  {
    EffectItem* effectItem = new EffectItem( stack->effect( i ), this );
    effectItem->setEditable( false );
    parent->appendRow( effectItem );
  }
}


void QgsEffectStackPropertiesWidget::loadStack()
{
  mModel->clear();
  loadStack( mStack );
}

void QgsEffectStackPropertiesWidget::updateUi()
{
  QModelIndex currentIdx = mEffectsList->currentIndex();
  if ( !currentIdx.isValid() )
    return;

  EffectItem *item = static_cast<EffectItem*>( mModel->itemFromIndex( currentIdx ) );

  QStandardItem* root = mModel->invisibleRootItem();
  int rowCount = root->rowCount();
  int currentRow = item ? item->row() : 0;

  mUpButton->setEnabled( currentRow > 0 );
  mDownButton->setEnabled( currentRow < rowCount - 1 );
  mRemoveButton->setEnabled( rowCount > 1 );
}

void QgsEffectStackPropertiesWidget::updatePreview()
{
  QPainter painter;
  QImage previewImage( 150, 150, QImage::Format_ARGB32 );
  previewImage.fill( Qt::transparent );
  painter.begin( &previewImage );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext context = QgsSymbolLayerV2Utils::createRenderContext( &painter );
  if ( !mPreviewPicture )
  {
    QPicture previewPic;
    QPainter previewPicPainter;
    previewPicPainter.begin( &previewPic );
    previewPicPainter.setPen( Qt::red );
    previewPicPainter.setBrush( QColor( 255, 100, 100, 255 ) );
    previewPicPainter.drawEllipse( QPoint( 75, 75 ), 30, 30 );
    previewPicPainter.end();
    mStack->render( previewPic, context );
  }
  else
  {
    context.painter()->translate( 35, 35 );
    mStack->render( *mPreviewPicture, context );
  }
  painter.end();

  lblPreview->setPixmap( QPixmap::fromImage( previewImage ) );
  emit widgetChanged();
}

EffectItem* QgsEffectStackPropertiesWidget::currentEffectItem()
{
  QModelIndex idx = mEffectsList->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  EffectItem *item = static_cast<EffectItem*>( mModel->itemFromIndex( idx ) );
  return item;
}

void QgsEffectStackPropertiesWidget::effectChanged()
{
  updateUi();

  EffectItem* currentItem = currentEffectItem();
  if ( !currentItem )
    return;

  QWidget *effectPropertiesWidget = new QgsPaintEffectPropertiesWidget( currentItem->effect() );
  setWidget( effectPropertiesWidget );

  connect( effectPropertiesWidget, SIGNAL( changeEffect( QgsPaintEffect* ) ), this, SLOT( changeEffect( QgsPaintEffect* ) ) );
  connect( effectPropertiesWidget, SIGNAL( changed() ), this, SLOT( updatePreview() ) );

}

void QgsEffectStackPropertiesWidget::setWidget( QWidget* widget )
{
  int index = stackedWidget->addWidget( widget );
  stackedWidget->setCurrentIndex( index );
  if ( mPresentWidget )
  {
    stackedWidget->removeWidget( mPresentWidget );
    QWidget *dummy = mPresentWidget;
    mPresentWidget = widget;
    delete dummy; // auto disconnects all signals
  }
}

void QgsEffectStackPropertiesWidget::addEffect()
{
  QgsPaintEffect* newEffect = new QgsDrawSourceEffect();
  mStack->insertEffect( 0, newEffect );

  EffectItem *newEffectItem = new EffectItem( newEffect, this );
  mModel->invisibleRootItem()->insertRow( mStack->count() - 1, newEffectItem );

  mEffectsList->setCurrentIndex( mModel->indexFromItem( newEffectItem ) );
  updateUi();
  updatePreview();
}

void QgsEffectStackPropertiesWidget::removeEffect()
{
  EffectItem *item = currentEffectItem();
  int row = item->row();
  QStandardItem* root = mModel->invisibleRootItem();

  int layerIdx = root->rowCount() - row - 1;
  QgsPaintEffect *tmpEffect = mStack->takeEffect( layerIdx );

  mModel->invisibleRootItem()->removeRow( row );

  int newSelection = qMin( row, root->rowCount() - 1 );
  QModelIndex newIdx = root->child( newSelection )->index();
  mEffectsList->setCurrentIndex( newIdx );

  updateUi();
  updatePreview();

  delete tmpEffect;
}

void QgsEffectStackPropertiesWidget::moveEffectDown()
{
  moveEffectByOffset( + 1 );
}

void QgsEffectStackPropertiesWidget::moveEffectUp()
{
  moveEffectByOffset( -1 );
}

void QgsEffectStackPropertiesWidget::moveEffectByOffset( int offset )
{
  EffectItem *item = currentEffectItem();
  if ( !item )
    return;

  int row = item->row();

  QStandardItem* root = mModel->invisibleRootItem();

  int layerIdx = root->rowCount() - row - 1;
  // switch effects
  QgsPaintEffect* tmpEffect = mStack->takeEffect( layerIdx );
  mStack->insertEffect( layerIdx - offset, tmpEffect );

  QList<QStandardItem *> toMove = root->takeRow( row );
  root->insertRows( row + offset, toMove );

  QModelIndex newIdx = toMove[ 0 ]->index();
  mEffectsList->setCurrentIndex( newIdx );

  updatePreview();
  updateUi();
}

void QgsEffectStackPropertiesWidget::changeEffect( QgsPaintEffect* newEffect )
{
  EffectItem *item = currentEffectItem();
  item->setEffect( newEffect );

  QStandardItem* root = mModel->invisibleRootItem();
  int effectIdx = root->rowCount() - item->row() - 1;
  mStack->changeEffect( effectIdx, newEffect );

  updatePreview();
  // Important: This lets the effect to have its own effect properties widget
  effectChanged();
}


//
// QgsEffectStackPropertiesDialog
//

QgsEffectStackPropertiesDialog::QgsEffectStackPropertiesDialog( QgsEffectStack *stack, QWidget *parent, const Qt::WindowFlags& f )
    : QgsDialog( parent, f, QDialogButtonBox::Ok | QDialogButtonBox::Cancel )
    , mPropertiesWidget( nullptr )
{
  setWindowTitle( tr( "Effect Properties" ) );
  mPropertiesWidget = new QgsEffectStackPropertiesWidget( stack, this );
  layout()->addWidget( mPropertiesWidget );
}

QgsEffectStackPropertiesDialog::~QgsEffectStackPropertiesDialog()
{

}

QgsEffectStack* QgsEffectStackPropertiesDialog::stack()
{
  return mPropertiesWidget->stack();
}

void QgsEffectStackPropertiesDialog::setPreviewPicture( const QPicture &picture )
{
  mPropertiesWidget->setPreviewPicture( picture );
}

//
// QgsEffectStackCompactWidget
//

QgsEffectStackCompactWidget::QgsEffectStackCompactWidget( QWidget *parent , QgsPaintEffect *effect )
    : QgsPanelWidget( parent )
    , mEnabledCheckBox( nullptr )
    , mButton( nullptr )
    , mPreviewPicture( nullptr )
{
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  setLayout( layout );

  mEnabledCheckBox = new QCheckBox( this );
  mEnabledCheckBox->setText( tr( "Draw effects" ) );
  layout->addWidget( mEnabledCheckBox );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( "mIconPaintEffects.svg" ) );
  mButton->setToolTip( tr( "Customise effects" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mEnabledCheckBox );

  connect( mButton, SIGNAL( clicked() ), this, SLOT( showDialog() ) );
  connect( mEnabledCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( enableToggled( bool ) ) );

  setPaintEffect( effect );
}

QgsEffectStackCompactWidget::~QgsEffectStackCompactWidget()
{
  delete mPreviewPicture;
}

void QgsEffectStackCompactWidget::setPaintEffect( QgsPaintEffect *effect )
{
  if ( !effect )
  {
    mEnabledCheckBox->setChecked( false );
    mEnabledCheckBox->setEnabled( false );
    mButton->setEnabled( false );
    mStack = nullptr;
    return;
  }

  //is effect a stack?
  QgsEffectStack* stack = dynamic_cast<QgsEffectStack*>( effect );
  if ( !stack )
  {
    //not already a stack, so promote to stack
    stack = new QgsEffectStack( *effect );
  }

  mStack = stack;
  mEnabledCheckBox->setChecked( mStack->enabled() );
  mEnabledCheckBox->setEnabled( true );
  mButton->setEnabled( mStack->enabled() );
}

void QgsEffectStackCompactWidget::setPreviewPicture( const QPicture &picture )
{
  delete mPreviewPicture;
  mPreviewPicture = new QPicture( picture );
}

void QgsEffectStackCompactWidget::showDialog()
{
  if ( !mStack )
    return;

  QgsEffectStack* clone = static_cast<QgsEffectStack*>( mStack->clone() );
  QgsEffectStackPropertiesWidget* widget = new QgsEffectStackPropertiesWidget( clone, nullptr );
  if ( mPreviewPicture )
  {
    widget->setPreviewPicture( *mPreviewPicture );
  }
  connect( widget, SIGNAL( widgetChanged() ), this, SLOT( updateEffectLive() ) );
  connect( widget, SIGNAL( panelAccepted( QgsPanelWidget* ) ), this, SLOT( updateAcceptWidget( QgsPanelWidget* ) ) );
  openPanel( widget );
}

void QgsEffectStackCompactWidget::enableToggled( bool checked )
{
  if ( !mStack )
  {
    return;
  }

  mStack->setEnabled( checked );
  mButton->setEnabled( checked );
  emit changed();
}

void QgsEffectStackCompactWidget::updateAcceptWidget( QgsPanelWidget *panel )
{
  QgsEffectStackPropertiesWidget* widget = qobject_cast<QgsEffectStackPropertiesWidget*>( panel );
  *mStack = *widget->stack();
  emit changed();
//    delete widget->stack();
}

void QgsEffectStackCompactWidget::updateEffectLive()
{
  QgsEffectStackPropertiesWidget* widget = qobject_cast<QgsEffectStackPropertiesWidget*>( sender() );
  *mStack = *widget->stack();
  emit changed();
}
