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
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QSettings>
#include <QSvgRenderer>

QgsComposerPictureWidget::QgsComposerPictureWidget( QgsComposerPicture* picture ): QgsComposerItemBaseWidget( nullptr, picture ), mPicture( picture ), mPreviewsLoaded( false )
{
  setupUi( this );

  mFillColorButton->setAllowAlpha( true );
  mFillColorButton->setColorDialogTitle( tr( "Select fill color" ) );
  mFillColorButton->setContext( "composer" );
  mOutlineColorButton->setAllowAlpha( true );
  mOutlineColorButton->setColorDialogTitle( tr( "Select outline color" ) );
  mOutlineColorButton->setContext( "composer" );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, picture );
  mainLayout->addWidget( itemPropertiesWidget );

  if ( mPicture->composition() )
  {
    mComposerMapComboBox->setComposition( mPicture->composition() );
    mComposerMapComboBox->setItemType( QgsComposerItem::ComposerMap );
    connect( mComposerMapComboBox, SIGNAL( itemChanged( QgsComposerItem* ) ), this, SLOT( composerMapChanged( QgsComposerItem* ) ) );
  }

  setGuiElementValues();
  mPreviewsLoadingLabel->hide();

  mPreviewListWidget->setIconSize( QSize( 30, 30 ) );

  // mSearchDirectoriesGroupBox is a QgsCollapsibleGroupBoxBasic, so its collapsed state should not be saved/restored
  mSearchDirectoriesGroupBox->setCollapsed( true );
  // setup connection for loading previews on first expansion of group box
  connect( mSearchDirectoriesGroupBox, SIGNAL( collapsedStateChanged( bool ) ), this, SLOT( loadPicturePreviews( bool ) ) );

  connect( mPicture, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  connect( mPicture, SIGNAL( pictureRotationChanged( double ) ), this, SLOT( setPicRotationSpinValue( double ) ) );

  QgsAtlasComposition* atlas = atlasComposition();
  if ( atlas )
  {
    // repopulate data defined buttons if atlas layer changes
    connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
             this, SLOT( populateDataDefinedButtons() ) );
    connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( populateDataDefinedButtons() ) );
  }

  //connections for data defined buttons
  connect( mSourceDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mSourceDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mSourceDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mPictureLineEdit, SLOT( setDisabled( bool ) ) );
}

QgsComposerPictureWidget::~QgsComposerPictureWidget()
{

}

void QgsComposerPictureWidget::on_mPictureBrowseButton_clicked()
{
  QSettings s;
  QString openDir;
  QString lineEditText = mPictureLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( "/UI/lastComposerPictureDir", QDir::homePath() ).toString();
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
    QMessageBox::critical( nullptr, "Invalid file", "Error, file does not exist or is not readable" );
    return;
  }

  s.setValue( "/UI/lastComposerPictureDir", fileInfo.absolutePath() );

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

void QgsComposerPictureWidget::on_mPictureLineEdit_editingFinished()
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

void QgsComposerPictureWidget::on_mPictureRotationSpinBox_valueChanged( double d )
{
  if ( mPicture )
  {
    mPicture->beginCommand( tr( "Picture rotation changed" ), QgsComposerMergeCommand::ComposerPictureRotation );
    mPicture->setPictureRotation( d );
    mPicture->endCommand();
  }
}

void QgsComposerPictureWidget::on_mPreviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
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

void QgsComposerPictureWidget::on_mAddDirectoryButton_clicked()
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
  QSettings s;
  QStringList userDirList = s.value( "/Composer/PictureWidgetDirectories" ).toStringList();
  if ( !userDirList.contains( directory ) )
  {
    userDirList.append( directory );
  }
  s.setValue( "/Composer/PictureWidgetDirectories", userDirList );
}

void QgsComposerPictureWidget::on_mRemoveDirectoryButton_clicked()
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
    QListWidgetItem* currentItem = mPreviewListWidget->item( i );
    if ( currentItem && currentItem->data( Qt::UserRole ).toString().startsWith( directoryToRemove ) )
    {
      delete( mPreviewListWidget->takeItem( i ) );
    }
  }

  //update the image directory list in the settings
  QSettings s;
  QStringList userDirList = s.value( "/Composer/PictureWidgetDirectories" ).toStringList();
  userDirList.removeOne( directoryToRemove );
  s.setValue( "/Composer/PictureWidgetDirectories", userDirList );
}

void QgsComposerPictureWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Picture resize mode changed" ) );
  mPicture->setResizeMode(( QgsComposerPicture::ResizeMode )index );
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

void QgsComposerPictureWidget::on_mAnchorPointComboBox_currentIndexChanged( int index )
{
  if ( !mPicture )
  {
    return;
  }

  mPicture->beginCommand( tr( "Picture placement changed" ) );
  mPicture->setPictureAnchor(( QgsComposerItem::ItemPositionMode )index );
  mPicture->endCommand();
}

