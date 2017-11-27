/***************************************************************************
                         qgscomposerpicturewidget.cpp
                         ----------------------------
    begin                : August 13, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerpicturewidget.h"
#include "qgsapplication.h"
#include "qgscomposermap.h"
#include "qgscomposerpicture.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposition.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgssvgcache.h"
#include "qgssettings.h"

#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QSvgRenderer>

QgsComposerPictureWidget::QgsComposerPictureWidget( QgsComposerPicture *picture ): QgsComposerItemBaseWidget( nullptr, picture ), mPicture( picture ), mPreviewsLoaded( false )
{
  setupUi( this );
  connect( mPictureBrowseButton, &QPushButton::clicked, this, &QgsComposerPictureWidget::mPictureBrowseButton_clicked );
  connect( mPictureLineEdit, &QLineEdit::editingFinished, this, &QgsComposerPictureWidget::mPictureLineEdit_editingFinished );
  connect( mPictureRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerPictureWidget::mPictureRotationSpinBox_valueChanged );
  connect( mPreviewListWidget, &QListWidget::currentItemChanged, this, &QgsComposerPictureWidget::mPreviewListWidget_currentItemChanged );
  connect( mAddDirectoryButton, &QPushButton::clicked, this, &QgsComposerPictureWidget::mAddDirectoryButton_clicked );
  connect( mRemoveDirectoryButton, &QPushButton::clicked, this, &QgsComposerPictureWidget::mRemoveDirectoryButton_clicked );
  connect( mRotationFromComposerMapCheckBox, &QCheckBox::stateChanged, this, &QgsComposerPictureWidget::mRotationFromComposerMapCheckBox_stateChanged );
  connect( mResizeModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerPictureWidget::mResizeModeComboBox_currentIndexChanged );
  connect( mAnchorPointComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerPictureWidget::mAnchorPointComboBox_currentIndexChanged );
  connect( mFillColorButton, &QgsColorButton::colorChanged, this, &QgsComposerPictureWidget::mFillColorButton_colorChanged );
  connect( mStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsComposerPictureWidget::mStrokeColorButton_colorChanged );
  connect( mStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerPictureWidget::mStrokeWidthSpinBox_valueChanged );
  connect( mPictureRotationOffsetSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerPictureWidget::mPictureRotationOffsetSpinBox_valueChanged );
  connect( mNorthTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerPictureWidget::mNorthTypeComboBox_currentIndexChanged );
  setPanelTitle( tr( "Picture properties" ) );

  mFillColorButton->setAllowOpacity( true );
  mFillColorButton->setColorDialogTitle( tr( "Select Fill Color" ) );
  mFillColorButton->setContext( QStringLiteral( "composer" ) );
  mStrokeColorButton->setAllowOpacity( true );
  mStrokeColorButton->setColorDialogTitle( tr( "Select Stroke Color" ) );
  mStrokeColorButton->setContext( QStringLiteral( "composer" ) );

  mNorthTypeComboBox->blockSignals( true );
  mNorthTypeComboBox->addItem( tr( "Grid north" ), QgsComposerPicture::GridNorth );
  mNorthTypeComboBox->addItem( tr( "True north" ), QgsComposerPicture::TrueNorth );
  mNorthTypeComboBox->blockSignals( false );
  mPictureRotationOffsetSpinBox->setClearValue( 0.0 );
  mPictureRotationSpinBox->setClearValue( 0.0 );

  //add widget for general composer item properties
  QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, picture );
  mainLayout->addWidget( itemPropertiesWidget );

  if ( mPicture->composition() )
  {
    mComposerMapComboBox->setComposition( mPicture->composition() );
    mComposerMapComboBox->setItemType( QgsComposerItem::ComposerMap );
    connect( mComposerMapComboBox, &QgsComposerItemComboBox::itemChanged, this, &QgsComposerPictureWidget::composerMapChanged );
  }

  setGuiElementValues();
  mPreviewsLoadingLabel->hide();

  mPreviewListWidget->setIconSize( QSize( 30, 30 ) );

  // mSearchDirectoriesGroupBox is a QgsCollapsibleGroupBoxBasic, so its collapsed state should not be saved/restored
  mSearchDirectoriesGroupBox->setCollapsed( true );
  // setup connection for loading previews on first expansion of group box
  connect( mSearchDirectoriesGroupBox, &QgsCollapsibleGroupBoxBasic::collapsedStateChanged, this, &QgsComposerPictureWidget::loadPicturePreviews );

  connect( mPicture, &QgsComposerObject::itemChanged, this, &QgsComposerPictureWidget::setGuiElementValues );
  connect( mPicture, &QgsComposerPicture::pictureRotationChanged, this, &QgsComposerPictureWidget::setPicRotationSpinValue );

  //connections for data defined buttons
  connect( mSourceDDBtn, &QgsPropertyOverrideButton::activated, mPictureLineEdit, &QLineEdit::setDisabled );
  registerDataDefinedButton( mSourceDDBtn, QgsComposerObject::PictureSource );
  registerDataDefinedButton( mFillColorDDBtn, QgsComposerObject::PictureSvgBackgroundColor );
  registerDataDefinedButton( mStrokeColorDDBtn, QgsComposerObject::PictureSvgStrokeColor );
  registerDataDefinedButton( mStrokeWidthDDBtn, QgsComposerObject::PictureSvgStrokeWidth );
}

void QgsComposerPictureWidget::mPictureBrowseButton_clicked()
{
  QgsSettings s;
  QString openDir;
  QString lineEditText = mPictureLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( QStringLiteral( "/UI/lastComposerPictureDir" ), QDir::homePath() ).toString();
  }

  //show file dialog
  QString filePath = QFileDialog::getOpenFileName( this, tr( "Select svg or image file" ), openDir );
  if ( filePath.isEmpty() )
  {
    return;
  }

  //check if file exists
  QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( nullptr, QStringLiteral( "Invalid file" ), QStringLiteral( "Error, file does not exist or is not readable" ) );
    return;
  }

  s.setValue( QStringLiteral( "/UI/lastComposerPictureDir" ), fileInfo.absolutePath() );

  mPictureLineEdit->blockSignals( true );
  mPictureLineEdit->setText( filePath );
  mPictureLineEdit->blockSignals( false );
  updateSvgParamGui();

  //pass file path to QgsComposerPicture
  if ( mPicture )
  {
    mPicture->beginCommand( tr( "Picture changed" ) );
    mPicture->setPicturePath( filePath );
    mPicture->update();
    mPicture->endCommand();
  }
}

void QgsComposerPictureWidget::mPictureLineEdit_editingFinished()
{
  if ( mPicture )
  {
    QString filePath = mPictureLineEdit->text();

    mPicture->beginCommand( tr( "Picture changed" ) );
    mPicture->setPicturePath( filePath );
    mPicture->update();
    mPicture->endCommand();
    updateSvgParamGui();
  }
}

void QgsComposerPictureWidget::mPictureRotationSpinBox_valueChanged( double d )
{
  if ( mPicture )
  {
    mPicture->beginCommand( tr( "Picture rotation changed" ), QgsComposerMergeCommand::ComposerPictureRotation );
    mPicture->setPictureRotation( d );
    mPicture->endCommand();
  }
}

void QgsComposerPictureWidget::mPreviewListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous )
{
  Q_UNUSED( previous );
  if ( !mPicture || !current )
  {
    return;
  }

  QString absoluteFilePath = current->data( Qt::UserRole ).toString();
  mPicture->beginCommand( tr( "Picture changed" ) );
  mPicture->setPicturePath( absoluteFilePath );
  mPictureLineEdit->setText( absoluteFilePath );
  mPicture->update();
  mPicture->endCommand();
  updateSvgParamGui();
}

void QgsComposerPictureWidget::mAddDirectoryButton_clicked()
{
  //let user select a directory
  QString directory = QFileDialog::getExistingDirectory( this, tr( "Select new preview directory" ) );
  if ( directory.isNull() )
  {
    return; //dialog canceled by user
  }

  //add entry to mSearchDirectoriesComboBox
  mSearchDirectoriesComboBox->addItem( directory );

  //and add icons to the preview
  addDirectoryToPreview( directory );

  //update the image directory list in the settings
  QgsSettings s;
  QStringList userDirList = s.value( QStringLiteral( "/Composer/PictureWidgetDirectories" ) ).toStringList();
  if ( !userDirList.contains( directory ) )
  {
    userDirList.append( directory );
  }
  s.setValue( QStringLiteral( "/Composer/PictureWidgetDirectories" ), userDirList );
}

void QgsComposerPictureWidget::mRemoveDirectoryButton_clicked()
{
  QString directoryToRemove = mSearchDirectoriesComboBox->currentText();
  if ( directoryToRemove.isEmpty() )
  {
    return;
  }
  mSearchDirectoriesComboBox->removeItem( mSearchDirectoriesComboBox->currentIndex() );

  //remove entries from back to front (to have the indices of existing items constant)
  for ( int i = ( mPreviewListWidget->count() - 1 ); i >= 0; --i )
  {
    QListWidgetItem *currentItem = mPreviewListWidget->item( i );
    if ( currentItem && currentItem->data( Qt::UserRole ).toString().startsWith( directoryToRemove ) )
    {
      delete ( mPreviewListWidget->takeItem( i ) );
    }
  }

  //update the image directory list in the settings
  QgsSettings s;
  QStringList userDirList = s.value( QStringLiteral( "/Composer/PictureWidgetDirectories" ) ).toStringList();
  userDirList.removeOne( directoryToRemove );
  s.setValue( QStringLiteral( "/Composer/PictureWidgetDirectories" ), userDirList );
}

void QgsComposerPictureWidget::mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Picture resize mode changed" ) );
  mPicture->setResizeMode( ( QgsComposerPicture::ResizeMode )index );
  mPicture->endCommand();

  //disable picture rotation for non-zoom modes
  mRotationGroupBox->setEnabled( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
                                 mPicture->resizeMode() == QgsComposerPicture::ZoomResizeFrame );

  //disable anchor point control for certain zoom modes
  if ( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
       mPicture->resizeMode() == QgsComposerPicture::Clip )
  {
    mAnchorPointComboBox->setEnabled( true );
  }
  else
  {
    mAnchorPointComboBox->setEnabled( false );
  }
}

void QgsComposerPictureWidget::mAnchorPointComboBox_currentIndexChanged( int index )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Picture placement changed" ) );
  mPicture->setPictureAnchor( ( QgsComposerItem::ItemPositionMode )index );
  mPicture->endCommand();
}

void QgsComposerPictureWidget::mRotationFromComposerMapCheckBox_stateChanged( int state )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Rotation synchronisation toggled" ) );
  if ( state == Qt::Unchecked )
  {
    mPicture->setRotationMap( -1 );
    mPictureRotationSpinBox->setEnabled( true );
    mComposerMapComboBox->setEnabled( false );
    mNorthTypeComboBox->setEnabled( false );
    mPictureRotationOffsetSpinBox->setEnabled( false );
    mPicture->setPictureRotation( mPictureRotationSpinBox->value() );
  }
  else
  {
    const QgsComposerMap *map = dynamic_cast< const QgsComposerMap * >( mComposerMapComboBox->currentItem() );
    int mapId = map ? map->id() : -1;
    mPicture->setRotationMap( mapId );
    mPictureRotationSpinBox->setEnabled( false );
    mNorthTypeComboBox->setEnabled( true );
    mPictureRotationOffsetSpinBox->setEnabled( true );
    mComposerMapComboBox->setEnabled( true );
  }
  mPicture->endCommand();
}

void QgsComposerPictureWidget::composerMapChanged( QgsComposerItem *item )
{
  if ( !mPicture )
  {
    return;
  }

  //get composition
  const QgsComposition *composition = mPicture->composition();
  if ( !composition )
  {
    return;
  }

  QgsComposerMap *composerMap = dynamic_cast< QgsComposerMap *>( item );
  int id = composerMap ? composerMap->id() : -1;
  if ( !composerMap )
  {
    return;
  }
  mPicture->beginCommand( tr( "Rotation map changed" ) );
  mPicture->setRotationMap( id );
  mPicture->update();
  mPicture->endCommand();
}

void QgsComposerPictureWidget::setPicRotationSpinValue( double r )
{
  mPictureRotationSpinBox->blockSignals( true );
  mPictureRotationSpinBox->setValue( r );
  mPictureRotationSpinBox->blockSignals( false );
}

void QgsComposerPictureWidget::setGuiElementValues()
{
  //set initial gui values
  if ( mPicture )
  {
    mPictureRotationSpinBox->blockSignals( true );
    mPictureLineEdit->blockSignals( true );
    mComposerMapComboBox->blockSignals( true );
    mRotationFromComposerMapCheckBox->blockSignals( true );
    mNorthTypeComboBox->blockSignals( true );
    mPictureRotationOffsetSpinBox->blockSignals( true );
    mResizeModeComboBox->blockSignals( true );
    mAnchorPointComboBox->blockSignals( true );
    mFillColorButton->blockSignals( true );
    mStrokeColorButton->blockSignals( true );
    mStrokeWidthSpinBox->blockSignals( true );

    mPictureLineEdit->setText( mPicture->picturePath() );
    mPictureRotationSpinBox->setValue( mPicture->pictureRotation() );

    const QgsComposerMap *map = mPicture->composition()->getComposerMapById( mPicture->rotationMap() );
    if ( map )
      mComposerMapComboBox->setItem( map );
    else
      mComposerMapComboBox->setCurrentIndex( 0 );

    if ( mPicture->useRotationMap() )
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Checked );
      mPictureRotationSpinBox->setEnabled( false );
      mComposerMapComboBox->setEnabled( true );
      mNorthTypeComboBox->setEnabled( true );
      mPictureRotationOffsetSpinBox->setEnabled( true );
    }
    else
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Unchecked );
      mPictureRotationSpinBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( false );
      mNorthTypeComboBox->setEnabled( false );
      mPictureRotationOffsetSpinBox->setEnabled( false );
    }
    mNorthTypeComboBox->setCurrentIndex( mNorthTypeComboBox->findData( mPicture->northMode() ) );
    mPictureRotationOffsetSpinBox->setValue( mPicture->northOffset() );

    mResizeModeComboBox->setCurrentIndex( ( int )mPicture->resizeMode() );
    //disable picture rotation for non-zoom modes
    mRotationGroupBox->setEnabled( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
                                   mPicture->resizeMode() == QgsComposerPicture::ZoomResizeFrame );

    mAnchorPointComboBox->setCurrentIndex( ( int )mPicture->pictureAnchor() );
    //disable anchor point control for certain zoom modes
    if ( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
         mPicture->resizeMode() == QgsComposerPicture::Clip )
    {
      mAnchorPointComboBox->setEnabled( true );
    }
    else
    {
      mAnchorPointComboBox->setEnabled( false );
    }

    updateSvgParamGui( false );
    mFillColorButton->setColor( mPicture->svgFillColor() );
    mStrokeColorButton->setColor( mPicture->svgStrokeColor() );
    mStrokeWidthSpinBox->setValue( mPicture->svgStrokeWidth() );

    mRotationFromComposerMapCheckBox->blockSignals( false );
    mPictureRotationSpinBox->blockSignals( false );
    mPictureLineEdit->blockSignals( false );
    mComposerMapComboBox->blockSignals( false );
    mNorthTypeComboBox->blockSignals( false );
    mPictureRotationOffsetSpinBox->blockSignals( false );
    mResizeModeComboBox->blockSignals( false );
    mAnchorPointComboBox->blockSignals( false );
    mFillColorButton->blockSignals( false );
    mStrokeColorButton->blockSignals( false );
    mStrokeWidthSpinBox->blockSignals( false );

    populateDataDefinedButtons();
  }
}

QIcon QgsComposerPictureWidget::svgToIcon( const QString &filePath ) const
{
  QColor fill, stroke;
  double strokeWidth, fillOpacity, strokeOpacity;
  bool fillParam, fillOpacityParam, strokeParam, strokeWidthParam, strokeOpacityParam;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultStrokeColor = false,
       hasDefaultStrokeWidth = false, hasDefaultStrokeOpacity = false;
  QgsApplication::svgCache()->containsParams( filePath, fillParam, hasDefaultFillColor, fill,
      fillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      strokeParam, hasDefaultStrokeColor, stroke,
      strokeWidthParam, hasDefaultStrokeWidth, strokeWidth,
      strokeOpacityParam, hasDefaultStrokeOpacity, strokeOpacity );

  //if defaults not set in symbol, use these values
  if ( !hasDefaultFillColor )
    fill = QColor( 200, 200, 200 );
  fill.setAlphaF( hasDefaultFillOpacity ? fillOpacity : 1.0 );
  if ( !hasDefaultStrokeColor )
    stroke = Qt::black;
  stroke.setAlphaF( hasDefaultStrokeOpacity ? strokeOpacity : 1.0 );
  if ( !hasDefaultStrokeWidth )
    strokeWidth = 0.6;

  bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
  QImage img = QgsApplication::svgCache()->svgAsImage( filePath, 30.0, fill, stroke, strokeWidth, 3.5 /*appr. 88 dpi*/, fitsInCache );

  return QIcon( QPixmap::fromImage( img ) );
}

