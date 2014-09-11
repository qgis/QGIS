/***************************************************************************
    qgscolorbuttonv2.cpp - Button which displays a color
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

#include "qgscolorbuttonv2.h"
#include "qgscolordialog.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssymbollayerv2utils.h"
#include "qgscursors.h"
#include "qgscolorswatchgrid.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorwidgets.h"

#include <QPainter>
#include <QSettings>
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

QgsColorButtonV2::QgsColorButtonV2( QWidget *parent, QString cdt, QColorDialog::ColorDialogOptions cdo, QgsColorSchemeRegistry* registry )
    : QToolButton( parent )
    , mColorDialogTitle( cdt.isEmpty() ? tr( "Select Color" ) : cdt )
    , mColor( Qt::black )
    , mDefaultColor( QColor() ) //default to invalid color
    , mColorDialogOptions( cdo )
    , mAcceptLiveUpdates( true )
    , mColorSet( false )
    , mShowNoColorOption( false )
    , mNoColorString( tr( "No color" ) )
    , mPickingColor( false )
    , mMenu( 0 )

{
  //if a color scheme registry was specified, use it, otherwise use the global instance
  mColorSchemeRegistry = registry ? registry : QgsColorSchemeRegistry::instance();

  setAcceptDrops( true );
  setMinimumSize( QSize( 24, 16 ) );
  connect( this, SIGNAL( clicked() ), this, SLOT( showColorDialog() ) );

  //setup dropdown menu
  mMenu = new QMenu( this );
  connect( mMenu, SIGNAL( aboutToShow() ), this, SLOT( prepareMenu() ) );
  setMenu( mMenu );
  setPopupMode( QToolButton::MenuButtonPopup );
}

QgsColorButtonV2::~QgsColorButtonV2()
{
}

QSize QgsColorButtonV2::sizeHint() const
{
  //make sure height of button looks good under different platforms
#ifdef Q_WS_WIN
  return QSize( 120, 22 );
#else
  return QSize( 120, 28 );
#endif
}

const QPixmap& QgsColorButtonV2::transparentBackground()
{
  static QPixmap transpBkgrd;

  if ( transpBkgrd.isNull() )
    transpBkgrd = QgsApplication::getThemePixmap( "/transp-background_8x8.png" );

  return transpBkgrd;
}

void QgsColorButtonV2::showColorDialog()
{
  QColor newColor;
  QSettings settings;
  if ( mAcceptLiveUpdates && settings.value( "/qgis/live_color_dialogs", false ).toBool() )
  {
    newColor = QgsColorDialog::getLiveColor(
                 color(), this, SLOT( setValidColor( const QColor& ) ),
                 this->parentWidget(), mColorDialogTitle, mColorDialogOptions );
  }
  else
  {
    newColor = QColorDialog::getColor( color(), this->parentWidget(), mColorDialogTitle, mColorDialogOptions );
  }
  setValidColor( newColor );

  // reactivate button's window
  activateWindow();
}

void QgsColorButtonV2::setToDefaultColor()
{
  if ( !mDefaultColor.isValid() )
  {
    return;
  }

  setColor( mDefaultColor );
}

void QgsColorButtonV2::setToNoColor()
{
  if ( mColorDialogOptions & QColorDialog::ShowAlphaChannel )
  {
    setColor( QColor( 0, 0, 0, 0 ) );
  }
}

void QgsColorButtonV2::mousePressEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //don't show dialog if in color picker mode
    e->accept();
    return;
  }

  if ( e->button() == Qt::LeftButton )
  {
    mDragStartPosition = e->pos();
  }
  QToolButton::mousePressEvent( e );
}

bool QgsColorButtonV2::colorFromMimeData( const QMimeData * mimeData, QColor& resultColor )
{
  bool hasAlpha = false;
  QColor mimeColor = QgsSymbolLayerV2Utils::colorFromMimeData( mimeData, hasAlpha );

  if ( mimeColor.isValid() )
  {
    if ( !( mColorDialogOptions & QColorDialog::ShowAlphaChannel ) )
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

void QgsColorButtonV2::mouseMoveEvent( QMouseEvent *e )
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

  if ( !( e->buttons() & Qt::LeftButton ) )
  {
    //left button not depressed, so not a drag
    QToolButton::mouseMoveEvent( e );
    return;
  }

  if (( e->pos() - mDragStartPosition ).manhattanLength() < QApplication::startDragDistance() )
  {
    //mouse not moved, so not a drag
    QToolButton::mouseMoveEvent( e );
    return;
  }

  //user is dragging color
  QDrag *drag = new QDrag( this );
  drag->setMimeData( QgsSymbolLayerV2Utils::colorToMimeData( mColor ) );
  drag->setPixmap( QgsColorWidget::createDragIcon( mColor ) );
  drag->exec( Qt::CopyAction );
  setDown( false );
}

void QgsColorButtonV2::mouseReleaseEvent( QMouseEvent *e )
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

void QgsColorButtonV2::stopPicking( QPointF eventPos, bool sampleColor )
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

void QgsColorButtonV2::keyPressEvent( QKeyEvent *e )
{
  if ( !mPickingColor )
  {
    //if not picking a color, use default tool button behaviour
    QToolButton::keyPressEvent( e );
    return;
  }

  //cancel picking, sampling the color if space was pressed
  stopPicking( QCursor::pos(), e->key() == Qt::Key_Space );
}

void QgsColorButtonV2::dragEnterEvent( QDragEnterEvent *e )
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

void QgsColorButtonV2::dragLeaveEvent( QDragLeaveEvent *e )
{
  Q_UNUSED( e );
  //reset button color
  setButtonBackground( mColor );
}

void QgsColorButtonV2::dropEvent( QDropEvent *e )
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

void QgsColorButtonV2::setValidColor( const QColor& newColor )
{
  if ( newColor.isValid() )
  {
    setColor( newColor );
    addRecentColor( newColor );
  }
}

QPixmap QgsColorButtonV2::createMenuIcon( const QColor color, const bool showChecks )
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

void QgsColorButtonV2::prepareMenu()
{
  //we need to tear down and rebuild this menu every time it is shown. Otherwise the space allocated to any
  //QgsColorSwatchGridAction is not recalculated by Qt and the swatch grid may not be the correct size
  //for the number of colors shown in the grid. Note that we MUST refresh color swatch grids every time this
  //menu is opened, otherwise color schemes like the recent color scheme grid are meaningless
  mMenu->clear();

  //show default color option if set
  if ( mDefaultColor.isValid() )
  {
    QAction* defaultColorAction = new QAction( tr( "Default color" ), this );
    defaultColorAction->setIcon( createMenuIcon( mDefaultColor ) );
    mMenu->addAction( defaultColorAction );
    connect( defaultColorAction, SIGNAL( triggered() ), this, SLOT( setToDefaultColor() ) );
  }

  if ( mShowNoColorOption && mColorDialogOptions & QColorDialog::ShowAlphaChannel )
  {
    QAction* noColorAction = new QAction( mNoColorString, this );
    noColorAction->setIcon( createMenuIcon( Qt::transparent, false ) );
    mMenu->addAction( noColorAction );
    connect( noColorAction, SIGNAL( triggered() ), this, SLOT( setToNoColor() ) );
  }

  if ( mColorSchemeRegistry )
  {
    QList< QgsColorScheme* > schemeList = mColorSchemeRegistry->schemes();
    QList< QgsColorScheme* >::iterator it = schemeList.begin();
    for ( ; it != schemeList.end(); ++it )
    {
      QgsColorSwatchGridAction* colorAction = new QgsColorSwatchGridAction( *it, mMenu, mContext, this );
      colorAction->setBaseColor( mColor );
      mMenu->addAction( colorAction );
      connect( colorAction, SIGNAL( colorChanged( QColor ) ), this, SLOT( setValidColor( QColor ) ) );
    }
  }

  mMenu->addSeparator();

  QAction* copyColorAction = new QAction( tr( "Copy color" ), this );
  mMenu->addAction( copyColorAction );
  connect( copyColorAction, SIGNAL( triggered() ), this, SLOT( copyColor() ) );

  QAction* pasteColorAction = new QAction( tr( "Paste color" ), this );
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
  connect( pasteColorAction, SIGNAL( triggered() ), this, SLOT( pasteColor() ) );

#ifndef Q_WS_MAC
  //disabled for OSX, as it is impossible to grab the mouse under OSX
  //see note for QWidget::grabMouse() re OSX Cocoa
  //http://qt-project.org/doc/qt-4.8/qwidget.html#grabMouse
  QAction* pickColorAction = new QAction( tr( "Pick color" ), this );
  mMenu->addAction( pickColorAction );
  connect( pickColorAction, SIGNAL( triggered() ), this, SLOT( activatePicker() ) );
#endif
  QAction* chooseColorAction = new QAction( tr( "Choose color..." ), this );
  mMenu->addAction( chooseColorAction );
  connect( chooseColorAction, SIGNAL( triggered() ), this, SLOT( showColorDialog() ) );
}

void QgsColorButtonV2::changeEvent( QEvent* e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    setButtonBackground();
  }
  QToolButton::changeEvent( e );
}

#if 0 // causes too many cyclical updates, but may be needed on some platforms
void QgsColorButtonV2::paintEvent( QPaintEvent* e )
{
  QToolButton::paintEvent( e );

  if ( !mBackgroundSet )
  {
    setButtonBackground();
  }
}
#endif

void QgsColorButtonV2::showEvent( QShowEvent* e )
{
  setButtonBackground();
  QToolButton::showEvent( e );
}

void QgsColorButtonV2::setColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    return;
  }

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

void QgsColorButtonV2::addRecentColor( const QColor color )
{
  //strip alpha from color
  QColor opaqueColor = color;
  opaqueColor.setAlpha( 255 );

  QSettings settings;
  QList< QVariant > recentColorVariants = settings.value( QString( "/colors/recent" ) ).toList();
  QVariant colorVariant = QVariant( opaqueColor );
  recentColorVariants.removeAll( colorVariant );

  recentColorVariants.prepend( colorVariant );

  while ( recentColorVariants.count() > 20 )
  {
    recentColorVariants.pop_back();
  }

  settings.setValue( QString( "/colors/recent" ), recentColorVariants );
}

void QgsColorButtonV2::setButtonBackground( const QColor color )
{
  QColor backgroundColor = color;

  if ( !color.isValid() )
  {
    backgroundColor = mColor;
  }

  bool useAlpha = ( mColorDialogOptions & QColorDialog::ShowAlphaChannel );

  //icon size is button size with a small margin
  if ( !mIconSize.isValid() )
  {
    //calculate size of push button part of widget (ie, without the menu dropdown button part)
    QStyleOptionToolButton opt;
    initStyleOption( &opt );
    QRect buttonSize = QApplication::style()->subControlRect( QStyle::CC_ToolButton, &opt, QStyle::SC_ToolButton,
                       this );
    //make sure height of icon looks good under different platforms
#ifdef Q_WS_WIN
    mIconSize = QSize( buttonSize.width() - 10, height() - 14 );
#else
    mIconSize = QSize( buttonSize.width() - 10, height() - 12 );
#endif
  }
  //create an icon pixmap
  QPixmap pixmap( mIconSize );
  pixmap.fill( Qt::transparent );

  QRect rect( 0, 0, mIconSize.width(), mIconSize.height() );
  QPainter p;
  p.begin( &pixmap );
  p.setRenderHint( QPainter::Antialiasing );
  p.setPen( Qt::NoPen );
  if ( useAlpha && backgroundColor.alpha() < 255 )
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

  setIconSize( mIconSize );
  setIcon( pixmap );
}

void QgsColorButtonV2::copyColor()
{
  //copy color
  QApplication::clipboard()->setMimeData( QgsSymbolLayerV2Utils::colorToMimeData( mColor ) );
}

void QgsColorButtonV2::pasteColor()
{
  QColor clipColor;
  if ( colorFromMimeData( QApplication::clipboard()->mimeData(), clipColor ) )
  {
    //paste color
    setColor( clipColor );
    addRecentColor( clipColor );
  }
}

void QgsColorButtonV2::activatePicker()
{
  //pick color
  QPixmap samplerPixmap = QPixmap(( const char ** ) sampler_cursor );
  setCursor( QCursor( samplerPixmap, 0, 0 ) );
  grabMouse();
  grabKeyboard();
  mPickingColor = true;
}

QColor QgsColorButtonV2::color() const
{
  return mColor;
}

void QgsColorButtonV2::setColorDialogOptions( const QColorDialog::ColorDialogOptions cdo )
{
  mColorDialogOptions = cdo;
}

QColorDialog::ColorDialogOptions QgsColorButtonV2::colorDialogOptions() const
{
  return mColorDialogOptions;
}

void QgsColorButtonV2::setColorDialogTitle( const QString title )
{
  mColorDialogTitle = title;
}

QString QgsColorButtonV2::colorDialogTitle() const
{
  return mColorDialogTitle;
}

void QgsColorButtonV2::setDefaultColor( const QColor color )
{
  mDefaultColor = color;
}

