/***************************************************************************
    qgssavestyletodbdialog.cpp
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 by Emilio Loi
    email                : loi at faunalia dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssavestyletodbdialog.h"

#include "qgssettings.h"

#include <QFileDialog>
#include <QDomDocument>
#include <QMessageBox>
#include <QDateTime>

QgsSaveStyleToDbDialog::QgsSaveStyleToDbDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  connect( mFilePickButton, &QToolButton::clicked, this, &QgsSaveStyleToDbDialog::mFilePickButton_clicked );
  setWindowTitle( QStringLiteral( "Save Style in Database" ) );
  mDescriptionEdit->setTabChangesFocus( true );
  setTabOrder( mNameEdit, mDescriptionEdit );
  setTabOrder( mDescriptionEdit, mUseAsDefault );
  setTabOrder( mUseAsDefault, buttonBox );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/saveStyleToDb/geometry" ) ).toByteArray() );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsSaveStyleToDbDialog::showHelp );
}

QgsSaveStyleToDbDialog::~QgsSaveStyleToDbDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/saveStyleToDb/geometry" ), saveGeometry() );
}

QString QgsSaveStyleToDbDialog::getName()
{
  return mNameEdit->text();
}

QString QgsSaveStyleToDbDialog::getDescription()
{
  return mDescriptionEdit->toPlainText();
}

bool QgsSaveStyleToDbDialog::isDefault()
{
  return mUseAsDefault->isChecked();
}

QString QgsSaveStyleToDbDialog::getUIFileContent()
{
  return mUIFileContent;
}

void QgsSaveStyleToDbDialog::accept()
{
  if ( getName().isEmpty() )
  {
    QMessageBox::information( this, tr( "Save style in database" ), tr( "A name is mandatory" ) );
    return;
  }
  QDialog::accept();
}

void QgsSaveStyleToDbDialog::mFilePickButton_clicked()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Attach Qt Designer UI file" ), myLastUsedDir, tr( "Qt Designer UI file .ui" ) + " (*.ui)" );
  if ( myFileName.isNull() )
  {
    return;
  }
  QFileInfo myFI( myFileName );
  QFile uiFile( myFI.filePath() );

  QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  if ( uiFile.open( QIODevice::ReadOnly ) )
  {
    QString content( uiFile.readAll() );
    QDomDocument doc;

    if ( !doc.setContent( content ) || doc.documentElement().tagName().compare( QLatin1String( "ui" ) ) )
    {
      QMessageBox::warning( this, tr( "Wrong file" ),
                            tr( "The selected file does not appear to be a valid Qt Designer UI file." ) );
      return;
    }
    mUIFileContent = content;
    mFileNameLabel->setText( myFI.fileName() );
  }
}

void QgsSaveStyleToDbDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#save-and-share-layer-properties" ) );
}