void QgsComposerPictureWidget::updateSvgParamGui( bool resetValues )
{
  if ( !mPicture )
    return;

  QString picturePath = mPicture->picturePath();
  if ( !picturePath.endsWith( QLatin1String( ".svg" ), Qt::CaseInsensitive ) )
  {
    mFillColorButton->setEnabled( false );
    mStrokeColorButton->setEnabled( false );
    mStrokeWidthSpinBox->setEnabled( false );
    return;
  }

  //activate gui for svg parameters only if supported by the svg file
  bool hasFillParam, hasFillOpacityParam, hasStrokeParam, hasStrokeWidthParam, hasStrokeOpacityParam;
  QColor defaultFill, defaultStroke;
  double defaultStrokeWidth, defaultFillOpacity, defaultStrokeOpacity;
  bool hasDefaultFillColor, hasDefaultFillOpacity, hasDefaultStrokeColor, hasDefaultStrokeWidth, hasDefaultStrokeOpacity;
  QgsApplication::svgCache()->containsParams( picturePath, hasFillParam, hasDefaultFillColor, defaultFill,
      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
      hasStrokeParam, hasDefaultStrokeColor, defaultStroke,
      hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
      hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );

  if ( resetValues )
  {
    QColor fill = mFillColorButton->color();
    double newOpacity = hasFillOpacityParam ? fill.alphaF() : 1.0;
    if ( hasDefaultFillColor )
    {
      fill = defaultFill;
    }
    fill.setAlphaF( hasDefaultFillOpacity ? defaultFillOpacity : newOpacity );
    mFillColorButton->setColor( fill );
  }
  mFillColorButton->setEnabled( hasFillParam );
  mFillColorButton->setAllowOpacity( hasFillOpacityParam );
  if ( resetValues )
  {
    QColor stroke = mStrokeColorButton->color();
    double newOpacity = hasStrokeOpacityParam ? stroke.alphaF() : 1.0;
    if ( hasDefaultStrokeColor )
    {
      stroke = defaultStroke;
    }
    stroke.setAlphaF( hasDefaultStrokeOpacity ? defaultStrokeOpacity : newOpacity );
    mStrokeColorButton->setColor( stroke );
  }
  mStrokeColorButton->setEnabled( hasStrokeParam );
  mStrokeColorButton->setAllowOpacity( hasStrokeOpacityParam );
  if ( hasDefaultStrokeWidth && resetValues )
  {
    mStrokeWidthSpinBox->setValue( defaultStrokeWidth );
  }
  mStrokeWidthSpinBox->setEnabled( hasStrokeWidthParam );
}

