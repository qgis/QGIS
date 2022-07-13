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
#include "qgscolorswatchgrid.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorwidgets.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsguiutils.h"
#include "qgsgui.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QClipboard>
#include <QDrag>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QWidgetAction>
#include <QScreen>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QBuffer>

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

  mMinimumSize.setHeight( std::max( static_cast<int>( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.1 ), mMinimumSize.height() ) );

  // If project colors change, we need to redraw the button, as it may be set to follow a project color
  connect( QgsProject::instance(), &QgsProject::projectColorsChanged, this, [ = ]
  {
    setButtonBackground();
  } );
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
    const QColor currentColor = color();
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
  const QgsSettings settings;

  // first check if we need to use the limited native dialogs
  const bool useNative = settings.value( QStringLiteral( "qgis/native_color_dialogs" ), false ).toBool();
  if ( useNative )
  {
    // why would anyone want this? who knows.... maybe the limited nature of native dialogs helps ease the transition for MapInfo users?
    newColor = QColorDialog::getColor( color(), this, mColorDialogTitle, mAllowOpacity ? QColorDialog::ShowAlphaChannel : ( QColorDialog::ColorDialogOption )0 );
  }
  else
  {
    QgsColorDialog dialog( this, Qt::WindowFlags(), color() );
    dialog.setTitle( mColorDialogTitle );
    dialog.setAllowOpacity( mAllowOpacity );

    if ( dialog.exec() )
    {
      newColor = dialog.color();
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
  emit cleared();
}

void QgsColorButton::unlink()
{
  linkToProjectColor( QString() );
  emit unlinked();
}

bool QgsColorButton::event( QEvent *e )
{
  if ( e->type() == QEvent::ToolTip )
  {
    QColor c = linkedProjectColor();
    const bool isProjectColor = c.isValid();
    if ( !isProjectColor )
      c = mColor;

    const QString name = c.name();
    const int hue = c.hue();
    const int value = c.value();
    const int saturation = c.saturation();

    // create very large preview swatch
    const int width = static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 23 );
    const int height = static_cast< int >( width / 1.61803398875 ); // golden ratio

    const int margin = static_cast< int >( height * 0.1 );
    QImage icon = QImage( width + 2 * margin, height + 2 * margin, QImage::Format_ARGB32 );
    icon.fill( Qt::transparent );

    QPainter p;
    p.begin( &icon );

    //start with checkboard pattern
    const QBrush checkBrush = QBrush( transparentBackground() );
    p.setPen( Qt::NoPen );
    p.setBrush( checkBrush );
    p.drawRect( margin, margin, width, height );

    //draw color over pattern
    p.setBrush( QBrush( c ) );

    //draw border
    p.setPen( QColor( 197, 197, 197 ) );
    p.drawRect( margin, margin, width, height );
    p.end();

    QByteArray data;
    QBuffer buffer( &data );
    icon.save( &buffer, "PNG", 100 );

    const QString info = ( isProjectColor ? QStringLiteral( "<p>%1: %2</p>" ).arg( tr( "Linked color" ), mLinkedColorName ) : QString() )
                         + QStringLiteral( "<b>HEX</b> %1<br>"
                                           "<b>RGB</b> %2<br>"
                                           "<b>HSV</b> %3,%4,%5<p>"
                                           "<img src='data:image/png;base64, %0'>" ).arg( QString( data.toBase64() ), name,
                                               QgsSymbolLayerUtils::encodeColor( c ) )
                         .arg( hue ).arg( saturation ).arg( value );
    setToolTip( info );
  }
  return QToolButton::event( e );
}

void QgsColorButton::setToNoColor()
{
  QColor noColor = QColor( mColor );
  noColor.setAlpha( 0 );
  setColor( noColor );
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
    setButtonBackground( QgsGui::sampleColor( e->globalPos() ) );
    e->accept();
    return;
  }

  //handle dragging colors from button
  QColor c = linkedProjectColor();
  if ( !c.isValid() )
    c = mColor;

  if ( !( e->buttons() & Qt::LeftButton ) || !c.isValid() )
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
  drag->setMimeData( QgsSymbolLayerUtils::colorToMimeData( c ) );
  drag->setPixmap( QgsColorWidget::createDragIcon( c ) );
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

