/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssavestyletodbdialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QDomDocument>
#include <QMessageBox>
#include <QDateTime>

QgsSaveStyleToDbDialog::QgsSaveStyleToDbDialog( QWidget *parent ) :
    QDialog( parent )
{
  setupUi( this );
  setWindowTitle( "Save style in database" );
  mDescriptionEdit->setTabChangesFocus( true );
  setTabOrder( mNameEdit, mDescriptionEdit );
  setTabOrder( mDescriptionEdit, mUseAsDefault );
  setTabOrder( mUseAsDefault, buttonBox );

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

void QgsSaveStyleToDbDialog::on_mFilePickButton_clicked()
{
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Attach Qt Designer UI file" ), myLastUsedDir, tr( "Qt Designer UI file .ui" ) + " (*.ui)" );
  if ( myFileName.isNull() )
  {
    return;
  }


  QFileInfo myFI( myFileName );

  QFile uiFile( myFI.filePath() );

  QString myPath = myFI.path();
  myQSettings.setValue( "style/lastStyleDir", myPath );

  if ( uiFile.open( QIODevice::ReadOnly ) )
  {
    QString content( uiFile.readAll() );
    QDomDocument doc;

    if ( !doc.setContent( content ) || doc.documentElement().tagName().compare( "ui" ) )
    {
      QMessageBox::warning( this, tr( "Wrong file" ),
                            tr( "The selected file does not appear to be a valid Qt Designer UI file." ) );
      return;
    }
    mUIFileContent = content;
    mFileNameLabel->setText( myFI.fileName() );
  }

}

