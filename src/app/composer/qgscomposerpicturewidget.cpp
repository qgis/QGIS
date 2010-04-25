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
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QSvgRenderer>

QgsComposerPictureWidget::QgsComposerPictureWidget( QgsComposerPicture* picture ): QWidget(), mPicture( picture )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, picture );
  toolBox->addItem( itemPropertiesWidget, tr( "General options" ) );

  mWidthLineEdit->setValidator( new QDoubleValidator( this ) );
  mHeightLineEdit->setValidator( new QDoubleValidator( this ) );
  setGuiElementValues();

  mPreviewListWidget->setIconSize( QSize( 30, 30 ) );

  //add preview icons
  addStandardDirectoriesToPreview();
  connect( mPicture, SIGNAL( settingsChanged() ), this, SLOT( setGuiElementValues() ) );
  connect( mPicture, SIGNAL( rotationChanged( double ) ), this, SLOT( setGuiElementValues() ) );
}

QgsComposerPictureWidget::~QgsComposerPictureWidget()
{

}

void QgsComposerPictureWidget::on_mPictureBrowseButton_clicked()
{
  QString openDir;
  QString lineEditText = mPictureLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }


  //show file dialog
  QString filePath = QFileDialog::getOpenFileName( 0, tr( "Select svg or image file" ), openDir );
  if ( filePath.isEmpty() )
  {
    return;
  }

  //check if file exists
  QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( 0, "Invalid file", "Error, file does not exist or is not readable" );
    return;
  }

  mPictureLineEdit->blockSignals( true );
  mPictureLineEdit->setText( filePath );
  mPictureLineEdit->blockSignals( false );

  //pass file path to QgsComposerPicture
  if ( mPicture )
  {
    mPicture->setPictureFile( filePath );
    mPicture->update();
  }
}

void QgsComposerPictureWidget::on_mPictureLineEdit_editingFinished()
{
  if ( mPicture )
  {
    QString filePath = mPictureLineEdit->text();

    //check if file exists
    QFileInfo fileInfo( filePath );

    if ( !fileInfo.exists() || !fileInfo.isReadable() )
    {
      QMessageBox::critical( 0, "Invalid file", "Error, file does not exist or is not readable" );
      return;
    }

    mPicture->setPictureFile( filePath );
    mPicture->update();
  }
}

void QgsComposerPictureWidget::on_mWidthLineEdit_editingFinished()
{
  if ( mPicture )
  {
    QRectF pictureRect = mPicture->rect();

    bool conversionOk;
    double newWidth = mWidthLineEdit->text().toDouble( &conversionOk );
    if ( conversionOk )
    {
      QRectF newSceneRect( mPicture->transform().dx(), mPicture->transform().dy(), newWidth, pictureRect.height() );
      mPicture->setSceneRect( newSceneRect );
    }
  }
}

void QgsComposerPictureWidget::on_mHeightLineEdit_editingFinished()
{
  if ( mPicture )
  {
    QRectF pictureRect = mPicture->rect();

    bool conversionOk;
    double newHeight = mHeightLineEdit->text().toDouble( &conversionOk );
    if ( conversionOk )
    {
      QRectF newSceneRect( mPicture->transform().dx(), mPicture->transform().dy(), pictureRect.width(), newHeight );
      mPicture->setSceneRect( newSceneRect );
    }
  }
}

void QgsComposerPictureWidget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mPicture )
  {
    mPicture->setRotation( d );
    mPicture->update();
  }
}

void QgsComposerPictureWidget::on_mPreviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  if ( !mPicture || !current )
  {
    return;
  }

  QString absoluteFilePath = current->data( Qt::UserRole ).toString();
  mPicture->setPictureFile( absoluteFilePath );
  mPictureLineEdit->setText( absoluteFilePath );
  mPicture->update();
}

void QgsComposerPictureWidget::on_mAddDirectoryButton_clicked()
{
  //let user select a directory
  QString directory = QFileDialog::getExistingDirectory( 0, tr( "Select new preview directory" ) );
  if ( directory.isNull() )
  {
    return; //dialog canceled by user
  }

  //add entry to mSearchDirectoriesComboBox
  mSearchDirectoriesComboBox->addItem( directory );

  //and add icons to the preview
  addDirectoryToPreview( directory );
}

void QgsComposerPictureWidget::on_mRemoveDirectoryButton_clicked()
{
  QString directoryToRemove = mSearchDirectoriesComboBox->currentText();
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
}

void QgsComposerPictureWidget::on_mRotationFromComposerMapCheckBox_stateChanged( int state )
{
  if ( !mPicture )
  {
    return;
  }

  if ( state == Qt::Unchecked )
  {
    mPicture->setRotationMap( -1 );
    mRotationSpinBox->setEnabled( true );
    mComposerMapComboBox->setEnabled( false );
  }
  else
  {
    int currentItemIndex = mComposerMapComboBox->currentIndex();
    if ( currentItemIndex == -1 )
    {
      return;
    }
    int composerId = mComposerMapComboBox->itemData( currentItemIndex, Qt::UserRole ).toInt();
    mPicture->setRotationMap( composerId );
    mRotationSpinBox->setEnabled( false );
    mComposerMapComboBox->setEnabled( true );
  }
}