void QgsComposerPictureWidget::on_mRotationFromComposerMapCheckBox_stateChanged( int state )
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
    mPicture->setPictureRotation( mPictureRotationSpinBox->value() );
  }
  else
  {
    const QgsComposerMap* map = dynamic_cast< const QgsComposerMap* >( mComposerMapComboBox->currentItem() );
    int mapId = map ? map->id() : -1;
    mPicture->setRotationMap( mapId );
    mPictureRotationSpinBox->setEnabled( false );
    mComposerMapComboBox->setEnabled( true );
  }
  mPicture->endCommand();
}

void QgsComposerPictureWidget::composerMapChanged( QgsComposerItem* item )
{
  if ( !mPicture )
  {
    return;
  }

  //get composition
  const QgsComposition* composition = mPicture->composition();
  if ( !composition )
  {
    return;
  }

  QgsComposerMap* composerMap = dynamic_cast< QgsComposerMap*>( item );
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
    mResizeModeComboBox->blockSignals( true );
    mAnchorPointComboBox->blockSignals( true );
    mFillColorButton->blockSignals( true );
    mOutlineColorButton->blockSignals( true );
    mOutlineWidthSpinBox->blockSignals( true );

    mPictureLineEdit->setText( mPicture->picturePath() );
    mPictureRotationSpinBox->setValue( mPicture->pictureRotation() );

    const QgsComposerMap* map = mPicture->composition()->getComposerMapById( mPicture->rotationMap() );
    if ( map )
      mComposerMapComboBox->setItem( map );
    else
      mComposerMapComboBox->setCurrentIndex( 0 );

    if ( mPicture->useRotationMap() )
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Checked );
      mPictureRotationSpinBox->setEnabled( false );
      mComposerMapComboBox->setEnabled( true );
    }
    else
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Unchecked );
      mPictureRotationSpinBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( false );
    }

    mResizeModeComboBox->setCurrentIndex(( int )mPicture->resizeMode() );
    //disable picture rotation for non-zoom modes
    mRotationGroupBox->setEnabled( mPicture->resizeMode() == QgsComposerPicture::Zoom ||
                                   mPicture->resizeMode() == QgsComposerPicture::ZoomResizeFrame );

    mAnchorPointComboBox->setCurrentIndex(( int )mPicture->pictureAnchor() );
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
    mOutlineColorButton->setColor( mPicture->svgBorderColor() );
    mOutlineWidthSpinBox->setValue( mPicture->svgBorderWidth() );

    mRotationFromComposerMapCheckBox->blockSignals( false );
    mPictureRotationSpinBox->blockSignals( false );
    mPictureLineEdit->blockSignals( false );
    mComposerMapComboBox->blockSignals( false );
    mResizeModeComboBox->blockSignals( false );
    mAnchorPointComboBox->blockSignals( false );
    mFillColorButton->blockSignals( false );
    mOutlineColorButton->blockSignals( false );
    mOutlineWidthSpinBox->blockSignals( false );

    populateDataDefinedButtons();
  }
}

QIcon QgsComposerPictureWidget::svgToIcon( const QString& filePath ) const
{
  QColor fill, outline;
  double outlineWidth, fillOpacity, outlineOpacity;
  bool fillParam, fillOpacityParam, outlineParam, outlineWidthParam, outlineOpacityParam;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultOutlineColor = false,
                             hasDefaultOutlineWidth = false, hasDefaultOutlineOpacity = false;
  QgsSvgCache::instance()->containsParams( filePath, fillParam, hasDefaultFillColor, fill,
      fillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      outlineParam, hasDefaultOutlineColor, outline,
      outlineWidthParam, hasDefaultOutlineWidth, outlineWidth,
      outlineOpacityParam, hasDefaultOutlineOpacity, outlineOpacity );

  //if defaults not set in symbol, use these values
  if ( !hasDefaultFillColor )
    fill = QColor( 200, 200, 200 );
  fill.setAlphaF( hasDefaultFillOpacity ? fillOpacity : 1.0 );
  if ( !hasDefaultOutlineColor )
    outline = Qt::black;
  outline.setAlphaF( hasDefaultOutlineOpacity ? outlineOpacity : 1.0 );
  if ( !hasDefaultOutlineWidth )
    outlineWidth = 0.6;

  bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
  const QImage& img = QgsSvgCache::instance()->svgAsImage( filePath, 30.0, fill, outline, outlineWidth, 3.5 /*appr. 88 dpi*/, 1.0, fitsInCache );

  return QIcon( QPixmap::fromImage( img ) );
}

