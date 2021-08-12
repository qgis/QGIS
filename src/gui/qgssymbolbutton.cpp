/***************************************************************************
     qgssymbolbutton.h
     -----------------
    Date                 : July 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolbutton.h"
#include "qgspanelwidget.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsvectorlayer.h"
#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgscolorwidgets.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorswatchgrid.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgscolordialog.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

#include <QMenu>
#include <QClipboard>
#include <QDrag>
#include <QBuffer>

QgsSymbolButton::QgsSymbolButton( QWidget *parent, const QString &dialogTitle )
  : QToolButton( parent )
  , mDialogTitle( dialogTitle.isEmpty() ? tr( "Symbol Settings" ) : dialogTitle )
{
  mSymbol.reset( QgsFillSymbol::createSimple( QVariantMap() ) );

  setAcceptDrops( true );
  connect( this, &QAbstractButton::clicked, this, &QgsSymbolButton::showSettingsDialog );

  //setup dropdown menu
  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsSymbolButton::prepareMenu );
  setMenu( mMenu );
  setPopupMode( QToolButton::MenuButtonPopup );

  //make sure height of button looks good under different platforms
  const QSize size = QToolButton::minimumSizeHint();
  const int fontHeight = static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.4 );
  mSizeHint = QSize( size.width(), std::max( size.height(), fontHeight ) );
}

QgsSymbolButton::~QgsSymbolButton() = default;

QSize QgsSymbolButton::minimumSizeHint() const
{

  return mSizeHint;
}

QSize QgsSymbolButton::sizeHint() const
{
  return mSizeHint;
}

void QgsSymbolButton::setSymbolType( Qgis::SymbolType type )
{
  if ( type != mType )
  {
    switch ( type )
    {
      case Qgis::SymbolType::Marker:
        mSymbol.reset( QgsMarkerSymbol::createSimple( QVariantMap() ) );
        break;

      case Qgis::SymbolType::Line:
        mSymbol.reset( QgsLineSymbol::createSimple( QVariantMap() ) );
        break;

      case Qgis::SymbolType::Fill:
        mSymbol.reset( QgsFillSymbol::createSimple( QVariantMap() ) );
        break;

      case Qgis::SymbolType::Hybrid:
        break;
    }
  }
  updatePreview();
  mType = type;
}

void QgsSymbolButton::showSettingsDialog()
{
  QgsExpressionContext context;
  if ( mExpressionContextGenerator )
    context  = mExpressionContextGenerator->createExpressionContext();
  else
  {
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer.data() ) );
  }
  QgsSymbol *newSymbol = mSymbol->clone();

  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  symbolContext.setMapCanvas( mMapCanvas );
  symbolContext.setMessageBar( mMessageBar );

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSymbolSelectorWidget *d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), mLayer, panel );
    d->setPanelTitle( mDialogTitle );
    d->setContext( symbolContext );
    connect( d, &QgsPanelWidget::widgetChanged, this, &QgsSymbolButton::updateSymbolFromWidget );
    connect( d, &QgsPanelWidget::panelAccepted, this, &QgsSymbolButton::cleanUpSymbolSelector );
    panel->openPanel( d );
  }
  else
  {
    QgsSymbolSelectorDialog dialog( newSymbol, QgsStyle::defaultStyle(), mLayer, this );
    dialog.setWindowTitle( mDialogTitle );
    dialog.setContext( symbolContext );
    if ( dialog.exec() )
    {
      setSymbol( newSymbol );
    }
    else
    {
      delete newSymbol;
    }

    // reactivate button's window
    activateWindow();
  }
}

void QgsSymbolButton::updateSymbolFromWidget()
{
  if ( QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( sender() ) )
    setSymbol( w->symbol()->clone() );
}

void QgsSymbolButton::cleanUpSymbolSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !w )
    return;

  delete w->symbol();
}

QgsMapCanvas *QgsSymbolButton::mapCanvas() const
{
  return mMapCanvas;
}

void QgsSymbolButton::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  mMapCanvas = mapCanvas;
}

void QgsSymbolButton::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsSymbolButton::messageBar() const
{
  return mMessageBar;
}

QgsVectorLayer *QgsSymbolButton::layer() const
{
  return mLayer;
}

void QgsSymbolButton::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsSymbolButton::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

void QgsSymbolButton::setSymbol( QgsSymbol *symbol )
{
  mSymbol.reset( symbol );
  updatePreview();
  emit changed();
}

void QgsSymbolButton::setColor( const QColor &color )
{
  QColor opaque = color;
  opaque.setAlphaF( 1.0 );

  if ( opaque == mSymbol->color() )
    return;

  mSymbol->setColor( opaque );
  updatePreview();
  emit changed();
}

void QgsSymbolButton::copySymbol()
{
  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( mSymbol.get() ) );
}

void QgsSymbolButton::pasteSymbol()
{
  std::unique_ptr< QgsSymbol > symbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( symbol && symbol->type() == mType )
    setSymbol( symbol.release() );
}

void QgsSymbolButton::copyColor()
{
  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::colorToMimeData( mSymbol->color() ) );
}

void QgsSymbolButton::pasteColor()
{
  QColor clipColor;
  bool hasAlpha = false;
  if ( colorFromMimeData( QApplication::clipboard()->mimeData(), clipColor, hasAlpha ) )
  {
    //paste color
    setColor( clipColor );
    QgsRecentColorScheme::addRecentColor( clipColor );
  }
}

void QgsSymbolButton::mousePressEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //don't show dialog if in color picker mode
    e->accept();
    return;
  }

  if ( e->button() == Qt::RightButton )
  {
    QToolButton::showMenu();
    return;
  }
  else if ( e->button() == Qt::LeftButton )
  {
    mDragStartPosition = e->pos();
  }
  QToolButton::mousePressEvent( e );
}

void QgsSymbolButton::mouseMoveEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    updatePreview( QgsGui::sampleColor( e->globalPos() ) );
    e->accept();
    return;
  }

  //handle dragging colors/symbols from button

  if ( !( e->buttons() & Qt::LeftButton ) )
  {
    //left button not depressed, so not a drag
    QToolButton::mouseMoveEvent( e );
    return;
  }

  if ( ( e->pos() - mDragStartPosition ).manhattanLength() < QApplication::startDragDistance() )
  {
    //mouse not moved, so not a drag
    QToolButton::mouseMoveEvent( e );
    return;
  }

  //user is dragging
  QDrag *drag = new QDrag( this );
  drag->setMimeData( QgsSymbolLayerUtils::colorToMimeData( mSymbol->color() ) );
  drag->setPixmap( QgsColorWidget::createDragIcon( mSymbol->color() ) );
  drag->exec( Qt::CopyAction );
  setDown( false );
}

void QgsSymbolButton::mouseReleaseEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //end color picking operation by sampling the color under cursor
    stopPicking( e->globalPos() );
    e->accept();
    return;
  }

  QToolButton::mouseReleaseEvent( e );
}

void QgsSymbolButton::keyPressEvent( QKeyEvent *e )
{
  if ( !mPickingColor )
  {
    //if not picking a color, use default tool button behavior
    QToolButton::keyPressEvent( e );
    return;
  }

  //cancel picking, sampling the color if space was pressed
  stopPicking( QCursor::pos(), e->key() == Qt::Key_Space );
}

void QgsSymbolButton::dragEnterEvent( QDragEnterEvent *e )
{
  //is dragged data valid color data?
  QColor mimeColor;
  bool hasAlpha = false;

  if ( colorFromMimeData( e->mimeData(), mimeColor, hasAlpha ) )
  {
    //if so, we accept the drag, and temporarily change the button's color
    //to match the dragged color. This gives immediate feedback to the user
    //that colors can be dropped here
    e->acceptProposedAction();
    updatePreview( mimeColor );
  }
}

void QgsSymbolButton::dragLeaveEvent( QDragLeaveEvent *e )
{
  Q_UNUSED( e )
  //reset button color
  updatePreview();
}

void QgsSymbolButton::dropEvent( QDropEvent *e )
{
  //is dropped data valid format data?
  QColor mimeColor;
  bool hasAlpha = false;
  if ( colorFromMimeData( e->mimeData(), mimeColor, hasAlpha ) )
  {
    //accept drop and set new color
    e->acceptProposedAction();
    mimeColor.setAlphaF( 1.0 );
    mSymbol->setColor( mimeColor );
    QgsRecentColorScheme::addRecentColor( mimeColor );
    updatePreview();
    emit changed();
  }
  updatePreview();
}

void QgsSymbolButton::prepareMenu()
{
  //we need to tear down and rebuild this menu every time it is shown. Otherwise the space allocated to any
  //QgsColorSwatchGridAction is not recalculated by Qt and the swatch grid may not be the correct size
  //for the number of colors shown in the grid. Note that we MUST refresh color swatch grids every time this
  //menu is opened, otherwise color schemes like the recent color scheme grid are meaningless
  mMenu->clear();

  QAction *configureAction = new QAction( tr( "Configure Symbol…" ), this );
  mMenu->addAction( configureAction );
  connect( configureAction, &QAction::triggered, this, &QgsSymbolButton::showSettingsDialog );

  QAction *copySymbolAction = new QAction( tr( "Copy Symbol" ), this );
  mMenu->addAction( copySymbolAction );
  connect( copySymbolAction, &QAction::triggered, this, &QgsSymbolButton::copySymbol );
  QAction *pasteSymbolAction = new QAction( tr( "Paste Symbol" ), this );
  //enable or disable paste action based on current clipboard contents. We always show the paste
  //action, even if it's disabled, to give hint to the user that pasting symbols is possible
  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( tempSymbol && tempSymbol->type() == mType )
  {
    const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
    pasteSymbolAction->setIcon( QgsSymbolLayerUtils::symbolPreviewIcon( tempSymbol.get(), QSize( iconSize, iconSize ), 1 ) );
  }
  else
  {
    pasteSymbolAction->setEnabled( false );
  }
  mMenu->addAction( pasteSymbolAction );
  connect( pasteSymbolAction, &QAction::triggered, this, &QgsSymbolButton::pasteSymbol );

  mMenu->addSeparator();

  QgsColorWheel *colorWheel = new QgsColorWheel( mMenu );
  colorWheel->setColor( mSymbol->color() );
  QgsColorWidgetAction *colorAction = new QgsColorWidgetAction( colorWheel, mMenu, mMenu );
  colorAction->setDismissOnColorSelection( false );
  connect( colorAction, &QgsColorWidgetAction::colorChanged, this, &QgsSymbolButton::setColor );
  mMenu->addAction( colorAction );

  QgsColorRampWidget *alphaRamp = new QgsColorRampWidget( mMenu, QgsColorWidget::Alpha, QgsColorRampWidget::Horizontal );
  QColor alphaColor = mSymbol->color();
  alphaColor.setAlphaF( mSymbol->opacity() );
  alphaRamp->setColor( alphaColor );
  QgsColorWidgetAction *alphaAction = new QgsColorWidgetAction( alphaRamp, mMenu, mMenu );
  alphaAction->setDismissOnColorSelection( false );
  connect( alphaAction, &QgsColorWidgetAction::colorChanged, this, [ = ]( const QColor & color )
  {
    const double opacity = color.alphaF();
    mSymbol->setOpacity( opacity );
    updatePreview();
    emit changed();
  } );
  connect( colorAction, &QgsColorWidgetAction::colorChanged, alphaRamp, [alphaRamp]( const QColor & color ) { alphaRamp->setColor( color, false ); }
         );
  mMenu->addAction( alphaAction );

  //get schemes with ShowInColorButtonMenu flag set
  QList< QgsColorScheme * > schemeList = QgsApplication::colorSchemeRegistry()->schemes( QgsColorScheme::ShowInColorButtonMenu );
  QList< QgsColorScheme * >::iterator it = schemeList.begin();
  for ( ; it != schemeList.end(); ++it )
  {
    QgsColorSwatchGridAction *colorAction = new QgsColorSwatchGridAction( *it, mMenu, QStringLiteral( "symbology" ), this );
    colorAction->setBaseColor( mSymbol->color() );
    mMenu->addAction( colorAction );
    connect( colorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsSymbolButton::setColor );
    connect( colorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsSymbolButton::addRecentColor );
  }

  mMenu->addSeparator();

  QAction *copyColorAction = new QAction( tr( "Copy Color" ), this );
  mMenu->addAction( copyColorAction );
  connect( copyColorAction, &QAction::triggered, this, &QgsSymbolButton::copyColor );

  QAction *pasteColorAction = new QAction( tr( "Paste Color" ), this );
  //enable or disable paste action based on current clipboard contents. We always show the paste
  //action, even if it's disabled, to give hint to the user that pasting colors is possible
  QColor clipColor;
  bool hasAlpha = false;
  if ( colorFromMimeData( QApplication::clipboard()->mimeData(), clipColor, hasAlpha ) )
  {
    pasteColorAction->setIcon( createColorIcon( clipColor ) );
  }
  else
  {
    pasteColorAction->setEnabled( false );
  }
  mMenu->addAction( pasteColorAction );
  connect( pasteColorAction, &QAction::triggered, this, &QgsSymbolButton::pasteColor );

  QAction *pickColorAction = new QAction( tr( "Pick Color" ), this );
  mMenu->addAction( pickColorAction );
  connect( pickColorAction, &QAction::triggered, this, &QgsSymbolButton::activatePicker );

  QAction *chooseColorAction = new QAction( tr( "Choose Color…" ), this );
  mMenu->addAction( chooseColorAction );
  connect( chooseColorAction, &QAction::triggered, this, &QgsSymbolButton::showColorDialog );
}

void QgsSymbolButton::addRecentColor( const QColor &color )
{
  QgsRecentColorScheme::addRecentColor( color );
}

void QgsSymbolButton::activatePicker()
{
  //activate picker color
  QApplication::setOverrideCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Sampler ) );
  grabMouse();
  grabKeyboard();
  mPickingColor = true;
  setMouseTracking( true );
}


void QgsSymbolButton::changeEvent( QEvent *e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    updatePreview();
  }
  QToolButton::changeEvent( e );
}

void QgsSymbolButton::showEvent( QShowEvent *e )
{
  updatePreview();
  QToolButton::showEvent( e );
}

void QgsSymbolButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  //recalculate icon size and redraw icon
  mIconSize = QSize();
  updatePreview();
}

void QgsSymbolButton::updatePreview( const QColor &color, QgsSymbol *tempSymbol )
{
  std::unique_ptr< QgsSymbol > previewSymbol;

  if ( tempSymbol )
    previewSymbol.reset( tempSymbol->clone() );
  else
    previewSymbol.reset( mSymbol->clone() );

  if ( color.isValid() )
    previewSymbol->setColor( color );

  QSize currentIconSize;
  //icon size is button size with a small margin
  if ( menu() )
  {
    if ( !mIconSize.isValid() )
    {
      //calculate size of push button part of widget (ie, without the menu dropdown button part)
      QStyleOptionToolButton opt;
      initStyleOption( &opt );
      const QRect buttonSize = QApplication::style()->subControlRect( QStyle::CC_ToolButton, &opt, QStyle::SC_ToolButton,
                               this );
      //make sure height of icon looks good under different platforms
#ifdef Q_OS_WIN
      mIconSize = QSize( buttonSize.width() - 10, height() - 6 );
#else
      mIconSize = QSize( buttonSize.width() - 10, height() - 12 );
#endif
    }
    currentIconSize = mIconSize;
  }
  else
  {
    //no menu
#ifdef Q_OS_WIN
    currentIconSize = QSize( width() - 10, height() - 6 );
#else
    currentIconSize = QSize( width() - 10, height() - 12 );
#endif
  }

  if ( !currentIconSize.isValid() || currentIconSize.width() <= 0 || currentIconSize.height() <= 0 )
  {
    return;
  }

  //create an icon pixmap
  const QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( previewSymbol.get(), currentIconSize );
  setIconSize( currentIconSize );
  setIcon( icon );

  // set tooltip
  // create very large preview image

  const int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 23 );
  const int height = static_cast< int >( width / 1.61803398875 ); // golden ratio

  const QPixmap pm = QgsSymbolLayerUtils::symbolPreviewPixmap( previewSymbol.get(), QSize( width, height ), height / 20 );
  QByteArray data;
  QBuffer buffer( &data );
  pm.save( &buffer, "PNG", 100 );
  setToolTip( QStringLiteral( "<img src='data:image/png;base64, %3'>" ).arg( QString( data.toBase64() ) ) );
}

bool QgsSymbolButton::colorFromMimeData( const QMimeData *mimeData, QColor &resultColor, bool &hasAlpha )
{
  hasAlpha = false;
  const QColor mimeColor = QgsSymbolLayerUtils::colorFromMimeData( mimeData, hasAlpha );

  if ( mimeColor.isValid() )
  {
    resultColor = mimeColor;
    return true;
  }

  //could not get color from mime data
  return false;
}

QPixmap QgsSymbolButton::createColorIcon( const QColor &color ) const
{
  //create an icon pixmap
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  QPixmap pixmap( iconSize, iconSize );
  pixmap.fill( Qt::transparent );

  QPainter p;
  p.begin( &pixmap );

  //draw color over pattern
  p.setBrush( QBrush( color ) );

  //draw border
  p.setPen( QColor( 197, 197, 197 ) );
  p.drawRect( 0, 0, iconSize - 1, iconSize - 1 );
  p.end();
  return pixmap;
}

void QgsSymbolButton::stopPicking( QPoint eventPos, bool samplingColor )
{
  //release mouse and keyboard, and reset cursor
  releaseMouse();
  releaseKeyboard();
  QgsApplication::restoreOverrideCursor();
  setMouseTracking( false );
  mPickingColor = false;

  if ( !samplingColor )
  {
    //not sampling color, restore old icon
    updatePreview();
    return;
  }

  const QColor newColor = QgsGui::sampleColor( eventPos );
  setColor( newColor );
  addRecentColor( newColor );
}

void QgsSymbolButton::showColorDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    const QColor currentColor = mSymbol->color();
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, currentColor, QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Symbol Color" ) );
    colorWidget->setAllowOpacity( true );

    if ( currentColor.isValid() )
    {
      colorWidget->setPreviousColor( currentColor );
    }

    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, [ = ]( const QColor & newColor )
    {
      if ( newColor.isValid() )
      {
        setColor( newColor );
      }
    } );
    panel->openPanel( colorWidget );
    return;
  }

  QgsColorDialog dialog( this, Qt::WindowFlags(), mSymbol->color() );
  dialog.setTitle( tr( "Symbol Color" ) );
  dialog.setAllowOpacity( true );

  if ( dialog.exec() && dialog.color().isValid() )
  {
    setColor( dialog.color() );
  }

  // reactivate button's window
  activateWindow();
}

void QgsSymbolButton::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

QString QgsSymbolButton::dialogTitle() const
{
  return mDialogTitle;
}

QgsSymbol *QgsSymbolButton::symbol()
{
  return mSymbol.get();
}