void QgsColorButton::stopPicking( QPoint eventPos, bool samplingColor )
{
  //release mouse and keyboard, and reset cursor
  releaseMouse();
  releaseKeyboard();
  QgsApplication::restoreOverrideCursor();
  setMouseTracking( false );
  mPickingColor = false;

  if ( !samplingColor )
  {
    //not sampling color, restore old color
    setButtonBackground();
    return;
  }

  setColor( QgsGui::sampleColor( eventPos ) );
  addRecentColor( mColor );
}

QColor QgsColorButton::linkedProjectColor() const
{
  QList<QgsProjectColorScheme *> projectSchemes;
  QgsApplication::colorSchemeRegistry()->schemes( projectSchemes );
  if ( projectSchemes.length() > 0 )
  {
    QgsProjectColorScheme *scheme = projectSchemes.at( 0 );
    const QgsNamedColorList colors = scheme->fetchColors();
    for ( const auto &color : colors )
    {
      if ( color.second.isEmpty() )
        continue;

      if ( color.second == mLinkedColorName )
      {
        return color.first;
      }
    }
  }
  return QColor();
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
  const bool isProjectColor = linkedProjectColor().isValid();
  if ( isProjectColor )
    return;

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
  Q_UNUSED( e )
  //reset button color
  setButtonBackground();
}