void QgsComposerPictureWidget::showEvent( QShowEvent * event )
{
  refreshMapComboBox();
  QWidget::showEvent( event );
}

void QgsComposerPictureWidget::on_mComposerMapComboBox_activated( const QString & text )
{
  if ( !mPicture || text.isEmpty() || !mPicture->useRotationMap() )
  {
    return;
  }

  //get composition
  const QgsComposition* composition = mPicture->composition();
  if ( !composition )
  {
    return;
  }

  //extract id
  int id;
  bool conversionOk;
  QStringList textSplit = text.split( " " );
  if ( textSplit.size() < 1 )
  {
    return;
  }

  QString idString = textSplit.at( textSplit.size() - 1 );
  id = idString.toInt( &conversionOk );

  if ( !conversionOk )
  {
    return;
  }

  const QgsComposerMap* composerMap = composition->getComposerMapById( id );
  if ( !composerMap )
  {
    return;
  }
  mPicture->setRotationMap( id );
  mPicture->update();
}

void QgsComposerPictureWidget::refreshMapComboBox()
{
  mComposerMapComboBox->blockSignals( true );
  //save the current entry in case it is still present after refresh
  QString saveCurrentComboText = mComposerMapComboBox->currentText();

  mComposerMapComboBox->clear();

  if ( mPicture )
  {
    //insert available maps into mMapComboBox
    const QgsComposition* composition = mPicture->composition();
    if ( composition )
    {
      QList<const QgsComposerMap*> availableMaps = composition->composerMapItems();
      QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
      for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
      {
        mComposerMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ), ( *mapItemIt )->id() );
      }
    }
  }

  if ( !saveCurrentComboText.isEmpty() )
  {
    if ( mComposerMapComboBox->findText( saveCurrentComboText ) == -1 )
    {
      //the former entry is no longer present. Inform the scalebar about the changed composer map
      on_mComposerMapComboBox_activated( mComposerMapComboBox->currentText() );
    }
    else
    {
      //the former entry is still present. Make it the current entry again
      mComposerMapComboBox->setCurrentIndex( mComposerMapComboBox->findText( saveCurrentComboText ) );
    }
  }
  mComposerMapComboBox->blockSignals( false );
}

void QgsComposerPictureWidget::setGuiElementValues()
{
  //set initial gui values
  if ( mPicture )
  {
    mWidthLineEdit->blockSignals( true );
    mHeightLineEdit->blockSignals( true );
    mRotationSpinBox->blockSignals( true );
    mPictureLineEdit->blockSignals( true );
    mComposerMapComboBox->blockSignals( true );
    mRotationFromComposerMapCheckBox->blockSignals( true );

    mPictureLineEdit->setText( mPicture->pictureFile() );
    QRectF pictureRect = mPicture->rect();
    mWidthLineEdit->setText( QString::number( pictureRect.width() ) );
    mHeightLineEdit->setText( QString::number( pictureRect.height() ) );
    mRotationSpinBox->setValue( mPicture->rotation() );

    refreshMapComboBox();

    if ( mPicture->useRotationMap() )
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Checked );
      mRotationSpinBox->setEnabled( false );
      mComposerMapComboBox->setEnabled( true );
      QString mapText = tr( "Map %1" ).arg( mPicture->rotationMap() );
      int itemId = mComposerMapComboBox->findText( mapText );
      if ( itemId >= 0 )
      {
        mComposerMapComboBox->setCurrentIndex( itemId );
      }
    }
    else
    {
      mRotationFromComposerMapCheckBox->setCheckState( Qt::Unchecked );
      mRotationSpinBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( false );
    }


    mRotationFromComposerMapCheckBox->blockSignals( false );
    mWidthLineEdit->blockSignals( false );
    mHeightLineEdit->blockSignals( false );
    mRotationSpinBox->blockSignals( false );
    mPictureLineEdit->blockSignals( false );
    mComposerMapComboBox->blockSignals( false );
  }
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
      ++counter; continue;
    }

    QListWidgetItem * listItem = new QListWidgetItem( mPreviewListWidget );
    listItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    if ( fileIsSvg )
    {
      QIcon icon( filePath );
      listItem->setIcon( icon );
    }
    else if ( fileIsPixel ) //for pixel formats: create icon from scaled pixmap
    {
      QPixmap iconPixmap( filePath );
      if ( iconPixmap.isNull() )
      {
        ++counter; continue; //unknown file format or other problem
      }
      //set pixmap hardcoded to 30/30, same as icon size for mPreviewListWidget
      QPixmap scaledPixmap( iconPixmap.scaled( QSize( 30, 30 ), Qt::KeepAspectRatio ) );
      QIcon icon( scaledPixmap );
      listItem->setIcon( icon );
    }
    else
    {
      ++counter; continue;
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
