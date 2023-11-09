/***************************************************************************
    qgscompoundcolorwidget.cpp
    --------------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscompoundcolorwidget.h"
#include "qgscolorscheme.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsscreenhelper.h"
#include "qgsguiutils.h"

#include <QHeaderView>
#include <QPushButton>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScreen>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QRegularExpression>

QgsCompoundColorWidget::QgsCompoundColorWidget( QWidget *parent, const QColor &color, Layout widgetLayout )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mScreenHelper = new QgsScreenHelper( this );

  connect( mHueRadio, &QRadioButton::toggled, this, &QgsCompoundColorWidget::mHueRadio_toggled );
  connect( mSaturationRadio, &QRadioButton::toggled, this, &QgsCompoundColorWidget::mSaturationRadio_toggled );
  connect( mValueRadio, &QRadioButton::toggled, this, &QgsCompoundColorWidget::mValueRadio_toggled );
  connect( mRedRadio, &QRadioButton::toggled, this, &QgsCompoundColorWidget::mRedRadio_toggled );
  connect( mGreenRadio, &QRadioButton::toggled, this, &QgsCompoundColorWidget::mGreenRadio_toggled );
  connect( mBlueRadio, &QRadioButton::toggled, this, &QgsCompoundColorWidget::mBlueRadio_toggled );
  connect( mAddColorToSchemeButton, &QPushButton::clicked, this, &QgsCompoundColorWidget::mAddColorToSchemeButton_clicked );
  connect( mAddCustomColorButton, &QPushButton::clicked, this, &QgsCompoundColorWidget::mAddCustomColorButton_clicked );
  connect( mSampleButton, &QPushButton::clicked, this, &QgsCompoundColorWidget::mSampleButton_clicked );
  connect( mTabWidget, &QTabWidget::currentChanged, this, &QgsCompoundColorWidget::mTabWidget_currentChanged );
  connect( mActionShowInButtons, &QAction::toggled, this, &QgsCompoundColorWidget::mActionShowInButtons_toggled );

  if ( widgetLayout == LayoutVertical )
  {
    // shuffle stuff around
    QVBoxLayout *newLayout = new QVBoxLayout();
    newLayout->setContentsMargins( 0, 0, 0, 0 );
    newLayout->addWidget( mTabWidget );
    newLayout->addWidget( mSlidersWidget );
    newLayout->addWidget( mPreviewWidget );
    newLayout->addWidget( mSwatchesWidget );
    delete layout();
    setLayout( newLayout );
  }

  const QgsSettings settings;

  mSchemeList->header()->hide();
  mSchemeList->setColumnWidth( 0, static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 6 ) );

  //get schemes with ShowInColorDialog set
  refreshSchemeComboBox();
  const QList<QgsColorScheme *> schemeList = QgsApplication::colorSchemeRegistry()->schemes( QgsColorScheme::ShowInColorDialog );

  //choose a reasonable starting scheme
  int activeScheme = settings.value( QStringLiteral( "Windows/ColorDialog/activeScheme" ), 0 ).toInt();
  activeScheme = activeScheme >= mSchemeComboBox->count() ? 0 : activeScheme;

  mSchemeList->setScheme( schemeList.at( activeScheme ) );

  mSchemeComboBox->setCurrentIndex( activeScheme );
  updateActionsForCurrentScheme();

  //listen out for selection changes in list, so we can enable/disable the copy colors option
  connect( mSchemeList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsCompoundColorWidget::listSelectionChanged );
  //copy action defaults to disabled
  mActionCopyColors->setEnabled( false );

  connect( mActionCopyColors, &QAction::triggered, mSchemeList, &QgsColorSchemeList::copyColors );
  connect( mActionPasteColors, &QAction::triggered, mSchemeList, &QgsColorSchemeList::pasteColors );
  connect( mActionExportColors, &QAction::triggered, mSchemeList, &QgsColorSchemeList::showExportColorsDialog );
  connect( mActionImportColors, &QAction::triggered, mSchemeList, &QgsColorSchemeList::showImportColorsDialog );
  connect( mActionImportPalette, &QAction::triggered, this, &QgsCompoundColorWidget::importPalette );
  connect( mActionRemovePalette, &QAction::triggered, this, &QgsCompoundColorWidget::removePalette );
  connect( mActionNewPalette, &QAction::triggered, this, &QgsCompoundColorWidget::newPalette );
  connect( mRemoveColorsFromSchemeButton, &QAbstractButton::clicked, mSchemeList, &QgsColorSchemeList::removeSelection );

  QMenu *schemeMenu = new QMenu( mSchemeToolButton );
  schemeMenu->addAction( mActionCopyColors );
  schemeMenu->addAction( mActionPasteColors );
  schemeMenu->addSeparator();
  schemeMenu->addAction( mActionImportColors );
  schemeMenu->addAction( mActionExportColors );
  schemeMenu->addSeparator();
  schemeMenu->addAction( mActionNewPalette );
  schemeMenu->addAction( mActionImportPalette );
  schemeMenu->addAction( mActionRemovePalette );
  schemeMenu->addAction( mActionShowInButtons );
  mSchemeToolButton->setMenu( schemeMenu );

  connect( mSchemeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsCompoundColorWidget::schemeIndexChanged );
  connect( mSchemeList, &QgsColorSchemeList::colorSelected, this, &QgsCompoundColorWidget::setColor );

  mOldColorLabel->hide();

  mVerticalRamp->setOrientation( QgsColorRampWidget::Vertical );
  mVerticalRamp->setInteriorMargin( 2 );
  mVerticalRamp->setShowFrame( true );

  mRedSlider->setComponent( QgsColorWidget::Red );
  mGreenSlider->setComponent( QgsColorWidget::Green );
  mBlueSlider->setComponent( QgsColorWidget::Blue );
  mHueSlider->setComponent( QgsColorWidget::Hue );
  mSaturationSlider->setComponent( QgsColorWidget::Saturation );
  mValueSlider->setComponent( QgsColorWidget::Value );
  mAlphaSlider->setComponent( QgsColorWidget::Alpha );

  mSwatchButton1->setShowMenu( false );
  mSwatchButton1->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton2->setShowMenu( false );
  mSwatchButton2->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton3->setShowMenu( false );
  mSwatchButton3->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton4->setShowMenu( false );
  mSwatchButton4->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton5->setShowMenu( false );
  mSwatchButton5->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton6->setShowMenu( false );
  mSwatchButton6->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton7->setShowMenu( false );
  mSwatchButton7->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton8->setShowMenu( false );
  mSwatchButton8->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton9->setShowMenu( false );
  mSwatchButton9->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton10->setShowMenu( false );
  mSwatchButton10->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton11->setShowMenu( false );
  mSwatchButton11->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton12->setShowMenu( false );
  mSwatchButton12->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton13->setShowMenu( false );
  mSwatchButton13->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton14->setShowMenu( false );
  mSwatchButton14->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton15->setShowMenu( false );
  mSwatchButton15->setBehavior( QgsColorButton::SignalOnly );
  mSwatchButton16->setShowMenu( false );
  mSwatchButton16->setBehavior( QgsColorButton::SignalOnly );
  //restore custom colors
  mSwatchButton1->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor1" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton2->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor2" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton3->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor3" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton4->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor4" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton5->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor5" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton6->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor6" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton7->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor7" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton8->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor8" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton9->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor9" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton10->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor10" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton11->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor11" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton12->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor12" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton13->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor13" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton14->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor14" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton15->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor15" ), QVariant( QColor() ) ).value<QColor>() );
  mSwatchButton16->setColor( settings.value( QStringLiteral( "Windows/ColorDialog/customColor16" ), QVariant( QColor() ) ).value<QColor>() );

  //restore sample radius
  mSpinBoxRadius->setValue( settings.value( QStringLiteral( "Windows/ColorDialog/sampleRadius" ), 1 ).toInt() );
  mSamplePreview->setColor( QColor() );

  // hidpi friendly sizes
  const int swatchWidth = static_cast< int >( std::round( std::max( Qgis::UI_SCALE_FACTOR * 1.9 * mSwatchButton1->fontMetrics().height(), 38.0 ) ) );
  const int swatchHeight = static_cast< int >( std::round( std::max( Qgis::UI_SCALE_FACTOR * 1.5 * mSwatchButton1->fontMetrics().height(), 30.0 ) ) );
  mSwatchButton1->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton1->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton2->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton2->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton3->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton3->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton4->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton4->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton5->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton5->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton6->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton6->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton7->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton7->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton8->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton8->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton9->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton9->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton10->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton10->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton11->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton11->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton12->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton12->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton13->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton13->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton14->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton14->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton15->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton15->setMaximumSize( swatchWidth, swatchHeight );
  mSwatchButton16->setMinimumSize( swatchWidth, swatchHeight );
  mSwatchButton16->setMaximumSize( swatchWidth, swatchHeight );
  const int previewHeight = static_cast< int >( std::round( std::max( Qgis::UI_SCALE_FACTOR * 2.0 * mSwatchButton1->fontMetrics().height(), 40.0 ) ) );
  mColorPreview->setMinimumSize( 0, previewHeight );
  mPreviewWidget->setMaximumHeight( previewHeight * 2 );
  const int swatchAddSize = static_cast< int >( std::round( std::max( Qgis::UI_SCALE_FACTOR * 1.4 * mSwatchButton1->fontMetrics().height(), 28.0 ) ) );
  mAddCustomColorButton->setMinimumWidth( swatchAddSize );
  mAddCustomColorButton->setMaximumWidth( swatchAddSize );

  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  mTabWidget->setIconSize( QSize( iconSize, iconSize ) );

  if ( color.isValid() )
  {
    setColor( color );
  }

  //restore active component radio button
  const int activeRadio = settings.value( QStringLiteral( "Windows/ColorDialog/activeComponent" ), 2 ).toInt();
  switch ( activeRadio )
  {
    case 0:
      mHueRadio->setChecked( true );
      break;
    case 1:
      mSaturationRadio->setChecked( true );
      break;
    case 2:
      mValueRadio->setChecked( true );
      break;
    case 3:
      mRedRadio->setChecked( true );
      break;
    case 4:
      mGreenRadio->setChecked( true );
      break;
    case 5:
      mBlueRadio->setChecked( true );
      break;
  }
  const int currentTab = settings.value( QStringLiteral( "Windows/ColorDialog/activeTab" ), 0 ).toInt();
  mTabWidget->setCurrentIndex( currentTab );

  //setup connections
  connect( mColorBox, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mColorWheel, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mColorText, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mVerticalRamp, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mRedSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mGreenSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mBlueSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mHueSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mValueSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mSaturationSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mAlphaSlider, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mColorPreview, &QgsColorWidget::colorChanged, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton1, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton2, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton3, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton4, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton5, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton6, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton7, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton8, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton9, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton10, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton11, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton12, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton13, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton14, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton15, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
  connect( mSwatchButton16, &QgsColorButton::colorClicked, this, &QgsCompoundColorWidget::setColor );
}

QgsCompoundColorWidget::~QgsCompoundColorWidget()
{
  if ( !mDiscarded )
  {
    QgsRecentColorScheme::addRecentColor( color() );
  }
}

QColor QgsCompoundColorWidget::color() const
{
  //all widgets should have the same color, so it shouldn't matter
  //which we fetch it from
  return mColorPreview->color();
}

void QgsCompoundColorWidget::setAllowOpacity( const bool allowOpacity )
{
  mAllowAlpha = allowOpacity;
  mAlphaLabel->setVisible( allowOpacity );
  mAlphaSlider->setVisible( allowOpacity );
  mColorText->setAllowOpacity( allowOpacity );
  if ( !allowOpacity )
  {
    mAlphaLayout->setContentsMargins( 0, 0, 0, 0 );
    mAlphaLayout->setSpacing( 0 );
  }
}

void QgsCompoundColorWidget::refreshSchemeComboBox()
{
  mSchemeComboBox->blockSignals( true );
  mSchemeComboBox->clear();
  const QList<QgsColorScheme *> schemeList = QgsApplication::colorSchemeRegistry()->schemes( QgsColorScheme::ShowInColorDialog );
  QList<QgsColorScheme *>::const_iterator schemeIt = schemeList.constBegin();
  for ( ; schemeIt != schemeList.constEnd(); ++schemeIt )
  {
    mSchemeComboBox->addItem( ( *schemeIt )->schemeName() );
  }
  mSchemeComboBox->blockSignals( false );
}


QgsUserColorScheme *QgsCompoundColorWidget::importUserPaletteFromFile( QWidget *parent )
{
  QgsSettings s;
  const QString lastDir = s.value( QStringLiteral( "/UI/lastGplPaletteDir" ), QDir::homePath() ).toString();
  const QString filePath = QFileDialog::getOpenFileName( parent, tr( "Select Palette File" ), lastDir, QStringLiteral( "GPL (*.gpl);;All files (*.*)" ) );
  if ( parent )
    parent->activateWindow();
  if ( filePath.isEmpty() )
  {
    return nullptr;
  }

  //check if file exists
  const QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( nullptr, tr( "Import Color Palette" ), tr( "Error, file does not exist or is not readable." ) );
    return nullptr;
  }

  s.setValue( QStringLiteral( "/UI/lastGplPaletteDir" ), fileInfo.absolutePath() );
  QFile file( filePath );

  QgsNamedColorList importedColors;
  bool ok = false;
  QString paletteName;
  importedColors = QgsSymbolLayerUtils::importColorsFromGpl( file, ok, paletteName );
  if ( !ok )
  {
    QMessageBox::critical( nullptr, tr( "Import Color Palette" ), tr( "Palette file is not readable." ) );
    return nullptr;
  }

  if ( importedColors.length() == 0 )
  {
    //no imported colors
    QMessageBox::critical( nullptr, tr( "Import Color Palette" ), tr( "No colors found in palette file." ) );
    return nullptr;
  }

  //TODO - handle conflicting file names, name for new palette
  QgsUserColorScheme *importedScheme = new QgsUserColorScheme( fileInfo.fileName() );
  importedScheme->setName( paletteName );
  importedScheme->setColors( importedColors );

  QgsApplication::colorSchemeRegistry()->addColorScheme( importedScheme );
  return importedScheme;
}

void QgsCompoundColorWidget::importPalette()
{
  if ( importUserPaletteFromFile( this ) )
  {
    //refresh combobox
    refreshSchemeComboBox();
    mSchemeComboBox->setCurrentIndex( mSchemeComboBox->count() - 1 );
  }
}


bool QgsCompoundColorWidget::removeUserPalette( QgsUserColorScheme *scheme, QWidget *parent )
{
  if ( QMessageBox::question( parent, tr( "Remove Color Palette" ),
                              tr( "Are you sure you want to remove %1?" ).arg( scheme->schemeName() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
  {
    //user canceled
    return false;
  }

  //remove palette and associated gpl file
  if ( !scheme->erase() )
  {
    //something went wrong
    return false;
  }

  //remove scheme from registry
  QgsApplication::colorSchemeRegistry()->removeColorScheme( scheme );
  return true;
}

void QgsCompoundColorWidget::removePalette()
{
  //get current scheme
  const QList<QgsColorScheme *> schemeList = QgsApplication::colorSchemeRegistry()->schemes( QgsColorScheme::ShowInColorDialog );
  int prevIndex = mSchemeComboBox->currentIndex();
  if ( prevIndex >= schemeList.length() )
  {
    return;
  }

  //make user scheme is a user removable scheme
  QgsUserColorScheme *userScheme = dynamic_cast<QgsUserColorScheme *>( schemeList.at( prevIndex ) );
  if ( !userScheme )
  {
    return;
  }

  if ( removeUserPalette( userScheme, this ) )
  {
    refreshSchemeComboBox();
    prevIndex = std::max( std::min( prevIndex, mSchemeComboBox->count() - 1 ), 0 );
    mSchemeComboBox->setCurrentIndex( prevIndex );
  }
}

QgsUserColorScheme *QgsCompoundColorWidget::createNewUserPalette( QWidget *parent )
{
  bool ok = false;
  const QString name = QInputDialog::getText( parent, tr( "Create New Palette" ), tr( "Enter a name for the new palette:" ),
                       QLineEdit::Normal, tr( "New palette" ), &ok );

  if ( !ok || name.isEmpty() )
  {
    //user canceled
    return nullptr;
  }

  //generate file name for new palette
  const QDir palettePath( gplFilePath() );
  const thread_local QRegularExpression badChars( "[,^@={}\\[\\]~!?:&*\"|#%<>$\"'();`' /\\\\]" );
  QString filename = name.simplified().toLower().replace( badChars, QStringLiteral( "_" ) );
  if ( filename.isEmpty() )
  {
    filename = tr( "new_palette" );
  }
  QFileInfo destFileInfo( palettePath.filePath( filename + ".gpl" ) );
  int fileNumber = 1;
  while ( destFileInfo.exists() )
  {
    //try to generate a unique file name
    destFileInfo = QFileInfo( palettePath.filePath( filename + QStringLiteral( "%1.gpl" ).arg( fileNumber ) ) );
    fileNumber++;
  }

  QgsUserColorScheme *newScheme = new QgsUserColorScheme( destFileInfo.fileName() );
  newScheme->setName( name );

  QgsApplication::colorSchemeRegistry()->addColorScheme( newScheme );
  return newScheme;
}

void QgsCompoundColorWidget::newPalette()
{
  if ( createNewUserPalette( this ) )
  {
    //refresh combobox and set new scheme as active
    refreshSchemeComboBox();
    mSchemeComboBox->setCurrentIndex( mSchemeComboBox->count() - 1 );
  }
}

QString QgsCompoundColorWidget::gplFilePath()
{
  QString palettesDir = QgsApplication::qgisSettingsDirPath() + "palettes";

  const QDir localDir;
  if ( !localDir.mkpath( palettesDir ) )
  {
    return QString();
  }

  return palettesDir;
}

void QgsCompoundColorWidget::schemeIndexChanged( int index )
{
  //save changes to scheme
  if ( mSchemeList->isDirty() )
  {
    mSchemeList->saveColorsToScheme();
  }

  //get schemes with ShowInColorDialog set
  const QList<QgsColorScheme *> schemeList = QgsApplication::colorSchemeRegistry()->schemes( QgsColorScheme::ShowInColorDialog );
  if ( index >= schemeList.length() )
  {
    return;
  }

  QgsColorScheme *scheme = schemeList.at( index );
  mSchemeList->setScheme( scheme );

  updateActionsForCurrentScheme();

  //copy action defaults to disabled
  mActionCopyColors->setEnabled( false );
}

void QgsCompoundColorWidget::listSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( deselected )
  mActionCopyColors->setEnabled( selected.length() > 0 );
}

void QgsCompoundColorWidget::mAddCustomColorButton_clicked()
{
  switch ( mLastCustomColorIndex )
  {
    case 0:
      mSwatchButton1->setColor( mColorPreview->color() );
      break;
    case 1:
      mSwatchButton2->setColor( mColorPreview->color() );
      break;
    case 2:
      mSwatchButton3->setColor( mColorPreview->color() );
      break;
    case 3:
      mSwatchButton4->setColor( mColorPreview->color() );
      break;
    case 4:
      mSwatchButton5->setColor( mColorPreview->color() );
      break;
    case 5:
      mSwatchButton6->setColor( mColorPreview->color() );
      break;
    case 6:
      mSwatchButton7->setColor( mColorPreview->color() );
      break;
    case 7:
      mSwatchButton8->setColor( mColorPreview->color() );
      break;
    case 8:
      mSwatchButton9->setColor( mColorPreview->color() );
      break;
    case 9:
      mSwatchButton10->setColor( mColorPreview->color() );
      break;
    case 10:
      mSwatchButton11->setColor( mColorPreview->color() );
      break;
    case 11:
      mSwatchButton12->setColor( mColorPreview->color() );
      break;
    case 12:
      mSwatchButton13->setColor( mColorPreview->color() );
      break;
    case 13:
      mSwatchButton14->setColor( mColorPreview->color() );
      break;
    case 14:
      mSwatchButton15->setColor( mColorPreview->color() );
      break;
    case 15:
      mSwatchButton16->setColor( mColorPreview->color() );
      break;
  }
  mLastCustomColorIndex++;
  if ( mLastCustomColorIndex >= 16 )
  {
    mLastCustomColorIndex = 0;
  }
}

void QgsCompoundColorWidget::mSampleButton_clicked()
{
  //activate picker color
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Sampler ) );
  grabMouse();
  grabKeyboard();
  mPickingColor = true;
  setMouseTracking( true );
}

void QgsCompoundColorWidget::mTabWidget_currentChanged( int index )
{
  //disable radio buttons if not using the first tab, as they have no meaning for other tabs
  const bool enabled = index == 0;
  mRedRadio->setEnabled( enabled );
  mBlueRadio->setEnabled( enabled );
  mGreenRadio->setEnabled( enabled );
  mHueRadio->setEnabled( enabled );
  mSaturationRadio->setEnabled( enabled );
  mValueRadio->setEnabled( enabled );
}

void QgsCompoundColorWidget::mActionShowInButtons_toggled( bool state )
{
  QgsUserColorScheme *scheme = dynamic_cast< QgsUserColorScheme * >( mSchemeList->scheme() );
  if ( scheme )
  {
    scheme->setShowSchemeInMenu( state );
  }
}

QScreen *QgsCompoundColorWidget::findScreenAt( QPoint pos )
{
  const QList< QScreen * > screens = QGuiApplication::screens();
  for ( QScreen *screen : screens )
  {
    if ( screen->geometry().contains( pos ) )
    {
      return screen;
    }
  }
  return nullptr;
}

void QgsCompoundColorWidget::saveSettings()
{
  //save changes to scheme
  if ( mSchemeList->isDirty() )
  {
    mSchemeList->saveColorsToScheme();
  }

  QgsSettings settings;

  //record active component
  int activeRadio = 0;
  if ( mHueRadio->isChecked() )
    activeRadio = 0;
  if ( mSaturationRadio->isChecked() )
    activeRadio = 1;
  if ( mValueRadio->isChecked() )
    activeRadio = 2;
  if ( mRedRadio->isChecked() )
    activeRadio = 3;
  if ( mGreenRadio->isChecked() )
    activeRadio = 4;
  if ( mBlueRadio->isChecked() )
    activeRadio = 5;
  settings.setValue( QStringLiteral( "Windows/ColorDialog/activeComponent" ), activeRadio );

  //record current scheme
  settings.setValue( QStringLiteral( "Windows/ColorDialog/activeScheme" ), mSchemeComboBox->currentIndex() );

  //record current tab
  settings.setValue( QStringLiteral( "Windows/ColorDialog/activeTab" ), mTabWidget->currentIndex() );

  //record custom colors
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor1" ), QVariant( mSwatchButton1->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor2" ), QVariant( mSwatchButton2->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor3" ), QVariant( mSwatchButton3->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor4" ), QVariant( mSwatchButton4->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor5" ), QVariant( mSwatchButton5->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor6" ), QVariant( mSwatchButton6->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor7" ), QVariant( mSwatchButton7->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor8" ), QVariant( mSwatchButton8->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor9" ), QVariant( mSwatchButton9->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor10" ), QVariant( mSwatchButton10->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor11" ), QVariant( mSwatchButton11->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor12" ), QVariant( mSwatchButton12->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor13" ), QVariant( mSwatchButton13->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor14" ), QVariant( mSwatchButton14->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor15" ), QVariant( mSwatchButton15->color() ) );
  settings.setValue( QStringLiteral( "Windows/ColorDialog/customColor16" ), QVariant( mSwatchButton16->color() ) );

  //sample radius
  settings.setValue( QStringLiteral( "Windows/ColorDialog/sampleRadius" ), mSpinBoxRadius->value() );
}

void QgsCompoundColorWidget::stopPicking( QPoint eventPos, const bool takeSample )
{
  //release mouse and keyboard, and reset cursor
  releaseMouse();
  releaseKeyboard();
  unsetCursor();
  setMouseTracking( false );
  mPickingColor = false;

  if ( !takeSample )
  {
    //not sampling color, nothing more to do
    return;
  }

  //grab snapshot of pixel under mouse cursor
  const QColor snappedColor = sampleColor( eventPos );
  mSamplePreview->setColor( snappedColor );
  mColorPreview->setColor( snappedColor, true );
}

void QgsCompoundColorWidget::setColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    return;
  }

  QColor fixedColor = QColor( color );
  if ( !mAllowAlpha )
  {
    //opacity disallowed, so don't permit transparent colors
    fixedColor.setAlpha( 255 );
  }
  const QList<QgsColorWidget *> colorWidgets = this->findChildren<QgsColorWidget *>();
  const auto constColorWidgets = colorWidgets;
  for ( QgsColorWidget *widget : constColorWidgets )
  {
    if ( widget == mSamplePreview )
    {
      continue;
    }
    widget->blockSignals( true );
    widget->setColor( fixedColor );
    widget->blockSignals( false );
  }
  emit currentColorChanged( fixedColor );
}

void QgsCompoundColorWidget::setPreviousColor( const QColor &color )
{
  mOldColorLabel->setVisible( color.isValid() );
  mColorPreview->setColor2( color );
}

void QgsCompoundColorWidget::hideEvent( QHideEvent *e )
{
  saveSettings();
  QWidget::hideEvent( e );
}

void QgsCompoundColorWidget::mousePressEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //don't show dialog if in color picker mode
    e->accept();
    return;
  }

  QWidget::mousePressEvent( e );
}

QColor QgsCompoundColorWidget::averageColor( const QImage &image ) const
{
  QRgb tmpRgb;
  int colorCount = 0;
  int sumRed = 0;
  int sumBlue = 0;
  int sumGreen = 0;
  //scan through image and sum rgb components
  for ( int heightIndex = 0; heightIndex < image.height(); ++heightIndex )
  {
    const QRgb *scanLine = reinterpret_cast< const QRgb * >( image.constScanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < image.width(); ++widthIndex )
    {
      tmpRgb = scanLine[widthIndex];
      sumRed += qRed( tmpRgb );
      sumBlue += qBlue( tmpRgb );
      sumGreen += qGreen( tmpRgb );
      colorCount++;
    }
  }
  //calculate average components as floats
  const double avgRed = static_cast<double>( sumRed ) / ( 255.0 * colorCount );
  const double avgGreen = static_cast<double>( sumGreen ) / ( 255.0 * colorCount );
  const double avgBlue = static_cast<double>( sumBlue ) / ( 255.0 * colorCount );

  //create a new color representing the average
  return QColor::fromRgbF( avgRed, avgGreen, avgBlue );
}

QColor QgsCompoundColorWidget::sampleColor( QPoint point ) const
{
  const int sampleRadius = mSpinBoxRadius->value() - 1;
  QScreen *screen = findScreenAt( point );
  if ( ! screen )
  {
    return QColor();
  }

  const int x = point.x() - screen->geometry().left();
  const int y = point.y() - screen->geometry().top();
  const QPixmap snappedPixmap = screen->grabWindow( 0,
                                x - sampleRadius,
                                y - sampleRadius,
                                1 + sampleRadius * 2,
                                1 + sampleRadius * 2 );
  const QImage snappedImage = snappedPixmap.toImage();
  //scan all pixels and take average color
  return averageColor( snappedImage );
}

void QgsCompoundColorWidget::mouseMoveEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //currently in color picker mode
    //sample color under cursor update preview widget to give feedback to user
    const QColor hoverColor = sampleColor( e->globalPos() );
    mSamplePreview->setColor( hoverColor );

    e->accept();
    return;
  }

  QWidget::mouseMoveEvent( e );
}

void QgsCompoundColorWidget::mouseReleaseEvent( QMouseEvent *e )
{
  if ( mPickingColor )
  {
    //end color picking operation by sampling the color under cursor
    stopPicking( e->globalPos() );
    e->accept();
    return;
  }

  QWidget::mouseReleaseEvent( e );
}

void QgsCompoundColorWidget::keyPressEvent( QKeyEvent *e )
{
  if ( !mPickingColor )
  {
    //if not picking a color, use default tool button behavior
    QgsPanelWidget::keyPressEvent( e );
    return;
  }

  //cancel picking, sampling the color if space was pressed
  stopPicking( QCursor::pos(), e->key() == Qt::Key_Space );
}

void QgsCompoundColorWidget::mHueRadio_toggled( bool checked )
{
  if ( checked )
  {
    mColorBox->setComponent( QgsColorWidget::Hue );
    mVerticalRamp->setComponent( QgsColorWidget::Hue );
  }
}

void QgsCompoundColorWidget::mSaturationRadio_toggled( bool checked )
{
  if ( checked )
  {
    mColorBox->setComponent( QgsColorWidget::Saturation );
    mVerticalRamp->setComponent( QgsColorWidget::Saturation );
  }
}

void QgsCompoundColorWidget::mValueRadio_toggled( bool checked )
{
  if ( checked )
  {
    mColorBox->setComponent( QgsColorWidget::Value );
    mVerticalRamp->setComponent( QgsColorWidget::Value );
  }
}

void QgsCompoundColorWidget::mRedRadio_toggled( bool checked )
{
  if ( checked )
  {
    mColorBox->setComponent( QgsColorWidget::Red );
    mVerticalRamp->setComponent( QgsColorWidget::Red );
  }
}

void QgsCompoundColorWidget::mGreenRadio_toggled( bool checked )
{
  if ( checked )
  {
    mColorBox->setComponent( QgsColorWidget::Green );
    mVerticalRamp->setComponent( QgsColorWidget::Green );
  }
}

void QgsCompoundColorWidget::mBlueRadio_toggled( bool checked )
{
  if ( checked )
  {
    mColorBox->setComponent( QgsColorWidget::Blue );
    mVerticalRamp->setComponent( QgsColorWidget::Blue );
  }
}

void QgsCompoundColorWidget::mAddColorToSchemeButton_clicked()
{
  mSchemeList->addColor( mColorPreview->color(), QgsSymbolLayerUtils::colorToName( mColorPreview->color() ) );
}

void QgsCompoundColorWidget::updateActionsForCurrentScheme()
{
  QgsColorScheme *scheme = mSchemeList->scheme();

  mActionImportColors->setEnabled( scheme->isEditable() );
  mActionPasteColors->setEnabled( scheme->isEditable() );
  mAddColorToSchemeButton->setEnabled( scheme->isEditable() );
  mRemoveColorsFromSchemeButton->setEnabled( scheme->isEditable() );

  QgsUserColorScheme *userScheme = dynamic_cast<QgsUserColorScheme *>( scheme );
  mActionRemovePalette->setEnabled( static_cast< bool >( userScheme ) );
  if ( userScheme )
  {
    mActionShowInButtons->setEnabled( true );
    whileBlocking( mActionShowInButtons )->setChecked( userScheme->flags() & QgsColorScheme::ShowInColorButtonMenu );
  }
  else
  {
    whileBlocking( mActionShowInButtons )->setChecked( false );
    mActionShowInButtons->setEnabled( false );
  }
}
