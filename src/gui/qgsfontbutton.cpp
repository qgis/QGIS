/***************************************************************************
     qgsfontbutton.h
     ---------------
    Date                 : May 2017
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

#include "qgsfontbutton.h"
#include "qgstextformatwidget.h"
#include "qgssymbollayerutils.h"
#include "qgscolorscheme.h"
#include "qgsmapcanvas.h"
#include "qgscolorwidgets.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorswatchgrid.h"
#include "qgsdoublespinbox.h"
#include "qgsunittypes.h"
#include "qgsmenuheader.h"
#include "qgsfontutils.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayer.h"
#include "qgstextrenderer.h"
#include <QMenu>
#include <QClipboard>
#include <QDrag>
#include <QDesktopWidget>
#include <QToolTip>

QgsFontButton::QgsFontButton( QWidget *parent, const QString &dialogTitle )
  : QToolButton( parent )
  , mDialogTitle( dialogTitle.isEmpty() ? tr( "Text Format" ) : dialogTitle )
  , mNullFormatString( tr( "No Format" ) )
{
  setText( tr( "Font" ) );

  setAcceptDrops( true );
  connect( this, &QAbstractButton::clicked, this, &QgsFontButton::showSettingsDialog );

  //setup dropdown menu
  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsFontButton::prepareMenu );
  setMenu( mMenu );
  setPopupMode( QToolButton::MenuButtonPopup );

  //make sure height of button looks good under different platforms
  const QSize size = QToolButton::minimumSizeHint();
  const int fontHeight = Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.4;
  const int minWidth = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 20;
  mSizeHint = QSize( std::max( minWidth, size.width() ), std::max( size.height(), fontHeight ) );
}

QSize QgsFontButton::minimumSizeHint() const
{
  return mSizeHint;
}

QSize QgsFontButton::sizeHint() const
{
  return mSizeHint;
}

void QgsFontButton::showSettingsDialog()
{
  switch ( mMode )
  {
    case ModeTextRenderer:
    {
      QgsExpressionContext context;
      if ( mExpressionContextGenerator )
        context  = mExpressionContextGenerator->createExpressionContext();
      else
      {
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer.data() ) );
      }

      QgsSymbolWidgetContext symbolContext;
      symbolContext.setExpressionContext( &context );
      symbolContext.setMapCanvas( mMapCanvas );
      symbolContext.setMessageBar( mMessageBar );

      QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
      if ( panel && panel->dockMode() )
      {
        mActivePanel = new QgsTextFormatPanelWidget( mFormat, mMapCanvas, this, mLayer.data() );
        mActivePanel->setPanelTitle( mDialogTitle );
        mActivePanel->setContext( symbolContext );

        connect( mActivePanel, &QgsTextFormatPanelWidget::widgetChanged, this, [ this ] { setTextFormat( mActivePanel->format() ); } );
        panel->openPanel( mActivePanel );
        return;
      }

      QgsTextFormatDialog dialog( mFormat, mMapCanvas, this, QgsGuiUtils::ModalDialogFlags, mLayer.data() );
      dialog.setWindowTitle( mDialogTitle );
      dialog.setContext( symbolContext );
      if ( dialog.exec() )
      {
        setTextFormat( dialog.format() );
        QgsFontUtils::addRecentFontFamily( mFormat.font().family() );
      }
      break;
    }

    case ModeQFont:
    {
      bool ok;
      const QFont newFont = QgsGuiUtils::getFont( ok, mFont, mDialogTitle );
      if ( ok )
      {
        QgsFontUtils::addRecentFontFamily( newFont.family() );
        setCurrentFont( newFont );
      }
      break;
    }
  }

  // reactivate button's window
  activateWindow();
  raise();
}

QgsMapCanvas *QgsFontButton::mapCanvas() const
{
  return mMapCanvas;
}

void QgsFontButton::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  mMapCanvas = mapCanvas;
}

void QgsFontButton::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsFontButton::messageBar() const
{
  return mMessageBar;
}

void QgsFontButton::setTextFormat( const QgsTextFormat &format )
{
  if ( mActivePanel && !format.isValid() )
    mActivePanel->acceptPanel();

  mFormat = format;
  updatePreview();

  if ( mActivePanel && format.isValid() )
    mActivePanel->setFormat( format );
  emit changed();
}

void QgsFontButton::setToNullFormat()
{
  mFormat = QgsTextFormat();
  updatePreview();
  emit changed();
}

void QgsFontButton::setColor( const QColor &color )
{
  QColor opaque = color;
  opaque.setAlphaF( 1.0 );

  if ( mNullFormatAction )
    mNullFormatAction->setChecked( false );

  if ( mFormat.color() != opaque )
  {
    mFormat.setColor( opaque );
    updatePreview();
    emit changed();
  }
}

void QgsFontButton::copyFormat()
{
  switch ( mMode )
  {
    case ModeTextRenderer:
      QApplication::clipboard()->setMimeData( mFormat.toMimeData() );
      break;

    case ModeQFont:
      QApplication::clipboard()->setMimeData( QgsFontUtils::toMimeData( mFont ) );
      break;
  }
}

void QgsFontButton::pasteFormat()
{
  QgsTextFormat tempFormat;
  QFont font;
  if ( mMode == ModeTextRenderer && formatFromMimeData( QApplication::clipboard()->mimeData(), tempFormat ) )
  {
    setTextFormat( tempFormat );
    QgsFontUtils::addRecentFontFamily( mFormat.font().family() );
  }
  else if ( mMode == ModeQFont && fontFromMimeData( QApplication::clipboard()->mimeData(), font ) )
  {
    QgsFontUtils::addRecentFontFamily( font.family() );
    setCurrentFont( font );
  }
}

bool QgsFontButton::event( QEvent *e )
{
  if ( e->type() == QEvent::ToolTip )
  {
    QHelpEvent *helpEvent = static_cast< QHelpEvent *>( e );
    QString toolTip;
    double fontSize = 0.0;
    switch ( mMode )
    {
      case ModeTextRenderer:
        fontSize = mFormat.size();
        break;

      case ModeQFont:
        fontSize = mFont.pointSizeF();
        break;
    }
    toolTip = QStringLiteral( "<b>%1</b><br>%2<br>Size: %3" ).arg( text(), mMode == ModeTextRenderer ? mFormat.font().family() : mFont.family() ).arg( fontSize );
    QToolTip::showText( helpEvent->globalPos(), toolTip );
  }
  return QToolButton::event( e );
}

void QgsFontButton::mousePressEvent( QMouseEvent *e )
{
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

void QgsFontButton::mouseMoveEvent( QMouseEvent *e )
{
  //handle dragging fonts from button

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

  //user is dragging font
  QDrag *drag = new QDrag( this );
  switch ( mMode )
  {
    case ModeTextRenderer:
      drag->setMimeData( mFormat.toMimeData() );
      break;

    case ModeQFont:
      drag->setMimeData( QgsFontUtils::toMimeData( mFont ) );
      break;
  }
  const int iconSize = QgsGuiUtils::scaleIconSize( 50 );
  drag->setPixmap( createDragIcon( QSize( iconSize, iconSize ) ) );
  drag->exec( Qt::CopyAction );
  setDown( false );
}

bool QgsFontButton::colorFromMimeData( const QMimeData *mimeData, QColor &resultColor, bool &hasAlpha )
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

void QgsFontButton::dragEnterEvent( QDragEnterEvent *e )
{
  //is dragged data valid font data?
  QColor mimeColor;
  QgsTextFormat format;
  QFont font;
  bool hasAlpha = false;

  if ( mMode == ModeTextRenderer && formatFromMimeData( e->mimeData(), format ) )
  {
    e->acceptProposedAction();
    updatePreview( QColor(), &format );
  }
  else if ( mMode == ModeQFont && fontFromMimeData( e->mimeData(), font ) )
  {
    e->acceptProposedAction();
    updatePreview( QColor(), nullptr, &font );
  }
  else if ( mMode == ModeTextRenderer && colorFromMimeData( e->mimeData(), mimeColor, hasAlpha ) )
  {
    //if so, we accept the drag, and temporarily change the button's color
    //to match the dragged color. This gives immediate feedback to the user
    //that colors can be dropped here
    e->acceptProposedAction();
    updatePreview( mimeColor );
  }
}

void QgsFontButton::dragLeaveEvent( QDragLeaveEvent *e )
{
  Q_UNUSED( e )
  //reset button color
  updatePreview();
}

void QgsFontButton::dropEvent( QDropEvent *e )
{
  //is dropped data valid format data?
  QColor mimeColor;
  QgsTextFormat format;
  QFont font;
  bool hasAlpha = false;
  if ( mMode == ModeTextRenderer && formatFromMimeData( e->mimeData(), format ) )
  {
    setTextFormat( format );
    QgsFontUtils::addRecentFontFamily( mFormat.font().family() );
    return;
  }
  else if ( mMode == ModeQFont && fontFromMimeData( e->mimeData(), font ) )
  {
    QgsFontUtils::addRecentFontFamily( font.family() );
    setCurrentFont( font );
    return;
  }
  else if ( mMode == ModeTextRenderer && colorFromMimeData( e->mimeData(), mimeColor, hasAlpha ) )
  {
    //accept drop and set new color
    e->acceptProposedAction();

    if ( hasAlpha )
    {
      mFormat.setOpacity( mimeColor.alphaF() );
    }
    mimeColor.setAlphaF( 1.0 );
    mFormat.setColor( mimeColor );
    QgsRecentColorScheme::addRecentColor( mimeColor );
    updatePreview();
    emit changed();
  }
  updatePreview();
}

void QgsFontButton::wheelEvent( QWheelEvent *event )
{
  double size = 0;
  switch ( mMode )
  {
    case ModeTextRenderer:
      size = mFormat.size();
      break;

    case ModeQFont:
      size = mFont.pointSizeF();
      break;
  }

  const double increment = ( event->modifiers() & Qt::ControlModifier ) ? 0.1 : 1;
  if ( event->angleDelta().y() > 0 )
  {
    size += increment;
  }
  else
  {
    size -= increment;
  }
  size = std::max( size, 1.0 );

  switch ( mMode )
  {
    case ModeTextRenderer:
    {
      QgsTextFormat newFormat = mFormat;
      newFormat.setSize( size );
      setTextFormat( newFormat );
      break;
    }

    case ModeQFont:
    {
      QFont newFont = mFont;
      newFont.setPointSizeF( size );
      setCurrentFont( newFont );
      break;
    }
  }

  event->accept();
}

QPixmap QgsFontButton::createColorIcon( const QColor &color ) const
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

QPixmap QgsFontButton::createDragIcon( QSize size, const QgsTextFormat *tempFormat, const QFont *tempFont ) const
{
  if ( !tempFormat )
    tempFormat = &mFormat;
  if ( !tempFont )
    tempFont = &mFont;

  //create an icon pixmap
  QPixmap pixmap( size.width(), size.height() );
  pixmap.fill( Qt::transparent );
  QPainter p;
  p.begin( &pixmap );
  p.setRenderHint( QPainter::Antialiasing );
  const QRect rect( 0, 0, size.width(), size.height() );

  if ( mMode == ModeQFont || tempFormat->color().lightnessF() < 0.7 )
  {
    p.setBrush( QBrush( QColor( 255, 255, 255 ) ) );
    p.setPen( QPen( QColor( 150, 150, 150 ), 0 ) );
  }
  else
  {
    p.setBrush( QBrush( QColor( 0, 0, 0 ) ) );
    p.setPen( QPen( QColor( 100, 100, 100 ), 0 ) );
  }
  p.drawRect( rect );
  p.setBrush( Qt::NoBrush );
  p.setPen( Qt::NoPen );

  switch ( mMode )
  {
    case ModeTextRenderer:
    {
      QgsRenderContext context;
      QgsMapToPixel newCoordXForm;
      newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
      context.setMapToPixel( newCoordXForm );

      context.setScaleFactor( QgsApplication::desktop()->logicalDpiX() / 25.4 );
      context.setUseAdvancedEffects( true );
      context.setPainter( &p );

      // slightly inset text to account for buffer/background
      const double fontSize = context.convertToPainterUnits( tempFormat->size(), tempFormat->sizeUnit(), tempFormat->sizeMapUnitScale() );
      double xtrans = 0;
      if ( tempFormat->buffer().enabled() )
        xtrans = tempFormat->buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                 ? fontSize * tempFormat->buffer().size() / 100
                 : context.convertToPainterUnits( tempFormat->buffer().size(), tempFormat->buffer().sizeUnit(), tempFormat->buffer().sizeMapUnitScale() );
      if ( tempFormat->background().enabled() && tempFormat->background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
        xtrans = std::max( xtrans, context.convertToPainterUnits( tempFormat->background().size().width(), tempFormat->background().sizeUnit(), tempFormat->background().sizeMapUnitScale() ) );

      double ytrans = 0.0;
      if ( tempFormat->buffer().enabled() )
        ytrans = std::max( ytrans, tempFormat->buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                           ? fontSize * tempFormat->buffer().size() / 100
                           : context.convertToPainterUnits( tempFormat->buffer().size(), tempFormat->buffer().sizeUnit(), tempFormat->buffer().sizeMapUnitScale() ) );
      if ( tempFormat->background().enabled() )
        ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat->background().size().height(), tempFormat->background().sizeUnit(), tempFormat->background().sizeMapUnitScale() ) );

      QRectF textRect = rect;
      textRect.setLeft( xtrans );
      textRect.setWidth( textRect.width() - xtrans );
      textRect.setTop( ytrans );
      if ( textRect.height() > 300 )
        textRect.setHeight( 300 );
      if ( textRect.width() > 2000 )
        textRect.setWidth( 2000 );

      QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::AlignCenter, QStringList() << tr( "Aa" ),
                                 context, *tempFormat );
      break;
    }
    case ModeQFont:
    {
      p.setBrush( Qt::NoBrush );
      p.setPen( QColor( 0, 0, 0 ) );
      p.setFont( *tempFont );
      QRectF textRect = rect;
      textRect.setLeft( 2 );
      p.drawText( textRect, Qt::AlignVCenter, tr( "Aa" ) );
      break;
    }
  }

  p.end();
  return pixmap;
}

void QgsFontButton::prepareMenu()
{
  //we need to tear down and rebuild this menu every time it is shown. Otherwise the space allocated to any
  //QgsColorSwatchGridAction is not recalculated by Qt and the swatch grid may not be the correct size
  //for the number of colors shown in the grid. Note that we MUST refresh color swatch grids every time this
  //menu is opened, otherwise color schemes like the recent color scheme grid are meaningless
  mMenu->clear();

  if ( mMode == ModeTextRenderer && mShowNoFormat )
  {
    mNullFormatAction = new QAction( mNullFormatString, this );
    mMenu->addAction( mNullFormatAction );
    connect( mNullFormatAction, &QAction::triggered, this, &QgsFontButton::setToNullFormat );
    if ( !mFormat.isValid() )
    {
      mNullFormatAction->setCheckable( true );
      mNullFormatAction->setChecked( true );
    }
  }

  QWidgetAction *sizeAction = new QWidgetAction( mMenu );
  QWidget *sizeWidget = new QWidget();
  QVBoxLayout *sizeLayout = new QVBoxLayout();
  sizeLayout->setContentsMargins( 0, 0, 0, 3 );
  sizeLayout->setSpacing( 2 );

  QString fontHeaderLabel;
  switch ( mMode )
  {
    case ModeTextRenderer:
      fontHeaderLabel = tr( "Font size (%1)" ).arg( QgsUnitTypes::toString( mFormat.sizeUnit() ) );
      break;

    case ModeQFont:
      fontHeaderLabel = tr( "Font size (pt)" );
      break;
  }

  QgsMenuHeader *sizeLabel = new QgsMenuHeader( fontHeaderLabel );
  sizeLayout->addWidget( sizeLabel );

  QgsDoubleSpinBox *sizeSpin = new QgsDoubleSpinBox( nullptr );
  sizeSpin->setDecimals( 4 );
  sizeSpin->setMaximum( 1e+9 );
  sizeSpin->setShowClearButton( false );
  sizeSpin->setValue( mMode == ModeTextRenderer ? mFormat.size() : mFont.pointSizeF() );
  connect( sizeSpin, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ),
           this, [ = ]( double value )
  {
    switch ( mMode )
    {
      case ModeTextRenderer:
        if ( mNullFormatAction )
          mNullFormatAction->setChecked( false );
        mFormat.setSize( value );
        break;
      case ModeQFont:
        mFont.setPointSizeF( value );
        break;
    }
    updatePreview();
    emit changed();
  } );
  QHBoxLayout *spinLayout = new QHBoxLayout();
  spinLayout->setContentsMargins( 4, 0, 4, 0 );
  spinLayout->addWidget( sizeSpin );
  sizeLayout->addLayout( spinLayout );
  sizeWidget->setLayout( sizeLayout );
  sizeAction->setDefaultWidget( sizeWidget );
  sizeWidget->setFocusProxy( sizeSpin );
  sizeWidget->setFocusPolicy( Qt::StrongFocus );
  mMenu->addAction( sizeAction );

  QMenu *recentFontMenu = new QMenu( tr( "Recent Fonts" ), mMenu );
  const auto recentFontFamilies { QgsFontUtils::recentFontFamilies() };
  for ( const QString &family : recentFontFamilies )
  {
    QAction *fontAction = new QAction( family, recentFontMenu );
    QFont f = fontAction->font();
    f.setFamily( family );
    fontAction->setFont( f );
    fontAction->setToolTip( family );
    recentFontMenu->addAction( fontAction );
    if ( ( mMode == ModeTextRenderer && family == mFormat.font().family() )
         || ( mMode == ModeQFont && family == mFont.family() ) )
    {
      fontAction->setCheckable( true );
      fontAction->setChecked( true );
    }
    auto setFont = [this, family]
    {
      switch ( mMode )
      {
        case ModeTextRenderer:
        {
          QgsTextFormat newFormat = mFormat;
          QFont f = newFormat.font();
          f.setFamily( family );
          newFormat.setFont( f );
          setTextFormat( newFormat );
          QgsFontUtils::addRecentFontFamily( mFormat.font().family() );
          break;
        }
        case ModeQFont:
        {
          QFont font = mFont;
          font.setFamily( family );
          setCurrentFont( font );
          QgsFontUtils::addRecentFontFamily( family );
          break;
        }
      }
    };
    connect( fontAction, &QAction::triggered, this, setFont );
  }
  mMenu->addMenu( recentFontMenu );

  QAction *configureAction = new QAction( tr( "Configure Format…" ), this );
  mMenu->addAction( configureAction );
  connect( configureAction, &QAction::triggered, this, &QgsFontButton::showSettingsDialog );

  QAction *copyFormatAction = new QAction( tr( "Copy Format" ), this );
  mMenu->addAction( copyFormatAction );
  connect( copyFormatAction, &QAction::triggered, this, &QgsFontButton::copyFormat );
  QAction *pasteFormatAction = new QAction( tr( "Paste Format" ), this );
  //enable or disable paste action based on current clipboard contents. We always show the paste
  //action, even if it's disabled, to give hint to the user that pasting colors is possible
  QgsTextFormat tempFormat;
  QFont tempFont;
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  if ( mMode == ModeTextRenderer && formatFromMimeData( QApplication::clipboard()->mimeData(), tempFormat ) )
  {
    tempFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
    tempFormat.setSize( 14 );
    pasteFormatAction->setIcon( createDragIcon( QSize( iconSize, iconSize ), &tempFormat ) );
  }
  else if ( mMode == ModeQFont && fontFromMimeData( QApplication::clipboard()->mimeData(), tempFont ) )
  {
    tempFont.setPointSize( 8 );
    pasteFormatAction->setIcon( createDragIcon( QSize( iconSize, iconSize ), nullptr, &tempFont ) );
  }
  else
  {
    pasteFormatAction->setEnabled( false );
  }
  mMenu->addAction( pasteFormatAction );
  connect( pasteFormatAction, &QAction::triggered, this, &QgsFontButton::pasteFormat );

  if ( mMode == ModeTextRenderer )
  {
    mMenu->addSeparator();

    QgsColorWheel *colorWheel = new QgsColorWheel( mMenu );
    colorWheel->setColor( mFormat.color() );
    QgsColorWidgetAction *colorAction = new QgsColorWidgetAction( colorWheel, mMenu, mMenu );
    colorAction->setDismissOnColorSelection( false );
    connect( colorAction, &QgsColorWidgetAction::colorChanged, this, &QgsFontButton::setColor );
    mMenu->addAction( colorAction );

    QgsColorRampWidget *alphaRamp = new QgsColorRampWidget( mMenu, QgsColorWidget::Alpha, QgsColorRampWidget::Horizontal );
    QColor alphaColor = mFormat.color();
    alphaColor.setAlphaF( mFormat.opacity() );
    alphaRamp->setColor( alphaColor );
    QgsColorWidgetAction *alphaAction = new QgsColorWidgetAction( alphaRamp, mMenu, mMenu );
    alphaAction->setDismissOnColorSelection( false );
    connect( alphaAction, &QgsColorWidgetAction::colorChanged, this, [ = ]( const QColor & color )
    {
      const double opacity = color.alphaF();
      mFormat.setOpacity( opacity );
      updatePreview();
      if ( mNullFormatAction )
        mNullFormatAction->setChecked( false );
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
      QgsColorSwatchGridAction *colorAction = new QgsColorSwatchGridAction( *it, mMenu, QStringLiteral( "labeling" ), this );
      colorAction->setBaseColor( mFormat.color() );
      mMenu->addAction( colorAction );
      connect( colorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsFontButton::setColor );
      connect( colorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsFontButton::addRecentColor );
    }

    mMenu->addSeparator();

    QAction *copyColorAction = new QAction( tr( "Copy Color" ), this );
    mMenu->addAction( copyColorAction );
    connect( copyColorAction, &QAction::triggered, this, &QgsFontButton::copyColor );

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
    connect( pasteColorAction, &QAction::triggered, this, &QgsFontButton::pasteColor );
  }
}

void QgsFontButton::addRecentColor( const QColor &color )
{
  QgsRecentColorScheme::addRecentColor( color );
}

QFont QgsFontButton::currentFont() const
{
  return mFont;
}

QgsVectorLayer *QgsFontButton::layer() const
{
  return mLayer;
}

void QgsFontButton::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsFontButton::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

void QgsFontButton::setCurrentFont( const QFont &font )
{
  mFont = font;
  updatePreview();
  emit changed();
}

QgsFontButton::Mode QgsFontButton::mode() const
{
  return mMode;
}

void QgsFontButton::setMode( Mode mode )
{
  mMode = mode;
  updatePreview();
}

bool QgsFontButton::formatFromMimeData( const QMimeData *mimeData, QgsTextFormat &resultFormat ) const
{
  bool ok = false;
  resultFormat = QgsTextFormat::fromMimeData( mimeData, &ok );
  return ok;
}

bool QgsFontButton::fontFromMimeData( const QMimeData *mimeData, QFont &resultFont ) const
{
  bool ok = false;
  resultFont = QgsFontUtils::fromMimeData( mimeData, &ok );
  return ok;
}

void QgsFontButton::changeEvent( QEvent *e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    updatePreview();
  }
  QToolButton::changeEvent( e );
}

void QgsFontButton::showEvent( QShowEvent *e )
{
  updatePreview();
  QToolButton::showEvent( e );
}

void QgsFontButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  //recalculate icon size and redraw icon
  mIconSize = QSize();
  updatePreview();
}

void QgsFontButton::updatePreview( const QColor &color, QgsTextFormat *format, QFont *font )
{
  if ( mShowNoFormat && !mFormat.isValid() )
  {
    setIcon( QPixmap() );
    return;
  }

  QgsTextFormat tempFormat;
  QFont tempFont;

  if ( format )
    tempFormat = *format;
  else
    tempFormat = mFormat;
  if ( font )
    tempFont = *font;
  else
    tempFont = mFont;

  if ( color.isValid() )
    tempFormat.setColor( color );

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
#elif defined(Q_OS_MAC)
      mIconSize = QSize( buttonSize.width() - 10, height() - 2 );
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
  QPainter p;
  p.begin( &pixmap );
  p.setRenderHint( QPainter::Antialiasing );
  const QRect rect( 0, 0, currentIconSize.width(), currentIconSize.height() );

  switch ( mMode )
  {
    case ModeTextRenderer:
    {
      QgsRenderContext context;
      QgsMapToPixel newCoordXForm;
      newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
      context.setMapToPixel( newCoordXForm );

      context.setScaleFactor( QgsApplication::desktop()->logicalDpiX() / 25.4 );
      context.setUseAdvancedEffects( true );
      context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
      context.setPainter( &p );

      // slightly inset text to account for buffer/background
      const double fontSize = context.convertToPainterUnits( tempFormat.size(), tempFormat.sizeUnit(), tempFormat.sizeMapUnitScale() );
      double xtrans = 0;
      if ( tempFormat.buffer().enabled() )
        xtrans = tempFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                 ? fontSize * tempFormat.buffer().size() / 100
                 : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() );
      if ( tempFormat.background().enabled() && tempFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
        xtrans = std::max( xtrans, context.convertToPainterUnits( tempFormat.background().size().width(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

      double ytrans = 0.0;
      if ( tempFormat.buffer().enabled() )
        ytrans = std::max( ytrans, tempFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                           ? fontSize * tempFormat.buffer().size() / 100
                           : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() ) );
      if ( tempFormat.background().enabled() )
        ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat.background().size().height(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

      QRectF textRect = rect;
      textRect.setLeft( xtrans );
      textRect.setWidth( textRect.width() - xtrans );
      textRect.setTop( ytrans );
      if ( textRect.height() > 300 )
        textRect.setHeight( 300 );
      if ( textRect.width() > 2000 )
        textRect.setWidth( 2000 );

      QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::AlignLeft, QStringList() << text(),
                                 context, tempFormat );
      break;
    }
    case ModeQFont:
    {
      p.setBrush( Qt::NoBrush );
      p.setPen( QColor( 0, 0, 0 ) );
      p.setFont( tempFont );
      QRectF textRect = rect;
      textRect.setLeft( 2 );
      p.drawText( textRect, Qt::AlignVCenter, text() );
      break;
    }

  }
  p.end();
  setIconSize( currentIconSize );
  setIcon( pixmap );
}

void QgsFontButton::copyColor()
{
  //copy color
  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::colorToMimeData( mFormat.color() ) );
}

void QgsFontButton::pasteColor()
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

void QgsFontButton::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

QString QgsFontButton::dialogTitle() const
{
  return mDialogTitle;
}
