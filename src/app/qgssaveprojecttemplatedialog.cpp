/***************************************************************************
    qgssaveprojecttemplatedialog.cpp
    --------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Jacky Volpes
    email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssaveprojecttemplatedialog.h"

#include "qgsgui.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "moc_qgssaveprojecttemplatedialog.cpp"

QgsSaveProjectTemplateDialog::QgsSaveProjectTemplateDialog( const QStringList &templateDirectories, const QString &initialName, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Save Project as Template" ) );

  mNameLineEdit = new QLineEdit( initialName, this );

  mDirectoryComboBox = new QComboBox( this );
  for ( const QString &templateDirectory : templateDirectories )
  {
    mDirectoryComboBox->addItem( QDir::toNativeSeparators( templateDirectory ), templateDirectory );
  }

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  QFormLayout *layout = new QFormLayout( this );
  layout->addRow( tr( "Template name" ), mNameLineEdit );
  layout->addRow( tr( "Template directory" ), mDirectoryComboBox );
  layout->addRow( mButtonBox );

  const auto updateOkButton = [this] { mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( !mNameLineEdit->text().trimmed().isEmpty() && mDirectoryComboBox->currentIndex() >= 0 ); };
  connect( mNameLineEdit, &QLineEdit::textChanged, this, updateOkButton );
  updateOkButton();

  QgsGui::enableAutoGeometryRestore( this );
}

QString QgsSaveProjectTemplateDialog::templateName() const
{
  return mNameLineEdit->text().trimmed();
}

QString QgsSaveProjectTemplateDialog::templateDirectory() const
{
  return mDirectoryComboBox->currentData().toString();
}

void QgsSaveProjectTemplateDialog::accept()
{
  const QString directory = templateDirectory();
  if ( !QDir( directory ).exists() )
  {
    if ( QMessageBox::question( this, tr( "Create Template Directory" ), tr( "The template directory %1 does not exist. Do you want to create it?" ).arg( QDir::toNativeSeparators( directory ) ) )
         != QMessageBox::Yes )
      return;

    if ( !QDir().mkpath( directory ) )
    {
      QMessageBox::warning( this, tr( "Create Template Directory" ), tr( "Could not create the template directory %1." ).arg( QDir::toNativeSeparators( directory ) ) );
      return;
    }
  }

  QDialog::accept();
}