void QgsColorButton::dropEvent( QDropEvent *e )
{
  const bool isProjectColor = linkedProjectColor().isValid();
  if ( isProjectColor )
    return;

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

void QgsColorButton::wheelEvent( QWheelEvent *event )
{
  if ( mAllowOpacity && isEnabled() && !isNull() )
  {
    const double increment = ( ( event->modifiers() & Qt::ControlModifier ) ? 0.01 : 0.1 ) *
                             ( event->angleDelta().y() > 0 ? 1 : -1 );
    const double alpha = std::min( std::max( 0.0, mColor.alphaF() + increment ), 1.0 );
    mColor.setAlphaF( alpha );

    setButtonBackground();
    emit colorChanged( mColor );
    event->accept();
  }
  else
  {
    QToolButton::wheelEvent( event );
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
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );

  //create an icon pixmap
  QPixmap pixmap( iconSize, iconSize );
  pixmap.fill( Qt::transparent );

  QPainter p;
  p.begin( &pixmap );

  //start with checkboard pattern
  if ( showChecks )
  {
    const QBrush checkBrush = QBrush( transparentBackground() );
    p.setPen( Qt::NoPen );
    p.setBrush( checkBrush );
    p.drawRect( 0, 0, iconSize - 1, iconSize - 1 );
  }

  //draw color over pattern
  p.setBrush( QBrush( color ) );

  //draw border
  p.setPen( QColor( 197, 197, 197 ) );
  p.drawRect( 0, 0, iconSize - 1, iconSize - 1 );
  p.end();
  return pixmap;
}

void QgsColorButton::buttonClicked()
{
  if ( linkedProjectColor().isValid() )
  {
    QToolButton::showMenu();
  }
  else
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
}

void QgsColorButton::prepareMenu()
{
  //we need to tear down and rebuild this menu every time it is shown. Otherwise the space allocated to any
  //QgsColorSwatchGridAction is not recalculated by Qt and the swatch grid may not be the correct size
  //for the number of colors shown in the grid. Note that we MUST refresh color swatch grids every time this
  //menu is opened, otherwise color schemes like the recent color scheme grid are meaningless
  mMenu->clear();

  const bool isProjectColor = linkedProjectColor().isValid();

  if ( !isProjectColor )
  {
    if ( mShowNull )
    {
      QAction *nullAction = new QAction( mNullColorString.isEmpty() ? tr( "Clear Color" ) : mNullColorString, this );
      nullAction->setIcon( createMenuIcon( Qt::transparent, false ) );
      mMenu->addAction( nullAction );
      connect( nullAction, &QAction::triggered, this, &QgsColorButton::setToNull );
    }

    //show default color option if set
    if ( mDefaultColor.isValid() )
    {
      QAction *defaultColorAction = new QAction( tr( "Default Color" ), this );
      defaultColorAction->setIcon( createMenuIcon( mDefaultColor ) );
      mMenu->addAction( defaultColorAction );
      connect( defaultColorAction, &QAction::triggered, this, &QgsColorButton::setToDefaultColor );
    }

    if ( mShowNoColorOption )
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
  }

  if ( isProjectColor )
  {
    QAction *unlinkAction = new QAction( tr( "Unlink Color" ), mMenu );
    mMenu->addAction( unlinkAction );
    connect( unlinkAction, &QAction::triggered, this, &QgsColorButton::unlink );
  }

  QAction *copyColorAction = new QAction( tr( "Copy Color" ), this );
  mMenu->addAction( copyColorAction );
  connect( copyColorAction, &QAction::triggered, this, &QgsColorButton::copyColor );

  if ( !isProjectColor )
  {
    QAction *pasteColorAction = new QAction( tr( "Paste Color" ), this );
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

    QAction *pickColorAction = new QAction( tr( "Pick Color" ), this );
    mMenu->addAction( pickColorAction );
    connect( pickColorAction, &QAction::triggered, this, &QgsColorButton::activatePicker );

    QAction *chooseColorAction = new QAction( tr( "Choose Color…" ), this );
    mMenu->addAction( chooseColorAction );
    connect( chooseColorAction, &QAction::triggered, this, &QgsColorButton::showColorDialog );
  }
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
  setButtonBackground();
}

void QgsColorButton::setColor( const QColor &color )
{
  const QColor oldColor = mColor;
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
  bool isProjectColor = false;
  if ( !backgroundColor.isValid() && !mLinkedColorName.isEmpty() )
  {
    backgroundColor = linkedProjectColor();
    isProjectColor = backgroundColor.isValid();
    if ( !isProjectColor )
    {
      mLinkedColorName.clear(); //color has been deleted, renamed, etc...
      emit unlinked();
    }
  }
  if ( !backgroundColor.isValid() )
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
  QPixmap pixmap( currentIconSize );
  pixmap.fill( Qt::transparent );

  if ( backgroundColor.isValid() )
  {
    const QRect rect( 0, 0, currentIconSize.width(), currentIconSize.height() );
    QPainter p;
    p.begin( &pixmap );
    p.setRenderHint( QPainter::Antialiasing );
    p.setPen( Qt::NoPen );
    if ( mAllowOpacity && backgroundColor.alpha() < 255 )
    {
      //start with checkboard pattern
      const QBrush checkBrush = QBrush( transparentBackground() );
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
  QColor c = linkedProjectColor();
  if ( !c.isValid() )
    c = mColor;
  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::colorToMimeData( c ) );
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
  //activate picker color
  // Store current color
  mCurrentColor = mColor;
  QApplication::setOverrideCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Sampler ) );
  grabMouse();
  grabKeyboard();
  mPickingColor = true;
  setMouseTracking( true );
}

QColor QgsColorButton::color() const
{
  QColor c = linkedProjectColor();
  if ( !c.isValid() )
    c = mColor;
  return c;
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
  mShowMenu = showMenu;
  setMenu( showMenu ? mMenu : nullptr );
  setPopupMode( showMenu ? QToolButton::MenuButtonPopup : QToolButton::DelayedPopup );
  //force recalculation of icon size
  mIconSize = QSize();
  setButtonBackground();
}

void QgsColorButton::setBehavior( const QgsColorButton::Behavior behavior )
{
  mBehavior = behavior;
}

void QgsColorButton::setDefaultColor( const QColor &color )
{
  mDefaultColor = color;
}

void QgsColorButton::setShowNull( bool showNull, const QString &nullString )
{
  mShowNull = showNull;
  mNullColorString = nullString;
}

bool QgsColorButton::showNull() const
{
  return mShowNull;
}

bool QgsColorButton::isNull() const
{
  return !mColor.isValid();
}

void QgsColorButton::linkToProjectColor( const QString &name )
{
  mLinkedColorName = name;
  setButtonBackground();
}