int QgsComposerPictureWidget::addDirectoryToPreview( const QString &path )
{
  //go through all files of a directory
  QDir directory( path );
  if ( !directory.exists() || !directory.isReadable() )
  {
    return 1; //error
  }

  QFileInfoList fileList = directory.entryInfoList( QDir::Files );
  QFileInfoList::const_iterator fileIt = fileList.constBegin();

  QProgressDialog progress( QStringLiteral( "Adding Icons..." ), QStringLiteral( "Abort" ), 0, fileList.size() - 1, this );
  //cancel button does not seem to work properly with modal dialog
  //progress.setWindowModality(Qt::WindowModal);

  int counter = 0;
  for ( ; fileIt != fileList.constEnd(); ++fileIt )
  {

    progress.setLabelText( tr( "Creating icon for file %1" ).arg( fileIt->fileName() ) );
    progress.setValue( counter );
    QCoreApplication::processEvents();
    if ( progress.wasCanceled() )
    {
      break;
    }
    QString filePath = fileIt->absoluteFilePath();

    //test if file is svg or pixel format
    bool fileIsPixel = false;
    bool fileIsSvg = testSvgFile( filePath );
    if ( !fileIsSvg )
    {
      fileIsPixel = testImageFile( filePath );
    }

    //exclude files that are not svg or image
    if ( !fileIsSvg && !fileIsPixel )
    {
      ++counter;
      continue;
    }

    QListWidgetItem *listItem = new QListWidgetItem( mPreviewListWidget );
    listItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    if ( fileIsSvg )
    {
      // render SVG file
      QIcon icon = svgToIcon( filePath );
      listItem->setIcon( icon );
    }
    else //for pixel formats: create icon from scaled pixmap
    {
      QPixmap iconPixmap( filePath );
      if ( iconPixmap.isNull() )
      {
        ++counter;
        continue; //unknown file format or other problem
      }
      //set pixmap hardcoded to 30/30, same as icon size for mPreviewListWidget
      QPixmap scaledPixmap( iconPixmap.scaled( QSize( 30, 30 ), Qt::KeepAspectRatio ) );
      QIcon icon( scaledPixmap );
      listItem->setIcon( icon );
    }

    listItem->setText( QString() );
    //store the absolute icon file path as user data
    listItem->setData( Qt::UserRole, fileIt->absoluteFilePath() );
    ++counter;
  }

  return 0;
}

