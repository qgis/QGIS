/***************************************************************************
    qgscolorbutton.cpp - Button which displays a color
     --------------------------------------
    Date                 : 12-Dec-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorbutton.h"
#include "qgscolordialog.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssymbollayerutils.h"
#include "qgscursors.h"
#include "qgscolorswatchgrid.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorwidgets.h"
#include "qgssettings.h"

#include <QPainter>
#include <QTemporaryFile>
#include <QMouseEvent>
#include <QMenu>
#include <QClipboard>
#include <QDrag>
#include <QDesktopWidget>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QWidgetAction>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

QgsColorButton::QgsColorButton( QWidget *parent, const QString &cdt, QgsColorSchemeRegistry *registry )
  : QToolButton( parent )
  , mColorDialogTitle( cdt.isEmpty() ? tr( "Select Color" ) : cdt )
  , mNoColorString( tr( "No color" ) )
{
  //if a color scheme registry was specified, use it, otherwise use the global instance
  mColorSchemeRegistry = registry ? registry : QgsApplication::colorSchemeRegistry();

  setAcceptDrops( true );
  setMinimumSize( QSize( 24, 16 ) );
  connect( this, &QAbstractButton::clicked, this, &QgsColorButton::buttonClicked );

  //setup drop-down menu
  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsColorButton::prepareMenu );
  setMenu( mMenu );
  setPopupMode( QToolButton::MenuButtonPopup );

#ifdef Q_OS_WIN
  mMinimumSize = QSize( 120, 22 );
#else
  mMinimumSize = QSize( 120, 28 );
#endif

  mMinimumSize.setHeight( std::max( static_cast<int>( fontMetrics().height() * 1.1 ), mMinimumSize.height() ) );
}



QSize QgsColorButton::minimumSizeHint() const
{
  return mMinimumSize;
}

QSize QgsColorButton::sizeHint() const
{
  return mMinimumSize;
}

const QPixmap &QgsColorButton::transparentBackground()
{
  static QPixmap sTranspBkgrd;

  if ( sTranspBkgrd.isNull() )
    sTranspBkgrd = QgsApplication::getThemePixmap( QStringLiteral( "/transp-background_8x8.png" ) );

  return sTranspBkgrd;
}

void QgsColorButton::showColorDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QColor currentColor = color();
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, currentColor, QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( mColorDialogTitle );
    colorWidget->setAllowOpacity( mAllowOpacity );

    if ( currentColor.isValid() )
    {
      colorWidget->setPreviousColor( currentColor );
    }

    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, &QgsColorButton::setValidTemporaryColor );
    panel->openPanel( colorWidget );
    return;
  }

  QColor newColor;
  QgsSettings settings;

  if ( mAcceptLiveUpdates && settings.value( QStringLiteral( "qgis/live_color_dialogs" ), false ).toBool() )
  {
    // live updating dialog - QgsColorDialog will automatically use native dialog if option is set
    newColor = QgsColorDialog::getLiveColor(
                 color(), this, SLOT( setValidColor( const QColor & ) ),
                 this, mColorDialogTitle, mAllowOpacity );
  }
  else
  {
    // not using live updating dialog - first check if we need to use the limited native dialogs
    bool useNative = settings.value( QStringLiteral( "qgis/native_color_dialogs" ), false ).toBool();
    if ( useNative )
    {
      // why would anyone want this? who knows.... maybe the limited nature of native dialogs helps ease the transition for MapInfo users?
      newColor = QColorDialog::getColor( color(), this, mColorDialogTitle, mAllowOpacity ? QColorDialog::ShowAlphaChannel : ( QColorDialog::ColorDialogOption )0 );
    }
    else
    {
      QgsColorDialog dialog( this, 0, color() );
      dialog.setTitle( mColorDialogTitle );
      dialog.setAllowOpacity( mAllowOpacity );

      if ( dialog.exec() )
      {
        newColor = dialog.color();
      }
    }
  }

  if ( newColor.isValid() )
  {
    setValidColor( newColor );
  }

  // reactivate button's window
  activateWindow();
}

void QgsColorButton::setToDefaultColor()
{
  if ( !mDefaultColor.isValid() )
  {
    return;
  }

  setColor( mDefaultColor );
}

void QgsColorButton::setToNull()
{
  setColor( QColor() );
}

bool QgsColorButton::event( QEvent *e )
{
  if ( e->type() == QEvent::ToolTip )
  {
    QString name = this->color().name();
    int hue = this->color().hue();
    int value = this->color().value();
    int saturation = this->color().saturation();
    QString info = QString( "HEX: %1 \n"
                            "RGB: %2 \n"
                            "HSV: %3,%4,%5" ).arg( name,
                                QgsSymbolLayerUtils::encodeColor( this->color() ) )
                   .arg( hue ).arg( saturation ).arg( value );
    setToolTip( info );
  }
  return QToolButton::event( e );
}

void QgsColorButton::setToNoColor()
{
  if ( mAllowOpacity )
  {
    QColor noColor = QColor( mColor );
    noColor.setAlpha( 0 );
    setColor( noColor );
  }
}

void QgsColorButton::mousePressEvent( QMouseEvent *e )
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

bool QgsColorButton::colorFromMimeData( const QMimeData *mimeData, QColor &resultColor )
{
  bool hasAlpha = false;
  QColor mimeColor = QgsSymbolLayerUtils::colorFromMimeData( mimeData, hasAlpha );

  if ( mimeColor.isValid() )
  {
    if ( !mAllowOpacity )
    {
      //remove alpha channel
      mimeColor.setAlpha( 255 );
    }
    else if ( !hasAlpha )
    {
      //mime color has no explicit alpha component, so keep existing alpha
      mimeColor.setAlpha( mColor.alpha() );
    }
    resultColor = mimeColor;
    return true;
  }

  //could not get color from mime data
  return false;
}

void QgsColorButton::mouseMoveEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //currently in color picker mode
    if ( e->buttons() & Qt::LeftButton )
    {
      //if left button depressed, sample color under cursor and temporarily update button color
      //to give feedback to user
      QPixmap snappedPixmap = QPixmap::grabWindow( QApplication::desktop()->winId(), e->globalPos().x(), e->globalPos().y(), 1, 1 );
      QImage snappedImage = snappedPixmap.toImage();
      QColor hoverColor = snappedImage.pixel( 0, 0 );
      setButtonBackground( hoverColor );
    }
    e->accept();
    return;
  }

  //handle dragging colors from button

  if ( !( e->buttons() & Qt::LeftButton ) || !mColor.isValid() )
  {
    //left button not depressed or no color set, so not a drag
    QToolButton::mouseMoveEvent( e );
    return;
  }

  if ( ( e->pos() - mDragStartPosition ).manhattanLength() < QApplication::startDragDistance() )
  {
    //mouse not moved, so not a drag
    QToolButton::mouseMoveEvent( e );
    return;
  }

  //user is dragging color
  QDrag *drag = new QDrag( this );
  drag->setMimeData( QgsSymbolLayerUtils::colorToMimeData( mColor ) );
  drag->setPixmap( QgsColorWidget::createDragIcon( mColor ) );
  drag->exec( Qt::CopyAction );
  setDown( false );
}

void QgsColorButton::mouseReleaseEvent( QMouseEvent *e )
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

void QgsColorButton::stopPicking( QPointF eventPos, bool sampleColor )
{
  //release mouse and keyboard, and reset cursor
  releaseMouse();
  releaseKeyboard();
  unsetCursor();
  mPickingColor = false;

  if ( !sampleColor )
  {
    //not sampling color, nothing more to do
    return;
  }

  //grab snapshot of pixel under mouse cursor
  QPixmap snappedPixmap = QPixmap::grabWindow( QApplication::desktop()->winId(), eventPos.x(), eventPos.y(), 1, 1 );
  QImage snappedImage = snappedPixmap.toImage();
  //extract color from pixel and set color
  setColor( snappedImage.pixel( 0, 0 ) );
  addRecentColor( mColor );
}

void QgsColorButton::keyPressEvent( QKeyEvent *e )
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

void QgsColorButton::dragEnterEvent( QDragEnterEvent *e )
{
  //is dragged data valid color data?
  QColor mimeColor;
  if ( colorFromMimeData( e->mimeData(), mimeColor ) )
  {
    //if so, we accept the drag, and temporarily change the button's color
    //to match the dragged color. This gives immediate feedback to the user
    //that colors can be dropped here
    e->acceptProposedAction();
    setButtonBackground( mimeColor );
  }
}

void QgsColorButton::dragLeaveEvent( QDragLeaveEvent *e )
{
  Q_UNUSED( e );
  //reset button color
  setButtonBackground( mColor );
}

void QgsColorButton::dropEvent( QDropEvent *e )
{
  //is dropped data valid color data?
  QColor mimeColor;
  if ( colorFromMimeData( e->mimeData(), mimeColor ) )
  {
    //accept drop and set new color
    e->acceptProposedAction();
    setColor( mimeColor );
    addRecentColor( mimeColor );
  }
}

void QgsColorButton::setValidColor( const QColor &newColor )
{
  if ( newColor.isValid() )
  {
    setColor( newColor );
    addRecentColor( newColor );
  }
}

void QgsColorButton::setValidTemporaryColor( const QColor &newColor )
{
  if ( newColor.isValid() )
  {
    setColor( newColor );
  }
}

QPixmap QgsColorButton::createMenuIcon( const QColor &color, const bool showChecks )
{
  //create an icon pixmap
  QPixmap pixmap( 16, 16 );
  pixmap.fill( Qt::transparent );

  QPainter p;
  p.begin( &pixmap );

  //start with checkboard pattern
  if ( showChecks )
  {
    QBrush checkBrush = QBrush( transparentBackground() );
    p.setPen( Qt::NoPen );
    p.setBrush( checkBrush );
    p.drawRect( 0, 0, 15, 15 );
  }

  //draw color over pattern
  p.setBrush( QBrush( color ) );

  //draw border
  p.setPen( QColor( 197, 197, 197 ) );
  p.drawRect( 0, 0, 15, 15 );
  p.end();
  return pixmap;
}

void QgsColorButton::buttonClicked()
{
  switch ( mBehavior )
  {
    case ShowDialog:
      showColorDialog();
      return;
    case SignalOnly:
      emit colorClicked( mColor );
      return;
  }
}

void QgsColorButton::prepareMenu()
{
  //we need to tear down and rebuild this menu every time it is shown. Otherwise the space allocated to any
  //QgsColorSwatchGridAction is not recalculated by Qt and the swatch grid may not be the correct size
  //for the number of colors shown in the grid. Note that we MUST refresh color swatch grids every time this
  //menu is opened, otherwise color schemes like the recent color scheme grid are meaningless
  mMenu->clear();

  if ( mShowNull )
  {
    QAction *nullAction = new QAction( tr( "Clear color" ), this );
    nullAction->setIcon( createMenuIcon( Qt::transparent, false ) );
    mMenu->addAction( nullAction );
    connect( nullAction, &QAction::triggered, this, &QgsColorButton::setToNull );
  }

  //show default color option if set
  if ( mDefaultColor.isValid() )
  {
    QAction *defaultColorAction = new QAction( tr( "Default color" ), this );
    defaultColorAction->setIcon( createMenuIcon( mDefaultColor ) );
    mMenu->addAction( defaultColorAction );
    connect( defaultColorAction, &QAction::triggered, this, &QgsColorButton::setToDefaultColor );
  }

  if ( mShowNoColorOption && mAllowOpacity )
  {
    QAction *noColorAction = new QAction( mNoColorString, this );
    noColorAction->setIcon( createMenuIcon( Qt::transparent, false ) );
    mMenu->addAction( noColorAction );
    connect( noColorAction, &QAction::triggered, this, &QgsColorButton::setToNoColor );
  }

  mMenu->addSeparator();
  QgsColorWheel *colorWheel = new QgsColorWheel( mMenu );
  colorWheel->setColor( color() );
  QgsColorWidgetAction *colorAction = new QgsColorWidgetAction( colorWheel, mMenu, mMenu );
  colorAction->setDismissOnColorSelection( false );
  connect( colorAction, &QgsColorWidgetAction::colorChanged, this, &QgsColorButton::setColor );
  mMenu->addAction( colorAction );
  if ( mAllowOpacity )
  {
    QgsColorRampWidget *alphaRamp = new QgsColorRampWidget( mMenu, QgsColorWidget::Alpha, QgsColorRampWidget::Horizontal );
    alphaRamp->setColor( color() );
    QgsColorWidgetAction *alphaAction = new QgsColorWidgetAction( alphaRamp, mMenu, mMenu );
    alphaAction->setDismissOnColorSelection( false );
    connect( alphaAction, &QgsColorWidgetAction::colorChanged, this, &QgsColorButton::setColor );
    connect( alphaAction, &QgsColorWidgetAction::colorChanged, colorWheel, [colorWheel]( const QColor & color ) { colorWheel->setColor( color, false ); }
           );
    connect( colorAction, &QgsColorWidgetAction::colorChanged, alphaRamp, [alphaRamp]( const QColor & color ) { alphaRamp->setColor( color, false ); }
           );
    mMenu->addAction( alphaAction );
  }

  if ( mColorSchemeRegistry )
  {
    //get schemes with ShowInColorButtonMenu flag set
    QList< QgsColorScheme * > schemeList = mColorSchemeRegistry->schemes( QgsColorScheme::ShowInColorButtonMenu );
    QList< QgsColorScheme * >::iterator it = schemeList.begin();
    for ( ; it != schemeList.end(); ++it )
    {
      QgsColorSwatchGridAction *colorAction = new QgsColorSwatchGridAction( *it, mMenu, mContext, this );
      colorAction->setBaseColor( mColor );
      mMenu->addAction( colorAction );
      connect( colorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsColorButton::setValidColor );
      connect( colorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsColorButton::addRecentColor );
    }
  }

  mMenu->addSeparator();

  QAction *copyColorAction = new QAction( tr( "Copy color" ), this );
  mMenu->addAction( copyColorAction );
  connect( copyColorAction, &QAction::triggered, this, &QgsColorButton::copyColor );

  QAction *pasteColorAction = new QAction( tr( "Paste color" ), this );
  //enable or disable paste action based on current clipboard contents. We always show the paste
  //action, even if it's disabled, to give hint to the user that pasting colors is possible
  QColor clipColor;
  if ( colorFromMimeData( QApplication::clipboard()->mimeData(), clipColor ) )
  {
    pasteColorAction->setIcon( createMenuIcon( clipColor ) );
  }
  else
  {
    pasteColorAction->setEnabled( false );
  }
  mMenu->addAction( pasteColorAction );
  connect( pasteColorAction, &QAction::triggered, this, &QgsColorButton::pasteColor );

  //disabled for OSX, as it is impossible to grab the mouse under OSX
  //see note for QWidget::grabMouse() re OSX Cocoa
  //http://qt-project.org/doc/qt-4.8/qwidget.html#grabMouse
  QAction *pickColorAction = new QAction( tr( "Pick color" ), this );
  mMenu->addAction( pickColorAction );
  connect( pickColorAction, &QAction::triggered, this, &QgsColorButton::activatePicker );

  QAction *chooseColorAction = new QAction( tr( "Choose color..." ), this );
  mMenu->addAction( chooseColorAction );
  connect( chooseColorAction, &QAction::triggered, this, &QgsColorButton::showColorDialog );
}

void QgsColorButton::changeEvent( QEvent *e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    setButtonBackground();
  }
  QToolButton::changeEvent( e );
}

#if 0 // causes too many cyclical updates, but may be needed on some platforms
void QgsColorButton::paintEvent( QPaintEvent *e )
{
  QToolButton::paintEvent( e );

  if ( !mBackgroundSet )
  {
    setButtonBackground();
  }
}
#endif

void QgsColorButton::showEvent( QShowEvent *e )
{
  setButtonBackground();
  QToolButton::showEvent( e );
}

void QgsColorButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  //recalculate icon size and redraw icon
  mIconSize = QSize();
  setButtonBackground( mColor );
}

void QgsColorButton::setColor( const QColor &color )
{
  QColor oldColor = mColor;
  mColor = color;

  // handle when initially set color is same as default (Qt::black); consider it a color change
  if ( oldColor != mColor || ( mColor == QColor( Qt::black ) && !mColorSet ) )
  {
    setButtonBackground();
    if ( isEnabled() )
    {
      // TODO: May be beneficial to have the option to set color without emitting this signal.
      //       Now done by blockSignals( bool ) where button is used
      emit colorChanged( mColor );
    }
  }
  mColorSet = true;
}

void QgsColorButton::addRecentColor( const QColor &color )
{
  QgsRecentColorScheme::addRecentColor( color );
}

void QgsColorButton::setButtonBackground( const QColor &color )
{
  QColor backgroundColor = color;

  if ( !color.isValid() )
  {
    backgroundColor = mColor;
  }

  QSize currentIconSize;
  //icon size is button size with a small margin
  if ( menu() )
  {
    if ( !mIconSize.isValid() )
    {
      //calculate size of push button part of widget (ie, without the menu drop-down button part)
      QStyleOptionToolButton opt;
      initStyleOption( &opt );
      QRect buttonSize = QApplication::style()->subControlRect( QStyle::CC_ToolButton, &opt, QStyle::SC_ToolButton,
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
  QPixmap pixmap( currentIconSize );
  pixmap.fill( Qt::transparent );

  if ( backgroundColor.isValid() )
  {
    QRect rect( 0, 0, currentIconSize.width(), currentIconSize.height() );
    QPainter p;
    p.begin( &pixmap );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    if ( mAllowOpacity && backgroundColor.alpha() < 255 )
    {
      //start with checkboard pattern
      QBrush checkBrush = QBrush( transparentBackground() );
      p.setBrush( checkBrush );
      p.drawRoundedRect( rect, 3, 3 );
    }

    //draw semi-transparent color on top
    p.setBrush( backgroundColor );
    p.drawRoundedRect( rect, 3, 3 );
    p.end();
  }

  setIconSize( currentIconSize );
  setIcon( pixmap );
}

void QgsColorButton::copyColor()
{
  //copy color
  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::colorToMimeData( mColor ) );
}

void QgsColorButton::pasteColor()
{
  QColor clipColor;
  if ( colorFromMimeData( QApplication::clipboard()->mimeData(), clipColor ) )
  {
    //paste color
    setColor( clipColor );
    addRecentColor( clipColor );
  }
}

void QgsColorButton::activatePicker()
{
  //pick color
  QPixmap samplerPixmap = QPixmap( ( const char ** ) sampler_cursor );
  setCursor( QCursor( samplerPixmap, 0, 0 ) );
  grabMouse();
  grabKeyboard();
  mPickingColor = true;
}

QColor QgsColorButton::color() const
{
  return mColor;
}

void QgsColorButton::setAllowOpacity( const bool allow )
{
  mAllowOpacity = allow;
}

void QgsColorButton::setColorDialogTitle( const QString &title )
{
  mColorDialogTitle = title;
}

QString QgsColorButton::colorDialogTitle() const
{
  return mColorDialogTitle;
}

void QgsColorButton::setShowMenu( const bool showMenu )
{
  setMenu( showMenu ? mMenu : nullptr );
  setPopupMode( showMenu ? QToolButton::MenuButtonPopup : QToolButton::DelayedPopup );
  //force recalculation of icon size
  mIconSize = QSize();
  setButtonBackground( mColor );
}

void QgsColorButton::setBehavior( const QgsColorButton::Behavior behavior )
{
  mBehavior = behavior;
}

void QgsColorButton::setDefaultColor( const QColor &color )
{
  mDefaultColor = color;
}

void QgsColorButton::setShowNull( bool showNull )
{
  mShowNull = showNull;
}

bool QgsColorButton::showNull() const
{
  return mShowNull;
}

bool QgsColorButton::isNull() const
{
  return !mColor.isValid();
}

