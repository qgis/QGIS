/***************************************************************************
     qgslegendpatchshapebutton.cpp
     -----------------
    Date                 : April 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendpatchshapebutton.h"
#include "moc_qgslegendpatchshapebutton.cpp"
#include "qgslegendpatchshapewidget.h"
#include "qgis.h"
#include "qgsguiutils.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

#include <QMenu>
#include <QBuffer>

QgsLegendPatchShapeButton::QgsLegendPatchShapeButton( QWidget *parent, const QString &dialogTitle )
  : QToolButton( parent )
  , mShape( QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Fill, QSizeF( 10, 5 ) ) )
  , mDialogTitle( dialogTitle.isEmpty() ? tr( "Legend Patch Shape" ) : dialogTitle )
{
  mPreviewSymbol.reset( QgsFillSymbol::createSimple( QVariantMap() ) );

  connect( this, &QAbstractButton::clicked, this, &QgsLegendPatchShapeButton::showSettingsDialog );

  //setup dropdown menu
  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsLegendPatchShapeButton::prepareMenu );
  setMenu( mMenu );
  setPopupMode( QToolButton::MenuButtonPopup );

  //make sure height of button looks good under different platforms
  QSize size = QToolButton::minimumSizeHint();
  int fontHeight = static_cast<int>( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 2.0 );
  mSizeHint = QSize( size.width(), std::max( size.height(), fontHeight ) );
}

QgsLegendPatchShapeButton::~QgsLegendPatchShapeButton() = default;

QSize QgsLegendPatchShapeButton::minimumSizeHint() const
{
  return mSizeHint;
}

QSize QgsLegendPatchShapeButton::sizeHint() const
{
  return mSizeHint;
}

void QgsLegendPatchShapeButton::setSymbolType( Qgis::SymbolType type )
{
  if ( mPreviewSymbol->type() != type )
  {
    switch ( type )
    {
      case Qgis::SymbolType::Marker:
        mPreviewSymbol.reset( QgsMarkerSymbol::createSimple( QVariantMap() ) );
        break;

      case Qgis::SymbolType::Line:
        mPreviewSymbol.reset( QgsLineSymbol::createSimple( QVariantMap() ) );
        break;

      case Qgis::SymbolType::Fill:
        mPreviewSymbol.reset( QgsFillSymbol::createSimple( QVariantMap() ) );
        break;

      case Qgis::SymbolType::Hybrid:
        break;
    }
  }

  if ( type != mType )
  {
    mType = type;
    setToDefault();
  }

  updatePreview();
}

void QgsLegendPatchShapeButton::setPreviewSymbol( QgsSymbol *symbol )
{
  mPreviewSymbol.reset( symbol );
  updatePreview();
}

void QgsLegendPatchShapeButton::showSettingsDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsLegendPatchShapeWidget *widget = new QgsLegendPatchShapeWidget( this, mShape );
    connect( widget, &QgsLegendPatchShapeWidget::changed, this, [=] {
      setShape( widget->shape() );
    } );
    widget->setPanelTitle( mDialogTitle );
    panel->openPanel( widget );
  }
}

void QgsLegendPatchShapeButton::setToDefault()
{
  switch ( mType )
  {
    case Qgis::SymbolType::Marker:
      mShape = QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Marker, QSizeF( 10, 5 ) );
      break;

    case Qgis::SymbolType::Line:
      mShape = QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Line, QSizeF( 10, 5 ) );
      break;

    case Qgis::SymbolType::Fill:
      mShape = QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Fill, QSizeF( 10, 5 ) );
      break;

    case Qgis::SymbolType::Hybrid:
      break;
  }
  mIsDefault = true;
  updatePreview();
  emit changed();
}

void QgsLegendPatchShapeButton::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsLegendPatchShapeButton::messageBar() const
{
  return mMessageBar;
}

void QgsLegendPatchShapeButton::setShape( const QgsLegendPatchShape &shape )
{
  mShape = shape.symbolType() == mType ? shape : QgsLegendPatchShape();
  mIsDefault = mShape.isNull();
  if ( mIsDefault )
  {
    switch ( mType )
    {
      case Qgis::SymbolType::Marker:
        mShape = QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Marker, QSizeF( 10, 5 ) );
        break;

      case Qgis::SymbolType::Line:
        mShape = QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Line, QSizeF( 10, 5 ) );
        break;

      case Qgis::SymbolType::Fill:
        mShape = QgsStyle::defaultStyle()->defaultPatch( Qgis::SymbolType::Fill, QSizeF( 10, 5 ) );
        break;

      case Qgis::SymbolType::Hybrid:
        break;
    }
  }

  updatePreview();
  emit changed();
}

void QgsLegendPatchShapeButton::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    QToolButton::showMenu();
    return;
  }
  QToolButton::mousePressEvent( e );
}

void QgsLegendPatchShapeButton::prepareMenu()
{
  mMenu->clear();

  QAction *configureAction = new QAction( tr( "Configure Patch…" ), this );
  mMenu->addAction( configureAction );
  connect( configureAction, &QAction::triggered, this, &QgsLegendPatchShapeButton::showSettingsDialog );

  QAction *defaultAction = new QAction( tr( "Reset to Default" ), this );
  mMenu->addAction( defaultAction );
  connect( defaultAction, &QAction::triggered, this, [=] { setToDefault(); emit changed(); } );

  mMenu->addSeparator();

  QStringList patchNames = QgsStyle::defaultStyle()->symbolsOfFavorite( QgsStyle::LegendPatchShapeEntity );
  patchNames.sort();
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  for ( const QString &name : std::as_const( patchNames ) )
  {
    const QgsLegendPatchShape shape = QgsStyle::defaultStyle()->legendPatchShape( name );
    if ( shape.symbolType() == mType )
    {
      if ( const QgsSymbol *symbol = QgsStyle::defaultStyle()->previewSymbolForPatchShape( shape ) )
      {
        QIcon icon = QgsSymbolLayerUtils::symbolPreviewPixmap( symbol, QSize( iconSize, iconSize ), 1, nullptr, false, nullptr, &shape, QgsScreenProperties( screen() ) );
        QAction *action = new QAction( name, this );
        action->setIcon( icon );
        connect( action, &QAction::triggered, this, [=] { loadPatchFromStyle( name ); } );
        mMenu->addAction( action );
      }
    }
  }
}

void QgsLegendPatchShapeButton::loadPatchFromStyle( const QString &name )
{
  if ( !QgsStyle::defaultStyle()->legendPatchShapeNames().contains( name ) )
    return;

  const QgsLegendPatchShape newShape = QgsStyle::defaultStyle()->legendPatchShape( name );
  setShape( newShape );
}

void QgsLegendPatchShapeButton::changeEvent( QEvent *e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    updatePreview();
  }
  QToolButton::changeEvent( e );
}

void QgsLegendPatchShapeButton::showEvent( QShowEvent *e )
{
  updatePreview();
  QToolButton::showEvent( e );
}

void QgsLegendPatchShapeButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  //recalculate icon size and redraw icon
  mIconSize = QSize();
  updatePreview();
}

void QgsLegendPatchShapeButton::updatePreview()
{
  QSize currentIconSize;
  //icon size is button size with a small margin
  if ( menu() )
  {
    if ( !mIconSize.isValid() )
    {
      //calculate size of push button part of widget (ie, without the menu dropdown button part)
      QStyleOptionToolButton opt;
      initStyleOption( &opt );
      QRect buttonSize = QApplication::style()->subControlRect( QStyle::CC_ToolButton, &opt, QStyle::SC_ToolButton, this );
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
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mPreviewSymbol.get(), currentIconSize, currentIconSize.height() / 10, &mShape, QgsScreenProperties( screen() ) );
  setIconSize( currentIconSize );
  setIcon( icon );

  // set tooltip
  // create very large preview image

  int width = static_cast<int>( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 23 );
  int height = static_cast<int>( width / 1.61803398875 ); // golden ratio

  QPixmap pm = QgsSymbolLayerUtils::symbolPreviewPixmap( mPreviewSymbol.get(), QSize( width, height ), height / 20, nullptr, false, nullptr, &mShape, QgsScreenProperties( screen() ) );
  QByteArray data;
  QBuffer buffer( &data );
  pm.save( &buffer, "PNG", 100 );
  setToolTip( QStringLiteral( "<img src='data:image/png;base64, %3' width=\"%4\">" ).arg( QString( data.toBase64() ) ).arg( width ) );
}

void QgsLegendPatchShapeButton::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

QString QgsLegendPatchShapeButton::dialogTitle() const
{
  return mDialogTitle;
}

QgsLegendPatchShape QgsLegendPatchShapeButton::shape()
{
  return mIsDefault ? QgsLegendPatchShape() : mShape;
}
