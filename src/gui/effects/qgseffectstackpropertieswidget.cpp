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
#include "qgssymbollayerutils.h"
#include "qgspanelwidget.h"
#include "qgshelp.h"

#include <QPicture>
#include <QPainter>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QCheckBox>
#include <QToolButton>

///@cond PRIVATE

static const int EFFECT_ITEM_TYPE = QStandardItem::UserType + 1;

class EffectItem : public QStandardItem
{
  public:
    EffectItem( QgsPaintEffect *effect, QgsEffectStackPropertiesWidget *propertiesWidget )
    {
      setEffect( effect );
      setCheckable( true );
      mWidget = propertiesWidget;
    }

    void setEffect( QgsPaintEffect *effect )
    {
      mEffect = effect;
      emitDataChanged();
    }

    int type() const override { return EFFECT_ITEM_TYPE; }

    QgsPaintEffect *effect()
    {
      return mEffect;
    }

    QVariant data( int role ) const override
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        return QgsApplication::paintEffectRegistry()->effectMetadata( mEffect->type() )->visibleName();
      }
      if ( role == Qt::CheckStateRole )
      {
        return mEffect->enabled() ? Qt::Checked : Qt::Unchecked;
      }
      return QStandardItem::data( role );
    }

    void setData( const QVariant &value, int role ) override
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
    QgsPaintEffect *mEffect = nullptr;
    QgsEffectStackPropertiesWidget *mWidget = nullptr;
};
///@endcond

//
// QgsEffectStackPropertiesWidget
//

QgsEffectStackPropertiesWidget::QgsEffectStackPropertiesWidget( QgsEffectStack *stack, QWidget *parent )
  : QgsPanelWidget( parent )
  , mStack( stack )

{

// TODO
#ifdef Q_OS_MAC
  //setWindowModality( Qt::WindowModal );
#endif

  mPresentWidget = nullptr;

  setupUi( this );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  mEffectsList->setMaximumHeight( static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 7 ) );
  mEffectsList->setMinimumHeight( mEffectsList->maximumHeight() );
  lblPreview->setMaximumWidth( mEffectsList->maximumHeight() );

  mAddButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mRemoveButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  mUpButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowUp.svg" ) ) );
  mDownButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowDown.svg" ) ) );

  mModel = new QStandardItemModel();
  // Set the effect
  mEffectsList->setModel( mModel );

  QItemSelectionModel *selModel = mEffectsList->selectionModel();
  connect( selModel, &QItemSelectionModel::currentChanged, this, &QgsEffectStackPropertiesWidget::effectChanged );

  loadStack( stack );
  updatePreview();

  connect( mUpButton, &QAbstractButton::clicked, this, &QgsEffectStackPropertiesWidget::moveEffectUp );
  connect( mDownButton, &QAbstractButton::clicked, this, &QgsEffectStackPropertiesWidget::moveEffectDown );
  connect( mAddButton, &QAbstractButton::clicked, this, &QgsEffectStackPropertiesWidget::addEffect );
  connect( mRemoveButton, &QAbstractButton::clicked, this, &QgsEffectStackPropertiesWidget::removeEffect );

  updateUi();

  // set first selected effect as active item in the tree
  int initialRow = 0;
  for ( int i = 0; i < stack->count(); ++i )
  {
    // list shows effects in opposite order to stack
    if ( stack->effect( stack->count() - i - 1 )->enabled() )
    {
      initialRow = i;
      break;
    }
  }
  const QModelIndex newIndex = mEffectsList->model()->index( initialRow, 0 );
  mEffectsList->setCurrentIndex( newIndex );

  setPanelTitle( tr( "Effects Properties" ) );
}

QgsEffectStackPropertiesWidget::~QgsEffectStackPropertiesWidget() = default;

void QgsEffectStackPropertiesWidget::setPreviewPicture( const QPicture &picture )
{
  mPreviewPicture = picture;
  updatePreview();
}

