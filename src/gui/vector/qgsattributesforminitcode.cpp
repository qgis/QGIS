/***************************************************************************
    qgsattributesforminitcode.cpp
    ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributesforminitcode.h"
#include "moc_qgsattributesforminitcode.cpp"
#include "ui_qgsattributesforminitcode.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgshelp.h"

#include <QFileDialog>

QgsAttributesFormInitCode::QgsAttributesFormInitCode()
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  // Init function stuff
  mInitCodeSourceComboBox->addItem( QString(), QVariant::fromValue( Qgis::AttributeFormPythonInitCodeSource::NoSource ) );
  mInitCodeSourceComboBox->addItem( tr( "Load from External File" ), QVariant::fromValue( Qgis::AttributeFormPythonInitCodeSource::File ) );
  mInitCodeSourceComboBox->addItem( tr( "Provide Code in this Dialog" ), QVariant::fromValue( Qgis::AttributeFormPythonInitCodeSource::Dialog ) );
  mInitCodeSourceComboBox->addItem( tr( "Load from the Environment" ), QVariant::fromValue( Qgis::AttributeFormPythonInitCodeSource::Environment ) );

  const QgsSettings settings;
  mInitFileWidget->setDefaultRoot( settings.value( QStringLiteral( "style/lastInitFilePathDir" ), "." ).toString() );
  mInitFileWidget->setDialogTitle( tr( "Select Python File" ) );
  mInitFileWidget->setFilter( tr( "Python files (*.py *.PY)" ) );

  connect( mInitCodeSourceComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsAttributesFormInitCode::mInitCodeSourceComboBox_currentIndexChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsAttributesFormInitCode::showHelp );
}

void QgsAttributesFormInitCode::setCodeSource( Qgis::AttributeFormPythonInitCodeSource initCodeSource )
{
  mInitCodeSourceComboBox->setCurrentIndex( mInitCodeSourceComboBox->findData( QVariant::fromValue( initCodeSource ) ) );
  mInitCodeSourceComboBox_currentIndexChanged( mInitCodeSourceComboBox->currentIndex() );
}
void QgsAttributesFormInitCode::setInitFunction( const QString &initFunction )
{
  mInitFunctionLineEdit->setText( initFunction );
}
void QgsAttributesFormInitCode::setInitFilePath( const QString &initFilePath )
{
  mInitFileWidget->setFilePath( initFilePath );
}
void QgsAttributesFormInitCode::setInitCode( const QString &initCode )
{
  mInitCodeEditorPython->setText( initCode );
}

Qgis::AttributeFormPythonInitCodeSource QgsAttributesFormInitCode::codeSource() const
{
  return mInitCodeSourceComboBox->currentData().value<Qgis::AttributeFormPythonInitCodeSource>();
}

QString QgsAttributesFormInitCode::initFunction() const
{
  return mInitFunctionLineEdit->text();
}
QString QgsAttributesFormInitCode::initFilePath() const
{
  return mInitFileWidget->filePath();
}
QString QgsAttributesFormInitCode::initCode() const
{
  return mInitCodeEditorPython->text();
}

void QgsAttributesFormInitCode::mInitCodeSourceComboBox_currentIndexChanged( int )
{
  Qgis::AttributeFormPythonInitCodeSource codeSource = mInitCodeSourceComboBox->currentData().value<Qgis::AttributeFormPythonInitCodeSource>();
  mInitFunctionContainer->setVisible( codeSource != Qgis::AttributeFormPythonInitCodeSource::NoSource );
  mInitFilePathLabel->setVisible( codeSource == Qgis::AttributeFormPythonInitCodeSource::File );
  mInitFileWidget->setVisible( codeSource == Qgis::AttributeFormPythonInitCodeSource::File );
  mInitCodeEditorPython->setVisible( codeSource == Qgis::AttributeFormPythonInitCodeSource::Dialog );
}

void QgsAttributesFormInitCode::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#enhance-your-form-with-custom-functions" ) );
}