void QgsComposerPictureWidget::addStandardDirectoriesToPreview()
{
  mPreviewListWidget->clear();

  //list all directories in $prefix/share/qgis/svg
  QStringList svgPaths = QgsApplication::svgPaths();
  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir svgDirectory( svgPaths[i] );
    if ( !svgDirectory.exists() || !svgDirectory.isReadable() )
    {
      continue;
    }

    //add directory itself
    mSearchDirectoriesComboBox->addItem( svgDirectory.absolutePath() );
    addDirectoryToPreview( svgDirectory.absolutePath() );

    //and also subdirectories
    QFileInfoList directoryList = svgDirectory.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );
    QFileInfoList::const_iterator dirIt = directoryList.constBegin();
    for ( ; dirIt != directoryList.constEnd(); ++dirIt )
    {
      if ( addDirectoryToPreview( dirIt->absoluteFilePath() ) == 0 )
      {
        mSearchDirectoriesComboBox->addItem( dirIt->absoluteFilePath() );
      }
    }
  }

  //include additional user-defined directories for images
  QgsSettings s;
  QStringList userDirList = s.value( QStringLiteral( "/Composer/PictureWidgetDirectories" ) ).toStringList();
  QStringList::const_iterator userDirIt = userDirList.constBegin();
  for ( ; userDirIt != userDirList.constEnd(); ++userDirIt )
  {
    addDirectoryToPreview( *userDirIt );
    mSearchDirectoriesComboBox->addItem( *userDirIt );
  }

  mPreviewsLoaded = true;
}

