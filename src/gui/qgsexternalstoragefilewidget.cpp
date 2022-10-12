/***************************************************************************
  qgsexternalstoragefilewidget.cpp
  --------------------------------------
  Date                 : August 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalstoragefilewidget.h"

#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QGridLayout>
#include <QUrl>
#include <QDropEvent>
#include <QRegularExpression>
#include <QProgressBar>

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsexternalstorage.h"
#include "qgsexternalstorageregistry.h"
#include "qgsmessagebar.h"
#include "qgsexpression.h"

#define FILEPATH_VARIABLE "selected_file_path"

QgsExternalStorageFileWidget::QgsExternalStorageFileWidget( QWidget *parent )
  : QgsFileWidget( parent )
{
  mProgressLabel = new QLabel( this );
  mLayout->addWidget( mProgressLabel );
  mProgressLabel->hide();

  mProgressBar = new QProgressBar( this );
  mLayout->addWidget( mProgressBar );
  mProgressBar->hide();

  mCancelButton = new QToolButton( this );
  mLayout->addWidget( mCancelButton );
  mCancelButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mTaskCancel.svg" ) ) );
  mCancelButton->hide();

  updateAcceptDrops();
}

void QgsExternalStorageFileWidget::setStorageType( const QString &storageType )
{
  if ( storageType.isEmpty() )
    mExternalStorage = nullptr;

  else
  {
    mExternalStorage = QgsApplication::externalStorageRegistry()->externalStorageFromType( storageType );
    if ( !mExternalStorage )
    {
      QgsDebugMsg( QStringLiteral( "Invalid storage type: %1" ).arg( storageType ) );
    }
    else
    {
      addFileWidgetScope();
    }
  }

  updateAcceptDrops();
}

void QgsExternalStorageFileWidget::setReadOnly( bool readOnly )
{
  QgsFileWidget::setReadOnly( readOnly );
  updateAcceptDrops();
}

void QgsExternalStorageFileWidget::updateAcceptDrops()
{
  setAcceptDrops( !mReadOnly &&  mExternalStorage );
}

QString QgsExternalStorageFileWidget::storageType() const
{
  return mExternalStorage ? mExternalStorage->type() : QString();
}

QgsExternalStorage *QgsExternalStorageFileWidget::externalStorage() const
{
  return mExternalStorage;
}

void QgsExternalStorageFileWidget::setStorageAuthConfigId( const QString &authCfg )
{
  mAuthCfg = authCfg;
}

const QString &QgsExternalStorageFileWidget::storageAuthConfigId() const
{
  return mAuthCfg;
}

void QgsExternalStorageFileWidget::setStorageUrlExpression( const QString &urlExpression )
{
  mStorageUrlExpression.reset( new QgsExpression( urlExpression ) );
}

QgsExpression *QgsExternalStorageFileWidget::storageUrlExpression() const
{
  return mStorageUrlExpression.get();
}

QString QgsExternalStorageFileWidget::storageUrlExpressionString() const
{
  return mStorageUrlExpression ? mStorageUrlExpression->expression() : QString();
}


void QgsExternalStorageFileWidget::setExpressionContext( const QgsExpressionContext &context )
{
  mScope = nullptr; // deleted by old context when we override it with the new one
  mExpressionContext = context;
  addFileWidgetScope();
}

void QgsExternalStorageFileWidget::addFileWidgetScope()
{
  if ( !mExternalStorage || mScope )
    return;

  mScope = createFileWidgetScope();
  mExpressionContext << mScope;
}

QgsExpressionContextScope *QgsExternalStorageFileWidget::createFileWidgetScope()
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "FileWidget" ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable(
                        QStringLiteral( FILEPATH_VARIABLE ),
                        QString(), true, false, tr( "User selected absolute filepath" ) ) );
  return scope;
}

const QgsExpressionContext &QgsExternalStorageFileWidget::expressionContext() const
{
  return mExpressionContext;
}


void QgsExternalStorageFileWidget::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}

QgsMessageBar *QgsExternalStorageFileWidget::messageBar() const
{
  return mMessageBar;
}

void QgsExternalStorageFileWidget::updateLayout()
{
  mProgressLabel->setVisible( mStoreInProgress );
  mProgressBar->setVisible( mStoreInProgress );
  mCancelButton->setVisible( mStoreInProgress );

  const bool linkVisible = mUseLink && !mIsLinkEdited;

  mLineEdit->setVisible( !mStoreInProgress && !linkVisible );
  mLinkLabel->setVisible( !mStoreInProgress && linkVisible );
  mLinkEditButton->setVisible( !mStoreInProgress && mUseLink && !mReadOnly );

  mFileWidgetButton->setVisible( mButtonVisible && !mStoreInProgress );
  mFileWidgetButton->setEnabled( !mReadOnly );
  mLineEdit->setEnabled( !mReadOnly );

  mLinkEditButton->setIcon( linkVisible && !mReadOnly ?
                            QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) :
                            QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) ) );
}

void QgsExternalStorageFileWidget::setSelectedFileNames( QStringList fileNames )
{
  Q_ASSERT( fileNames.count() );

  // store files first, update filePath later
  if ( mExternalStorage )
  {
    if ( !mStorageUrlExpression->prepare( &mExpressionContext ) )
    {
      if ( messageBar() )
      {
        messageBar()->pushWarning( tr( "Storing External resource" ),
                                   tr( "Storage URL expression is invalid : %1" ).arg( mStorageUrlExpression->evalErrorString() ) );
      }

      QgsDebugMsg( tr( "Storage URL expression is invalid : %1" ).arg( mStorageUrlExpression->evalErrorString() ) );
      return;
    }

    storeExternalFiles( fileNames );
  }
  else
  {
    QgsFileWidget::setSelectedFileNames( fileNames );
  }
}

void QgsExternalStorageFileWidget::storeExternalFiles( QStringList fileNames, QStringList storedUrls )
{
  if ( fileNames.isEmpty() )
    return;

  const QString filePath = fileNames.takeFirst();

  mProgressLabel->setText( tr( "Storing file %1 ..." ).arg( QFileInfo( filePath ).fileName() ) );
  mStoreInProgress = true;
  mProgressBar->setValue( 0 );
  updateLayout();

  Q_ASSERT( mScope );
  mScope->setVariable( QStringLiteral( FILEPATH_VARIABLE ), filePath );

  QVariant url = mStorageUrlExpression->evaluate( &mExpressionContext );
  if ( !url.isValid() )
  {
    if ( messageBar() )
    {
      messageBar()->pushWarning( tr( "Storing External resource" ),
                                 tr( "Storage URL expression is invalid : %1" ).arg( mStorageUrlExpression->evalErrorString() ) );
    }

    mStoreInProgress = false;
    updateLayout();

    return;
  }

  QgsExternalStorageStoredContent *storedContent = mExternalStorage->store( filePath, url.toString(), mAuthCfg );

  connect( storedContent, &QgsExternalStorageStoredContent::progressChanged, mProgressBar, &QProgressBar::setValue );
  connect( mCancelButton, &QToolButton::clicked, storedContent, &QgsExternalStorageStoredContent::cancel );

  auto onStoreFinished = [ = ]
  {
    mStoreInProgress = false;
    updateLayout();
    storedContent->deleteLater();

    if ( storedContent->status() == Qgis::ContentStatus::Failed && messageBar() )
    {
      messageBar()->pushWarning( tr( "Storing External resource" ),
                                 tr( "Storing file '%1' to url '%2' has failed : %3" ).arg( filePath, url.toString(), storedContent->errorString() ) );
    }

    if ( storedContent->status() != Qgis::ContentStatus::Finished )
      return;

    QStringList newStoredUrls = storedUrls;
    newStoredUrls << storedContent->url();

    // every thing has been stored, we update filepath
    if ( fileNames.isEmpty() )
    {
      setFilePaths( newStoredUrls );
    }
    else
      storeExternalFiles( fileNames, newStoredUrls );
  };

  connect( storedContent, &QgsExternalStorageStoredContent::stored, onStoreFinished );
  connect( storedContent, &QgsExternalStorageStoredContent::canceled, onStoreFinished );
  connect( storedContent, &QgsExternalStorageStoredContent::errorOccurred, onStoreFinished );

  storedContent->store();
}

void QgsExternalStorageFileWidget::dragEnterEvent( QDragEnterEvent *event )
{
  const QStringList filePaths = mLineEdit->acceptableFilePaths( event );
  if ( !filePaths.isEmpty() )
  {
    event->acceptProposedAction();
  }
  else
  {
    event->ignore();
  }
}

void QgsExternalStorageFileWidget::dropEvent( QDropEvent *event )
{
  storeExternalFiles( mLineEdit->acceptableFilePaths( event ) );
  event->acceptProposedAction();
}
