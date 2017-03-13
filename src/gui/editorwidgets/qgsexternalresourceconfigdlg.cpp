/***************************************************************************
    qgsexternalresourceconfigdlg.cpp
     --------------------------------------
    Date                 : 2015-11-26
    Copyright            : (C) 2015 Médéric Ribreux
    Email                : mederic.ribreux at medspx dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalresourceconfigdlg.h"
#include "qgsexternalresourcewidget.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgspropertyoverridebutton.h"

#include <QFileDialog>

class QgsExternalResourceWidgetWrapper;

const QgsPropertiesDefinition &QgsExternalResourceConfigDlg::propertyDefinitions()
{
  static QgsPropertiesDefinition propertyDefinitions;

  if ( propertyDefinitions.isEmpty() )
  {
    propertyDefinitions = QgsPropertiesDefinition
    {
      { RootPath, QgsPropertyDefinition( "propertyRootPath", QgsPropertyDefinition::DataTypeString, QObject::tr( "Root path" ), QString() ) }
    };
  }

  return propertyDefinitions;
}

QgsExternalResourceConfigDlg::QgsExternalResourceConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  // By default, uncheck some options
  mUseLink->setChecked( false );
  mFullUrl->setChecked( false );
  mDocumentViewerGroupBox->setChecked( false );

  QString defpath = QgsProject::instance()->fileName().isEmpty() ? QDir::homePath() : QgsProject::instance()->fileInfo().absolutePath();

  mRootPath->setPlaceholderText( QgsSettings().value( QStringLiteral( "/UI/lastExternalResourceWidgetDefaultPath" ), QDir::toNativeSeparators( QDir::cleanPath( defpath ) ) ).toString() );

  connect( mRootPathButton, &QToolButton::clicked, this, &QgsExternalResourceConfigDlg::chooseDefaultPath );

  mRootPathPropertyOverrideButton->init( RootPath, mPropertyCollection, propertyDefinitions(), vl );

  mRootPathPropertyOverrideButton->setVectorLayer( vl );
  connect( mRootPathPropertyOverrideButton, &QgsPropertyOverrideButton::changed, this, &QgsExternalResourceConfigDlg::rootPathPropertyChanged );

  // Activate Relative Default Path option only if Default Path is set
  connect( mRootPath, &QLineEdit::textChanged, this, &QgsExternalResourceConfigDlg::enableRelativeDefault );
  connect( mRootPathExpression, &QLineEdit::textChanged, this, &QgsExternalResourceConfigDlg::enableRelativeDefault );
  connect( mRelativeGroupBox, &QGroupBox::toggled, this, &QgsExternalResourceConfigDlg::enableRelativeDefault );

  // set ids for StorageTypeButtons
  mStorageButtonGroup->setId( mStoreFilesButton, QgsFileWidget::GetFile );
  mStorageButtonGroup->setId( mStoreDirsButton, QgsFileWidget::GetDirectory );
  mStoreFilesButton->setChecked( true );

  // set ids for RelativeButtons
  mRelativeButtonGroup->setId( mRelativeProject, QgsFileWidget::RelativeProject );
  mRelativeButtonGroup->setId( mRelativeDefault, QgsFileWidget::RelativeDefaultPath );
  mRelativeProject->setChecked( true );

  mDocumentViewerContentComboBox->addItem( tr( "Image" ), QgsExternalResourceWidget::Image );
  mDocumentViewerContentComboBox->addItem( tr( "Web view" ), QgsExternalResourceWidget::Web );


  connect( mFileWidgetGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFileWidgetButtonGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFileWidgetFilterLineEdit, SIGNAL( textChanged( QString ) ), this, SIGNAL( changed() ) );
  connect( mUseLink, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFullUrl, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mRootPath, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mStorageButtonGroup, SIGNAL( buttonClicked( int ) ), this, SIGNAL( changed() ) );
  connect( mRelativeGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mDocumentViewerGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mDocumentViewerContentComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( changed() ) );
  connect( mDocumentViewerHeight, SIGNAL( valueChanged( int ) ), this, SIGNAL( changed() ) );
  connect( mDocumentViewerWidth, SIGNAL( valueChanged( int ) ), this, SIGNAL( changed() ) );
}

void QgsExternalResourceConfigDlg::chooseDefaultPath()
{
  QString dir;
  if ( !mRootPath->text().isEmpty() )
  {
    dir = mRootPath->text();
  }
  else
  {
    dir = QgsSettings().value( QStringLiteral( "/UI/lastExternalResourceWidgetDefaultPath" ), QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) ) ).toString();
  }

  QString rootName = QFileDialog::getExistingDirectory( this, tr( "Select a directory" ), dir, QFileDialog::ShowDirsOnly );

  if ( !rootName.isNull() )
    mRootPath->setText( rootName );
}

void QgsExternalResourceConfigDlg::rootPathPropertyChanged()
{
  QgsProperty prop = mRootPathPropertyOverrideButton->toProperty();

  setRootPathExpression( prop.expressionString() );

  mRootPathExpression->setVisible( prop.isActive() );
  mRootPath->setVisible( !prop.isActive() );
  mRootPathButton->setEnabled( !prop.isActive() );
}

void QgsExternalResourceConfigDlg::enableRelativeDefault()
{
  bool relativePathActive = false;

  if ( mRootPathPropertyOverrideButton->isActive() )
  {
    if ( !mRootPathExpression->text().isEmpty() )
      relativePathActive = true;
  }
  else
  {
    if ( !mRootPath->text().isEmpty() )
      relativePathActive = true;
  }

  // Activate (or not) the RelativeDefault button if default path
  if ( mRelativeGroupBox->isChecked() )
    mRelativeDefault->setEnabled( relativePathActive );

  // If no default path, RelativeProj button enabled by default
  if ( !relativePathActive )
    mRelativeProject->toggle();
}

QVariantMap QgsExternalResourceConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "FileWidget" ), mFileWidgetGroupBox->isChecked() );
  cfg.insert( QStringLiteral( "FileWidgetButton" ), mFileWidgetButtonGroupBox->isChecked() );
  cfg.insert( QStringLiteral( "FileWidgetFilter" ), mFileWidgetFilterLineEdit->text() );

  if ( mUseLink->isChecked() )
  {
    cfg.insert( QStringLiteral( "UseLink" ), mUseLink->isChecked() );
    if ( mFullUrl->isChecked() )
      cfg.insert( QStringLiteral( "FullUrl" ), mFullUrl->isChecked() );
  }

  if ( mRootPathPropertyOverrideButton->isActive() )
    cfg.insert( QStringLiteral( "DefaultRootStyle" ), QStringLiteral( "expression" ) );
  else
    cfg.insert( QStringLiteral( "DefaultRootStyle" ), QStringLiteral( "path" ) );


  if ( !mRootPath->text().isEmpty() )
    cfg.insert( QStringLiteral( "DefaultRoot" ), mRootPath->text() );

  if ( !mRootPathExpression->text().isEmpty() )
    cfg.insert( QStringLiteral( "DefaultRootExpression" ), mRootPathExpression->toolTip() );

  // Save Storage Mode
  cfg.insert( QStringLiteral( "StorageMode" ), mStorageButtonGroup->checkedId() );

  // Save Relative Paths option
  if ( mRelativeGroupBox->isChecked() )
  {
    cfg.insert( QStringLiteral( "RelativeStorage" ), mRelativeButtonGroup->checkedId() );
  }
  else
  {
    cfg.insert( QStringLiteral( "RelativeStorage" ), ( int )QgsFileWidget::Absolute );
  }

  if ( mDocumentViewerGroupBox->isChecked() )
  {
    cfg.insert( QStringLiteral( "DocumentViewer" ), mDocumentViewerContentComboBox->currentData().toInt() );
    cfg.insert( QStringLiteral( "DocumentViewerHeight" ), mDocumentViewerHeight->value() );
    cfg.insert( QStringLiteral( "DocumentViewerWidth" ), mDocumentViewerWidth->value() );
  }
  else
  {
    cfg.insert( QStringLiteral( "DocumentViewer" ), ( int )QgsExternalResourceWidget::NoContent );
  }

  return cfg;
}


void QgsExternalResourceConfigDlg::setConfig( const QVariantMap &config )
{
  if ( config.contains( QStringLiteral( "FileWidget" ) ) )
  {
    mFileWidgetGroupBox->setChecked( config.value( QStringLiteral( "FileWidget" ) ).toBool() );
  }
  if ( config.contains( QStringLiteral( "FileWidget" ) ) )
  {
    mFileWidgetButtonGroupBox->setChecked( config.value( QStringLiteral( "FileWidgetButton" ) ).toBool() );
  }
  if ( config.contains( QStringLiteral( "FileWidgetFilter" ) ) )
  {
    mFileWidgetFilterLineEdit->setText( config.value( QStringLiteral( "FileWidgetFilter" ) ).toString() );
  }

  if ( config.contains( QStringLiteral( "UseLink" ) ) )
  {
    mUseLink->setChecked( config.value( QStringLiteral( "UseLink" ) ).toBool() );
    if ( config.contains( QStringLiteral( "FullUrl" ) ) )
      mFullUrl->setChecked( true );
  }

  mRootPath->setText( config.value( QStringLiteral( "DefaultRoot" ) ).toString() );
  setRootPathExpression( config.value( QStringLiteral( "DefaultRootExpression" ) ).toString() );

  bool rootPathIsExpression = config.value( QStringLiteral( "DefaultRootStyle" ) ) == QStringLiteral( "expression" );

  QgsProperty prop = mRootPathPropertyOverrideButton->toProperty();
  prop.setActive( rootPathIsExpression );
  mRootPathPropertyOverrideButton->setToProperty( prop );
  rootPathPropertyChanged();

  mRootPathExpression->setVisible( rootPathIsExpression );
  mRootPath->setVisible( !rootPathIsExpression );

  // relative storage
  if ( config.contains( QStringLiteral( "RelativeStorage" ) ) )
  {
    int relative = config.value( QStringLiteral( "RelativeStorage" ) ).toInt();
    if ( ( QgsFileWidget::RelativeStorage )relative == QgsFileWidget::Absolute )
    {
      mRelativeGroupBox->setChecked( false );
    }
    else
    {
      mRelativeGroupBox->setChecked( true );
      mRelativeButtonGroup->button( relative )->setChecked( true );
    }
  }

  // set storage mode
  if ( config.contains( QStringLiteral( "StorageMode" ) ) )
  {
    int mode = config.value( QStringLiteral( "StorageMode" ) ).toInt();
    mStorageButtonGroup->button( mode )->setChecked( true );
  }

  // Document viewer
  if ( config.contains( QStringLiteral( "DocumentViewer" ) ) )
  {
    QgsExternalResourceWidget::DocumentViewerContent content = ( QgsExternalResourceWidget::DocumentViewerContent )config.value( QStringLiteral( "DocumentViewer" ) ).toInt();
    mDocumentViewerGroupBox->setChecked( content != QgsExternalResourceWidget::NoContent );
    int idx = mDocumentViewerContentComboBox->findData( content );
    if ( idx >= 0 )
    {
      mDocumentViewerContentComboBox->setCurrentIndex( idx );
    }
    if ( config.contains( QStringLiteral( "DocumentViewerHeight" ) ) )
    {
      mDocumentViewerHeight->setValue( config.value( QStringLiteral( "DocumentViewerHeight" ) ).toInt() );
    }
    if ( config.contains( QStringLiteral( "DocumentViewerWidth" ) ) )
    {
      mDocumentViewerWidth->setValue( config.value( QStringLiteral( "DocumentViewerWidth" ) ).toInt() );
    }
  }
}

void QgsExternalResourceConfigDlg::setRootPathExpression( const QString &expression )
{
  mRootPathExpression->setToolTip( expression );
  mRootPathPropertyOverrideButton->setText( expression );

  QgsProperty prop = mRootPathPropertyOverrideButton->toProperty();
  prop.setExpressionString( expression );
  mRootPathPropertyOverrideButton->setToProperty( prop );

  QgsExpression exp( expression );
  QgsExpressionContext ctx = layer()->createExpressionContext();

  mRootPathExpression->setText( exp.evaluate( &ctx ).toString() );
  enableRelativeDefault();
}
