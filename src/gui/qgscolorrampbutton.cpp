/***************************************************************************
    qgscolorrampbutton.cpp - Color ramp button
     --------------------------------------
    Date                 : November 27, 2016
    Copyright            : (C) 2016 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorrampbutton.h"
#include "qgscolorramp.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssymbollayerutils.h"
#include "qgsstyle.h"

#include "qgsstylesavedialog.h"
#include "qgsgradientcolorrampdialog.h"
#include "qgslimitedrandomcolorrampdialog.h"
#include "qgscolorbrewercolorrampdialog.h"
#include "qgscptcitycolorrampdialog.h"
#include "qgspresetcolorrampdialog.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QWidget>

QgsColorRampButton::QgsColorRampButton( QWidget *parent, const QString &dialogTitle )
  : QToolButton( parent )
  , mColorRampDialogTitle( dialogTitle.isEmpty() ? tr( "Select Color Ramp" ) : dialogTitle )
{
  setAcceptDrops( true );
  setMinimumSize( QSize( 24, 16 ) );
  connect( this, &QPushButton::clicked, this, &QgsColorRampButton::buttonClicked );

  //setup drop-down menu
  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsColorRampButton::prepareMenu );
  setMenu( mMenu );

  mAllRampsMenu = new QMenu( mMenu );
  mAllRampsMenu->setTitle( tr( "All Color Ramps" ) );

  setPopupMode( QToolButton::MenuButtonPopup );

  mStyle = QgsStyle::defaultStyle();
}

QgsColorRampButton::~QgsColorRampButton()
{
  delete mColorRamp;
  delete mDefaultColorRamp;
}

QSize QgsColorRampButton::sizeHint() const
{
  //make sure height of button looks good under different platforms
#ifdef Q_OS_WIN
  return QSize( 120, static_cast<int>( std::max( Qgis::UI_SCALE_FACTOR * fontMetrics().height( ), 22.0 ) ) );
#else
  // Adjust height for HiDPI screens
  return QSize( 120, static_cast<int>( std::max( Qgis::UI_SCALE_FACTOR * fontMetrics().height( ) * 1.4, 28.0 ) ) );
#endif
}

void QgsColorRampButton::showColorRampDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  bool panelMode = panel && panel->dockMode();

  std::unique_ptr< QgsColorRamp > currentRamp( colorRamp() );
  if ( !currentRamp )
    return;

  setColorRampName( QString() );

  if ( currentRamp->type() == QLatin1String( "gradient" ) )
  {
    QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>( currentRamp.get() );
    QgsGradientColorRampDialog dlg( *gradRamp, this );
    dlg.setWindowTitle( mColorRampDialogTitle );
    if ( dlg.exec() )
    {
      setColorRamp( dlg.ramp().clone() );
    }
  }
  else if ( currentRamp->type() == QLatin1String( "random" ) )
  {
    QgsLimitedRandomColorRamp *randRamp = static_cast<QgsLimitedRandomColorRamp *>( currentRamp.get() );
    if ( panelMode )
    {
      QgsLimitedRandomColorRampWidget *widget = new QgsLimitedRandomColorRampWidget( *randRamp, this );
      widget->setPanelTitle( tr( "Edit Ramp" ) );
      connect( widget, &QgsLimitedRandomColorRampWidget::changed, this, &QgsColorRampButton::rampWidgetUpdated );
      panel->openPanel( widget );
    }
    else
    {
      QgsLimitedRandomColorRampDialog dlg( *randRamp, this );
      if ( dlg.exec() )
      {
        setColorRamp( dlg.ramp().clone() );
      }
    }
  }
  else if ( currentRamp->type() == QLatin1String( "preset" ) )
  {
    QgsPresetSchemeColorRamp *presetRamp = static_cast<QgsPresetSchemeColorRamp *>( currentRamp.get() );
    if ( panelMode )
    {
      QgsPresetColorRampWidget *widget = new QgsPresetColorRampWidget( *presetRamp, this );
      widget->setPanelTitle( tr( "Edit Ramp" ) );
      connect( widget, &QgsPresetColorRampWidget::changed, this, &QgsColorRampButton::rampWidgetUpdated );
      panel->openPanel( widget );
    }
    else
    {
      QgsPresetColorRampDialog dlg( *presetRamp, this );
      if ( dlg.exec() )
      {
        setColorRamp( dlg.ramp().clone() );
      }
    }
  }
  else if ( currentRamp->type() == QLatin1String( "colorbrewer" ) )
  {
    QgsColorBrewerColorRamp *brewerRamp = static_cast<QgsColorBrewerColorRamp *>( currentRamp.get() );
    if ( panelMode )
    {
      QgsColorBrewerColorRampWidget *widget = new QgsColorBrewerColorRampWidget( *brewerRamp, this );
      widget->setPanelTitle( tr( "Edit Ramp" ) );
      connect( widget, &QgsColorBrewerColorRampWidget::changed, this, &QgsColorRampButton::rampWidgetUpdated );
      panel->openPanel( widget );
    }
    else
    {
      QgsColorBrewerColorRampDialog dlg( *brewerRamp, this );
      if ( dlg.exec() )
      {
        setColorRamp( dlg.ramp().clone() );
      }
    }
  }
  else if ( currentRamp->type() == QLatin1String( "cpt-city" ) )
  {
    QgsCptCityColorRamp *cptCityRamp = static_cast<QgsCptCityColorRamp *>( currentRamp.get() );
    QgsCptCityColorRampDialog dlg( *cptCityRamp, this );
    if ( dlg.exec() )
    {
      if ( dlg.saveAsGradientRamp() )
      {
        setColorRamp( dlg.ramp().cloneGradientRamp() );
      }
      else
      {
        setColorRamp( dlg.ramp().clone() );
      }
    }
  }
}

void QgsColorRampButton::setToDefaultColorRamp()
{
  if ( !mDefaultColorRamp )
  {
    return;
  }

  setColorRamp( mDefaultColorRamp );
}

void QgsColorRampButton::setToNull()
{
  setColorRamp( nullptr );
}

bool QgsColorRampButton::event( QEvent *e )
{
  if ( e->type() == QEvent::ToolTip )
  {
    if ( !colorRampName().isEmpty() )
    {
      setToolTip( colorRampName() );
    }
  }
  return QToolButton::event( e );
}

void QgsColorRampButton::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    QToolButton::showMenu();
    return;
  }
  QToolButton::mousePressEvent( e );
}

QPixmap QgsColorRampButton::createMenuIcon( QgsColorRamp *colorramp )
{
  //create an icon pixmap
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  QPixmap pixmap = QgsSymbolLayerUtils::colorRampPreviewPixmap( colorramp, QSize( iconSize, iconSize ) );
  return pixmap;
}

void QgsColorRampButton::buttonClicked()
{
  if ( !isRandomColorRamp() && !isNull() )
  {
    showColorRampDialog();
  }
  else
  {
    QToolButton::showMenu();
  }
}

void QgsColorRampButton::prepareMenu()
{
  mMenu->clear();

  QAction *invertAction = new QAction( tr( "Invert Color Ramp" ), this );
  invertAction->setEnabled( !isNull() && !isRandomColorRamp() );
  mMenu->addAction( invertAction );
  connect( invertAction, &QAction::triggered, this, &QgsColorRampButton::invertColorRamp );

  if ( mShowNull )
  {
    QAction *nullAction = new QAction( tr( "Clear Current Ramp" ), this );
    mMenu->addAction( nullAction );
    connect( nullAction, &QAction::triggered, this, &QgsColorRampButton::setToNull );
  }

  mMenu->addSeparator();

  //show default color option if set
  if ( mDefaultColorRamp )
  {
    QAction *defaultColorRampAction = new QAction( tr( "Default Color Ramp" ), this );
    defaultColorRampAction->setIcon( createMenuIcon( mDefaultColorRamp ) );
    mMenu->addAction( defaultColorRampAction );
    connect( defaultColorRampAction, &QAction::triggered, this, &QgsColorRampButton::setToDefaultColorRamp );
  }

  if ( mShowRandomColorRamp )
  {
    QAction *randomColorRampAction = new QAction( tr( "Random Color Ramp" ), this );
    randomColorRampAction->setCheckable( true );
    randomColorRampAction->setChecked( isRandomColorRamp() );
    mMenu->addAction( randomColorRampAction );
    connect( randomColorRampAction, &QAction::triggered, this, &QgsColorRampButton::setRandomColorRamp );

    if ( isRandomColorRamp() || dynamic_cast<QgsLimitedRandomColorRamp *>( mColorRamp ) )
    {
      QAction *shuffleRandomColorRampAction = new QAction( tr( "Shuffle Random Colors" ), this );
      mMenu->addAction( shuffleRandomColorRampAction );
      connect( shuffleRandomColorRampAction, &QAction::triggered, this, &QgsColorRampButton::colorRampChanged );
    }
  }

  mMenu->addSeparator();

  QStringList rampNames = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  rampNames.sort();
  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  for ( QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it )
  {
    std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( *it ) );

    if ( !mShowGradientOnly || ( ramp->type() == QLatin1String( "gradient" ) || ramp->type() == QLatin1String( "cpt-city" ) ) )
    {
      QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp.get(), QSize( iconSize, iconSize ) );
      QAction *ra = new QAction( *it, this );
      ra->setIcon( icon );
      connect( ra, &QAction::triggered, this, &QgsColorRampButton::loadColorRamp );
      mMenu->addAction( ra );
    }
  }

  mMenu->addSeparator();

  mAllRampsMenu->clear();
  mMenu->addMenu( mAllRampsMenu );
  rampNames = mStyle->colorRampNames();
  rampNames.sort();
  for ( QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it )
  {
    std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( *it ) );

    if ( !mShowGradientOnly || ( ramp->type() == QLatin1String( "gradient" ) || ramp->type() == QLatin1String( "cpt-city" ) ) )
    {
      QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp.get(), QSize( iconSize, iconSize ) );
      QAction *ra = new QAction( *it, this );
      ra->setIcon( icon );
      connect( ra, &QAction::triggered, this, &QgsColorRampButton::loadColorRamp );
      mAllRampsMenu->addAction( ra );
    }
  }

  mMenu->addSeparator();

  QAction *newColorRampAction = new QAction( tr( "Create New Color Ramp…" ), this );
  connect( newColorRampAction, &QAction::triggered, this, &QgsColorRampButton::createColorRamp );
  mMenu->addAction( newColorRampAction );

  QAction *editColorRampAction = new QAction( tr( "Edit Color Ramp…" ), this );
  editColorRampAction->setEnabled( !isNull() && !isRandomColorRamp() );
  connect( editColorRampAction, &QAction::triggered, this, &QgsColorRampButton::showColorRampDialog );
  mMenu->addAction( editColorRampAction );

  QAction *saveColorRampAction = new QAction( tr( "Save Color Ramp…" ), this );
  saveColorRampAction->setEnabled( !isNull() && !isRandomColorRamp() );
  connect( saveColorRampAction, &QAction::triggered, this, &QgsColorRampButton::saveColorRamp );
  mMenu->addAction( saveColorRampAction );
}

void QgsColorRampButton::loadColorRamp()
{
  QAction *selectedItem = qobject_cast<QAction *>( sender() );
  if ( selectedItem )
  {
    QString name = selectedItem->iconText();
    setColorRampName( name );
    setColorRampFromName( name );
  }
}

void QgsColorRampButton::createColorRamp()
{
  QStringList rampTypes;
  QString rampType;
  bool ok = true;

  if ( mShowGradientOnly )
  {
    rampTypes << tr( "Gradient" ) << tr( "Catalog: cpt-city" );
  }
  else
  {
    rampTypes << tr( "Gradient" ) << tr( "Color presets" ) << tr( "Random" ) << tr( "Catalog: cpt-city" );
    rampTypes << tr( "Catalog: ColorBrewer" );
  }
  rampType = QInputDialog::getItem( this, tr( "Color ramp type" ),
                                    tr( "Please select color ramp type:" ), rampTypes, 0, false, &ok );

  if ( !ok || rampType.isEmpty() )
    return;

  QgsColorRamp  *ramp = nullptr;
  if ( rampType == tr( "Gradient" ) )
  {
    ramp = new QgsGradientColorRamp();
  }
  else if ( rampType == tr( "Random" ) )
  {
    ramp = new QgsLimitedRandomColorRamp();
  }
  else if ( rampType == tr( "Catalog: ColorBrewer" ) )
  {
    ramp = new QgsColorBrewerColorRamp();
  }
  else if ( rampType == tr( "Color presets" ) )
  {
    ramp = new QgsPresetSchemeColorRamp();
  }
  else if ( rampType == tr( "Catalog: cpt-city" ) )
  {
    ramp = new QgsCptCityColorRamp( QString(), QString() );
  }
  else
  {
    QgsDebugMsg( "invalid ramp type " + rampType );
    return;
  }

  setColorRamp( ramp );
  delete ramp;

  showColorRampDialog();
}

void QgsColorRampButton::saveColorRamp()
{
  QgsStyleSaveDialog saveDlg( this, QgsStyle::ColorrampEntity );
  if ( !saveDlg.exec() || saveDlg.name().isEmpty() )
  {
    return;
  }

  // check if there is no symbol with same name
  if ( mStyle->symbolNames().contains( saveDlg.name() ) )
  {
    int res = QMessageBox::warning( this, tr( "Save Color Ramp" ),
                                    tr( "Color ramp with name '%1' already exists. Overwrite?" )
                                    .arg( saveDlg.name() ),
                                    QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    mStyle->removeColorRamp( saveDlg.name() );
  }

  QStringList colorRampTags = saveDlg.tags().split( ',' );

  // add new symbol to style and re-populate the list
  QgsColorRamp *ramp = colorRamp();
  mStyle->addColorRamp( saveDlg.name(), ramp );
  mStyle->saveColorRamp( saveDlg.name(), ramp, saveDlg.isFavorite(), colorRampTags );

  setColorRampName( saveDlg.name() );
}

void QgsColorRampButton::invertColorRamp()
{
  mColorRamp->invert();
  setButtonBackground();
  emit colorRampChanged();
}

void QgsColorRampButton::changeEvent( QEvent *e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    setButtonBackground();
  }
  QToolButton::changeEvent( e );
}

#if 0 // causes too many cyclical updates, but may be needed on some platforms
void QgsColorRampButton::paintEvent( QPaintEvent *e )
{
  QToolButton::paintEvent( e );

  if ( !mBackgroundSet )
  {
    setButtonBackground();
  }
}
#endif

void QgsColorRampButton::showEvent( QShowEvent *e )
{
  setButtonBackground();
  QToolButton::showEvent( e );
}

void QgsColorRampButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  //recalculate icon size and redraw icon
  mIconSize = QSize();
  setButtonBackground( mColorRamp );
}

void QgsColorRampButton::setColorRamp( QgsColorRamp *colorramp )
{
  QgsColorRamp *oldColorRamp = mColorRamp;
  mColorRamp = colorramp->clone();

  // handle when initially set color is same as default (Qt::black); consider it a color change
  if ( ( oldColorRamp != mColorRamp ) || !mColorRampSet )
  {
    setButtonBackground();
    if ( isEnabled() )
    {
      emit colorRampChanged();
    }
  }
  delete oldColorRamp;
  mColorRampSet = true;
}

void QgsColorRampButton::setColorRampFromName( const QString &name )
{
  if ( !name.isEmpty() )
  {
    std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( name ) );
    if ( ramp )
    {
      setColorRamp( ramp.get() );
    }
  }
}

void QgsColorRampButton::setRandomColorRamp()
{
  if ( !isRandomColorRamp() )
  {
    std::unique_ptr< QgsRandomColorRamp > newRamp( new QgsRandomColorRamp() );
    setColorRamp( newRamp.get() );
  }
}

void QgsColorRampButton::setButtonBackground( QgsColorRamp *colorramp )
{
  QgsColorRamp *backgroundColorRamp = colorramp;
  if ( !colorramp )
  {
    backgroundColorRamp = mColorRamp;
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

  QPixmap pm;
  if ( isRandomColorRamp() )
  {
    //create a "random colors" label
    pm = QPixmap( currentIconSize );
    pm.fill( Qt::transparent );

    QPainter painter;
    QPen pen  = ( palette().buttonText().color() );

    painter.begin( &pm );
    painter.setPen( pen );
    painter.drawText( QRect( 0, 0, currentIconSize.width(), currentIconSize.height() ), Qt::AlignCenter, QStringLiteral( "Random colors" ) );
    painter.end();
  }
  else
  {
    //create an icon pixmap
    if ( backgroundColorRamp )
    {
      pm = QgsSymbolLayerUtils::colorRampPreviewPixmap( backgroundColorRamp, currentIconSize );
    }
  }

  setIconSize( currentIconSize );
  setIcon( pm );
}

QgsColorRamp *QgsColorRampButton::colorRamp() const
{
  return mColorRamp ? mColorRamp->clone() : nullptr;
}

void QgsColorRampButton::setColorRampDialogTitle( const QString &title )
{
  mColorRampDialogTitle = title;
}

QString QgsColorRampButton::colorRampDialogTitle() const
{
  return mColorRampDialogTitle;
}

void QgsColorRampButton::setShowMenu( const bool showMenu )
{
  setMenu( showMenu ? mMenu : nullptr );
  setPopupMode( showMenu ? QToolButton::MenuButtonPopup : QToolButton::DelayedPopup );
  //force recalculation of icon size
  mIconSize = QSize();
  setButtonBackground( mColorRamp );
}

void QgsColorRampButton::setDefaultColorRamp( QgsColorRamp *colorramp )
{
  delete mDefaultColorRamp;
  mDefaultColorRamp = colorramp->clone();
}

bool QgsColorRampButton::isRandomColorRamp() const
{
  return dynamic_cast<QgsRandomColorRamp *>( mColorRamp );
}

void QgsColorRampButton::setShowNull( bool showNull )
{
  mShowNull = showNull;
}

bool QgsColorRampButton::showNull() const
{
  return mShowNull;
}

bool QgsColorRampButton::isNull() const
{
  return !mColorRamp;
}

void QgsColorRampButton::rampWidgetUpdated()
{
  QgsLimitedRandomColorRampWidget *limitedRampWidget = qobject_cast< QgsLimitedRandomColorRampWidget * >( sender() );
  if ( limitedRampWidget )
  {
    setColorRamp( limitedRampWidget->ramp().clone() );
    emit colorRampChanged();
    return;
  }
  QgsColorBrewerColorRampWidget *colorBrewerRampWidget = qobject_cast< QgsColorBrewerColorRampWidget * >( sender() );
  if ( colorBrewerRampWidget )
  {
    setColorRamp( colorBrewerRampWidget->ramp().clone() );
    emit colorRampChanged();
    return;
  }
  QgsPresetColorRampWidget *presetRampWidget = qobject_cast< QgsPresetColorRampWidget * >( sender() );
  if ( presetRampWidget )
  {
    setColorRamp( presetRampWidget->ramp().clone() );
    emit colorRampChanged();
    return;
  }
}