void QgsEffectStackPropertiesWidget::loadStack( QgsEffectStack *stack )
{
  if ( !stack )
  {
    return;
  }

  EffectItem *parent = static_cast<EffectItem *>( mModel->invisibleRootItem() );

  const int count = stack->count();
  for ( int i = count - 1; i >= 0; i-- )
  {
    EffectItem *effectItem = new EffectItem( stack->effect( i ), this );
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
  const QModelIndex currentIdx = mEffectsList->currentIndex();
  if ( !currentIdx.isValid() )
    return;

  EffectItem *item = static_cast<EffectItem *>( mModel->itemFromIndex( currentIdx ) );

  QStandardItem *root = mModel->invisibleRootItem();
  const int rowCount = root->rowCount();
  const int currentRow = item ? item->row() : 0;

  mUpButton->setEnabled( currentRow > 0 );
  mDownButton->setEnabled( currentRow < rowCount - 1 );
  mRemoveButton->setEnabled( rowCount > 1 );
}

void QgsEffectStackPropertiesWidget::updatePreview()
{
  QPainter painter;
  QImage previewImage( 100, 100, QImage::Format_ARGB32 );
  previewImage.fill( Qt::transparent );
  painter.begin( &previewImage );
  painter.setRenderHint( QPainter::Antialiasing );
  QgsRenderContext context = QgsRenderContext::fromQPainter( &painter );
  context.setFlag( Qgis::RenderContextFlag::RenderSymbolPreview, true );
  if ( mPreviewPicture.isNull() )
  {
    QPicture previewPic;
    QPainter previewPicPainter;
    previewPicPainter.begin( &previewPic );
    previewPicPainter.setPen( Qt::red );
    previewPicPainter.setBrush( QColor( 255, 100, 100, 255 ) );
    previewPicPainter.drawEllipse( QPoint( 50, 50 ), 20, 20 );
    previewPicPainter.end();
    mStack->render( previewPic, context );
  }
  else
  {
    context.painter()->translate( 20, 20 );
    mStack->render( mPreviewPicture, context );
  }
  painter.end();

  lblPreview->setPixmap( QPixmap::fromImage( previewImage ) );
  emit widgetChanged();
}

EffectItem *QgsEffectStackPropertiesWidget::currentEffectItem()
{
  const QModelIndex idx = mEffectsList->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  EffectItem *item = static_cast<EffectItem *>( mModel->itemFromIndex( idx ) );
  return item;
}

void QgsEffectStackPropertiesWidget::effectChanged()
{
  updateUi();

  EffectItem *currentItem = currentEffectItem();
  if ( !currentItem )
    return;

  QgsPaintEffectPropertiesWidget *effectPropertiesWidget = new QgsPaintEffectPropertiesWidget( currentItem->effect() );
  setWidget( effectPropertiesWidget );

  connect( effectPropertiesWidget, &QgsPaintEffectPropertiesWidget::changeEffect, this, &QgsEffectStackPropertiesWidget::changeEffect );
  connect( effectPropertiesWidget, &QgsPaintEffectPropertiesWidget::changed, this, &QgsEffectStackPropertiesWidget::updatePreview );
}

void QgsEffectStackPropertiesWidget::setWidget( QWidget *widget )
{
  const int index = stackedWidget->addWidget( widget );
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
  QgsPaintEffect *newEffect = new QgsDrawSourceEffect();
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
  const int row = item->row();
  QStandardItem *root = mModel->invisibleRootItem();

  const int layerIdx = root->rowCount() - row - 1;
  QgsPaintEffect *tmpEffect = mStack->takeEffect( layerIdx );

  mModel->invisibleRootItem()->removeRow( row );

  const int newSelection = std::min( row, root->rowCount() - 1 );
  const QModelIndex newIdx = root->child( newSelection )->index();
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

  const int row = item->row();

  QStandardItem *root = mModel->invisibleRootItem();

  const int layerIdx = root->rowCount() - row - 1;
  // switch effects
  QgsPaintEffect *tmpEffect = mStack->takeEffect( layerIdx );
  mStack->insertEffect( layerIdx - offset, tmpEffect );

  QList<QStandardItem *> toMove = root->takeRow( row );
  root->insertRows( row + offset, toMove );

  const QModelIndex newIdx = toMove[ 0 ]->index();
  mEffectsList->setCurrentIndex( newIdx );

  updatePreview();
  updateUi();
}

void QgsEffectStackPropertiesWidget::changeEffect( QgsPaintEffect *newEffect )
{
  EffectItem *item = currentEffectItem();
  item->setEffect( newEffect );

  QStandardItem *root = mModel->invisibleRootItem();
  const int effectIdx = root->rowCount() - item->row() - 1;
  mStack->changeEffect( effectIdx, newEffect );

  updatePreview();
  // Important: This lets the effect to have its own effect properties widget
  effectChanged();
}


//
// QgsEffectStackPropertiesDialog
//

QgsEffectStackPropertiesDialog::QgsEffectStackPropertiesDialog( QgsEffectStack *stack, QWidget *parent, Qt::WindowFlags f )
  : QgsDialog( parent, f, QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok )

{
  setWindowTitle( tr( "Effect Properties" ) );
  mPropertiesWidget = new QgsEffectStackPropertiesWidget( stack, this );

  QDialogButtonBox *buttonBox = this->findChild<QDialogButtonBox *>( QString(), Qt::FindDirectChildrenOnly );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsEffectStackPropertiesDialog::showHelp );

  layout()->addWidget( mPropertiesWidget );
}