bool QgsComposerPictureWidget::testSvgFile( const QString &filename ) const
{
  //QSvgRenderer crashes with some (non-svg) xml documents.
  //So at least we try to sort out the ones with different suffixes
  if ( !filename.endsWith( QLatin1String( ".svg" ) ) )
  {
    return false;
  }

  QSvgRenderer svgRenderer( filename );
  return svgRenderer.isValid();
}

bool QgsComposerPictureWidget::testImageFile( const QString &filename ) const
{
  QString formatName = QString( QImageReader::imageFormat( filename ) );
  return !formatName.isEmpty(); //file is in a supported pixel format
}

void QgsComposerPictureWidget::loadPicturePreviews( bool collapsed )
{
  if ( mPreviewsLoaded )
  {
    return;
  }

  if ( !collapsed ) // load the previews only on first parent group box expansion
  {
    mPreviewListWidget->hide();
    mPreviewsLoadingLabel->show();
    addStandardDirectoriesToPreview();
    mPreviewsLoadingLabel->hide();
    mPreviewListWidget->show();
  }
}

void QgsComposerPictureWidget::mFillColorButton_colorChanged( const QColor &color )
{
  mPicture->beginCommand( tr( "Picture fill color changed" ), QgsComposerMergeCommand::ComposerPictureFillColor );
  mPicture->setSvgFillColor( color );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::mStrokeColorButton_colorChanged( const QColor &color )
{
  mPicture->beginCommand( tr( "Picture stroke color changed" ), QgsComposerMergeCommand::ComposerPictureStrokeColor );
  mPicture->setSvgStrokeColor( color );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::mStrokeWidthSpinBox_valueChanged( double d )
{
  mPicture->beginCommand( tr( "Picture stroke width changed" ) );
  mPicture->setSvgStrokeWidth( d );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::mPictureRotationOffsetSpinBox_valueChanged( double d )
{
  mPicture->beginCommand( tr( "Picture North offset changed" ), QgsComposerMergeCommand::ComposerPictureNorthOffset );
  mPicture->setNorthOffset( d );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::mNorthTypeComboBox_currentIndexChanged( int index )
{
  mPicture->beginCommand( tr( "Picture North mode changed" ) );
  mPicture->setNorthMode( static_cast< QgsComposerPicture::NorthMode >( mNorthTypeComboBox->itemData( index ).toInt() ) );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::resizeEvent( QResizeEvent *event )
{
  Q_UNUSED( event );
  mSearchDirectoriesComboBox->setMinimumWidth( mPreviewListWidget->sizeHint().width() );
}

void QgsComposerPictureWidget::populateDataDefinedButtons()
{
  updateDataDefinedButton( mSourceDDBtn );
  updateDataDefinedButton( mFillColorDDBtn );
  updateDataDefinedButton( mStrokeColorDDBtn );
  updateDataDefinedButton( mStrokeWidthDDBtn );

  //initial state of controls - disable related controls when dd buttons are active
  mPictureLineEdit->setEnabled( !mSourceDDBtn->isActive() );
}