void QgsComposerPictureWidget::updateSvgParamGui( bool resetValues )
{
  if ( !mPicture )
    return;

  QString picturePath = mPicture->picturePath();
  if ( !picturePath.endsWith( ".svg", Qt::CaseInsensitive ) )
  {
    mFillColorButton->setEnabled( false );
    mOutlineColorButton->setEnabled( false );
    mOutlineWidthSpinBox->setEnabled( false );
    return;
  }

  //activate gui for svg parameters only if supported by the svg file
  bool hasFillParam, hasFillOpacityParam, hasOutlineParam, hasOutlineWidthParam, hasOutlineOpacityParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth, defaultFillOpacity, defaultOutlineOpacity;
  bool hasDefaultFillColor, hasDefaultFillOpacity, hasDefaultOutlineColor, hasDefaultOutlineWidth, hasDefaultOutlineOpacity;
  QgsSvgCache::instance()->containsParams( picturePath, hasFillParam, hasDefaultFillColor, defaultFill,
      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
      hasOutlineParam, hasDefaultOutlineColor, defaultOutline,
      hasOutlineWidthParam, hasDefaultOutlineWidth, defaultOutlineWidth,
      hasOutlineOpacityParam, hasDefaultOutlineOpacity, defaultOutlineOpacity );

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
  mFillColorButton->setAllowAlpha( hasFillOpacityParam );
  if ( resetValues )
  {
    QColor outline = mOutlineColorButton->color();
    double newOpacity = hasOutlineOpacityParam ? outline.alphaF() : 1.0;
    if ( hasDefaultOutlineColor )
    {
      outline = defaultOutline;
    }
    outline.setAlphaF( hasDefaultOutlineOpacity ? defaultOutlineOpacity : newOpacity );
    mOutlineColorButton->setColor( outline );
  }
  mOutlineColorButton->setEnabled( hasOutlineParam );
  mOutlineColorButton->setAllowAlpha( hasOutlineOpacityParam );
  if ( hasDefaultOutlineWidth && resetValues )
  {
    mOutlineWidthSpinBox->setValue( defaultOutlineWidth );
  }
  mOutlineWidthSpinBox->setEnabled( hasOutlineWidthParam );
}

int QgsComposerPictureWidget::addDirectoryToPreview( const QString& path )
{
  //go through all files of a directory
  QDir directory( path );
  if ( !directory.exists() || !directory.isReadable() )
  {
    return 1; //error
  }

  QFileInfoList fileList = directory.entryInfoList( QDir::Files );
  QFileInfoList::const_iterator fileIt = fileList.constBegin();

  QProgressDialog progress( "Adding Icons...", "Abort", 0, fileList.size() - 1, this );
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

    QListWidgetItem * listItem = new QListWidgetItem( mPreviewListWidget );
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

    listItem->setText( "" );
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
  QSettings s;
  QStringList userDirList = s.value( "/Composer/PictureWidgetDirectories" ).toStringList();
  QStringList::const_iterator userDirIt = userDirList.constBegin();
  for ( ; userDirIt != userDirList.constEnd(); ++userDirIt )
  {
    addDirectoryToPreview( *userDirIt );
    mSearchDirectoriesComboBox->addItem( *userDirIt );
  }

  mPreviewsLoaded = true;
}

bool QgsComposerPictureWidget::testSvgFile( const QString& filename ) const
{
  //QSvgRenderer crashes with some (non-svg) xml documents.
  //So at least we try to sort out the ones with different suffixes
  if ( !filename.endsWith( ".svg" ) )
  {
    return false;
  }

  QSvgRenderer svgRenderer( filename );
  return svgRenderer.isValid();
}

bool QgsComposerPictureWidget::testImageFile( const QString& filename ) const
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

void QgsComposerPictureWidget::on_mFillColorButton_colorChanged( const QColor& color )
{
  mPicture->beginCommand( tr( "Picture fill color changed" ) );
  mPicture->setSvgFillColor( color );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::on_mOutlineColorButton_colorChanged( const QColor& color )
{
  mPicture->beginCommand( tr( "Picture border color changed" ) );
  mPicture->setSvgBorderColor( color );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  mPicture->beginCommand( tr( "Picture border width changed" ) );
  mPicture->setSvgBorderWidth( d );
  mPicture->endCommand();
  mPicture->update();
}

void QgsComposerPictureWidget::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  mSearchDirectoriesComboBox->setMinimumWidth( mPreviewListWidget->sizeHint().width() );
}

QgsComposerObject::DataDefinedProperty QgsComposerPictureWidget::ddPropertyForWidget( QgsDataDefinedButton *widget )
{
  if ( widget == mSourceDDBtn )
  {
    return QgsComposerObject::PictureSource;
  }

  return QgsComposerObject::NoProperty;
}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsComposerObject* composerObject = ( const QgsComposerObject* ) context;
  if ( !composerObject )
  {
    return QgsExpressionContext();
  }

  QScopedPointer< QgsExpressionContext > expContext( composerObject->createExpressionContext() );
  return QgsExpressionContext( *expContext );
}

void QgsComposerPictureWidget::populateDataDefinedButtons()
{
  QgsVectorLayer* vl = atlasCoverageLayer();

  //block signals from data defined buttons
  mSourceDDBtn->blockSignals( true );

  mSourceDDBtn->registerGetExpressionContextCallback( &_getExpressionContext, mPicture );

  //initialise buttons to use atlas coverage layer
  mSourceDDBtn->init( vl, mPicture->dataDefinedProperty( QgsComposerObject::PictureSource ),
                      QgsDataDefinedButton::AnyType, QgsDataDefinedButton::anyStringDesc() );

  //initial state of controls - disable related controls when dd buttons are active
  mPictureLineEdit->setEnabled( !mSourceDDBtn->isActive() );

  //unblock signals from data defined buttons
  mSourceDDBtn->blockSignals( false );
}

