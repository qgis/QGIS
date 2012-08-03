/***************************************************************************
                         qgssvgdiagramfactorywidget.cpp  -  description
                         ------------------------------
    begin                : December 2008
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

#include "qgssvgdiagramfactorywidget.h"
#include "qgsapplication.h"
#include "qgssvgdiagramfactory.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>

QgsSVGDiagramFactoryWidget::QgsSVGDiagramFactoryWidget(): QgsDiagramFactoryWidget()
{
  setupUi( this );

  //add preview icons
  addStandardDirectoriesToPreview();
}

QgsSVGDiagramFactoryWidget::~QgsSVGDiagramFactoryWidget()
{

}

QgsDiagramFactory* QgsSVGDiagramFactoryWidget::createFactory()
{
  QString filePath = mPictureLineEdit->text();
  if ( filePath.isEmpty() )
  {
    return 0;
  }

  QFile svgFile( filePath );
  if ( !svgFile.exists() )
  {
    return 0;
  }

  //open file and read binary array
  if ( !svgFile.open( QIODevice::ReadOnly ) )
  {
    return 0;
  }

  QByteArray svgData = svgFile.readAll();

  QgsSVGDiagramFactory* factory = new QgsSVGDiagramFactory();
  if ( !factory->setSVGData( svgData, filePath ) )
  {
    delete factory;
    return 0;
  }

  return factory;
}

void QgsSVGDiagramFactoryWidget::setExistingFactory( const QgsDiagramFactory* f )
{
  mPreviewListWidget->setLayoutDirection( Qt::LeftToRight );
  const QgsSVGDiagramFactory* svgFactory = dynamic_cast<const QgsSVGDiagramFactory *>( f );
  if ( svgFactory )
  {
    mPictureLineEdit->setText( svgFactory->svgFilePath() );
  }
}

void QgsSVGDiagramFactoryWidget::on_mPictureBrowseButton_clicked()
{
  QString openDir;
  QString lineEditText = mPictureLineEdit->text();
  if ( !lineEditText.isEmpty() )
  {
    QFileInfo openDirFileInfo( lineEditText );
    openDir = openDirFileInfo.path();
  }


  //show file dialog
  QString filePath = QFileDialog::getOpenFileName( 0, tr( "Select svg file" ), openDir );
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

  //check if it is a valid svg file
  if ( !testSvgFile( filePath ) )
  {
    QMessageBox::critical( 0, "Invalid file", "Error, the selected file is not a valid svg file" );
    return;
  }

  mPictureLineEdit->blockSignals( true );
  mPictureLineEdit->setText( filePath );
  mPictureLineEdit->blockSignals( false );
}

void QgsSVGDiagramFactoryWidget::on_mPreviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    return;
  }

  QString absoluteFilePath = current->data( Qt::UserRole ).toString();
  mPictureLineEdit->setText( absoluteFilePath );
}

void QgsSVGDiagramFactoryWidget::on_mAddDirectoryButton_clicked()
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

void QgsSVGDiagramFactoryWidget::on_mRemoveDirectoryButton_clicked()
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

int QgsSVGDiagramFactoryWidget::addDirectoryToPreview( const QString& path )
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
    bool fileIsSvg = testSvgFile( filePath );

    //exclude files that are not svg or image
    if ( !fileIsSvg )
    {
      ++counter;
      continue;
    }

    QListWidgetItem * listItem = new QListWidgetItem( mPreviewListWidget );

    if ( fileIsSvg )
    {
      QIcon icon( filePath );
      listItem->setIcon( icon );
    }

    listItem->setText( "" );
    //store the absolute icon file path as user data
    listItem->setData( Qt::UserRole, fileIt->absoluteFilePath() );
    ++counter;
  }

  return 0;
}

void QgsSVGDiagramFactoryWidget::addStandardDirectoriesToPreview()
{
  //list all directories in $prefix/share/qgis/svg
  foreach ( QString path, QgsApplication::svgPaths() )
  {
    QDir svgDirectory( path );
    if ( !svgDirectory.exists() || !svgDirectory.isReadable() )
    {
      continue; //error
    }

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

bool QgsSVGDiagramFactoryWidget::testSvgFile( const QString& filename ) const
{
  QSvgRenderer svgRenderer( filename );
  return svgRenderer.isValid();
}