QgsEffectStack *QgsEffectStackPropertiesDialog::stack()
{
  return mPropertiesWidget->stack();
}

void QgsEffectStackPropertiesDialog::setPreviewPicture( const QPicture &picture )
{
  mPropertiesWidget->setPreviewPicture( picture );
}

void QgsEffectStackPropertiesDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#draw-effects" ) );
}


//
// QgsEffectStackCompactWidget
//

QgsEffectStackCompactWidget::QgsEffectStackCompactWidget( QWidget *parent, QgsPaintEffect *effect )
  : QgsPanelWidget( parent )

{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 6 );
  setLayout( layout );

  mEnabledCheckBox = new QCheckBox( this );
  mEnabledCheckBox->setText( tr( "Draw effects" ) );
  layout->addWidget( mEnabledCheckBox );

  mButton = new QToolButton( this );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconPaintEffects.svg" ) ) );
  mButton->setToolTip( tr( "Customize effects" ) );
  layout->addWidget( mButton );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mEnabledCheckBox );

  connect( mButton, &QAbstractButton::clicked, this, &QgsEffectStackCompactWidget::showDialog );
  connect( mEnabledCheckBox, &QAbstractButton::toggled, this, &QgsEffectStackCompactWidget::enableToggled );

  setPaintEffect( effect );
}

QgsEffectStackCompactWidget::~QgsEffectStackCompactWidget() = default;

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
  QgsEffectStack *stack = dynamic_cast<QgsEffectStack *>( effect );
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

QgsPaintEffect *QgsEffectStackCompactWidget::paintEffect() const
{
  return mStack;
}

void QgsEffectStackCompactWidget::setPreviewPicture( const QPicture &picture )
{
  mPreviewPicture = picture;
}

void QgsEffectStackCompactWidget::showDialog()
{
  if ( !mStack )
    return;

  QgsEffectStack *clone = mStack->clone();
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsEffectStackPropertiesWidget *widget = new QgsEffectStackPropertiesWidget( clone, nullptr );
    widget->setPreviewPicture( mPreviewPicture );

    connect( widget, &QgsPanelWidget::widgetChanged, this, &QgsEffectStackCompactWidget::updateEffectLive );
    connect( widget, &QgsPanelWidget::panelAccepted, this, &QgsEffectStackCompactWidget::updateAcceptWidget );
    panel->openPanel( widget );
  }
  else
  {
    QgsEffectStackPropertiesDialog dlg( clone, this );
    dlg.setPreviewPicture( mPreviewPicture );

    if ( dlg.exec() == QDialog::Accepted )
    {
      *mStack = *clone;
      emit changed();
    }
  }
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
  QgsEffectStackPropertiesWidget *widget = qobject_cast<QgsEffectStackPropertiesWidget *>( panel );
  *mStack = *widget->stack();
  emit changed();
//    delete widget->stack();
}

void QgsEffectStackCompactWidget::updateEffectLive()
{
  QgsEffectStackPropertiesWidget *widget = qobject_cast<QgsEffectStackPropertiesWidget *>( sender() );
  *mStack = *widget->stack();
  emit changed();
}
