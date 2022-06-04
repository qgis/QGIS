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
#include "qgseditorwidgetwrapper.h"
#include "qgsexternalstorage.h"
#include "qgsexternalstorageregistry.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexternalstoragefilewidget.h"

#include <QFileDialog>
#include <QComboBox>

class QgsExternalResourceWidgetWrapper;

QgsExternalResourceConfigDlg::QgsExternalResourceConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  mStorageType->addItem( tr( "Select Existing file" ), QString() );
  for ( QgsExternalStorage *storage : QgsApplication::externalStorageRegistry()->externalStorages() )
  {
    mStorageType->addItem( storage->displayName(), storage->type() );
  }

  mExternalStorageGroupBox->setVisible( false );

  initializeDataDefinedButton( mStorageUrlPropertyOverrideButton, QgsEditorWidgetWrapper::StorageUrl );
  mStorageUrlPropertyOverrideButton->registerVisibleWidget( mStorageUrlExpression );
  mStorageUrlPropertyOverrideButton->registerExpressionWidget( mStorageUrlExpression );
  mStorageUrlPropertyOverrideButton->registerVisibleWidget( mStorageUrl, false );
  mStorageUrlPropertyOverrideButton->registerExpressionContextGenerator( this );

  // By default, uncheck some options
  mUseLink->setChecked( false );
  mFullUrl->setChecked( false );

  const QString defpath = QgsProject::instance()->fileName().isEmpty() ? QDir::homePath() : QFileInfo( QgsProject::instance()->absoluteFilePath() ).path();

  mRootPath->setPlaceholderText( QgsSettings().value( QStringLiteral( "/UI/lastExternalResourceWidgetDefaultPath" ), QDir::toNativeSeparators( QDir::cleanPath( defpath ) ) ).toString() );

  connect( mRootPathButton, &QToolButton::clicked, this, &QgsExternalResourceConfigDlg::chooseDefaultPath );

  initializeDataDefinedButton( mRootPathPropertyOverrideButton, QgsEditorWidgetWrapper::RootPath );
  mRootPathPropertyOverrideButton->registerVisibleWidget( mRootPathExpression );
  mRootPathPropertyOverrideButton->registerExpressionWidget( mRootPathExpression );
  mRootPathPropertyOverrideButton->registerVisibleWidget( mRootPath, false );
  mRootPathPropertyOverrideButton->registerEnabledWidget( mRootPathButton, false );

  initializeDataDefinedButton( mDocumentViewerContentPropertyOverrideButton, QgsEditorWidgetWrapper::DocumentViewerContent );
  mDocumentViewerContentPropertyOverrideButton->registerVisibleWidget( mDocumentViewerContentExpression );
  mDocumentViewerContentPropertyOverrideButton->registerExpressionWidget( mDocumentViewerContentExpression );
  mDocumentViewerContentPropertyOverrideButton->registerEnabledWidget( mDocumentViewerContentComboBox, false );

  // Activate Relative Default Path option only if Default Path is set
  connect( mRootPath, &QLineEdit::textChanged, this, &QgsExternalResourceConfigDlg::enableRelativeDefault );
  connect( mRootPathExpression, &QLineEdit::textChanged, this, &QgsExternalResourceConfigDlg::enableRelativeDefault );

  // Add storage modes
  mStorageModeCbx->addItem( tr( "File Paths" ), QgsFileWidget::GetFile );
  mStorageModeCbx->addItem( tr( "Directory Paths" ), QgsFileWidget::GetDirectory );

  // Add storage path options
  mStoragePathCbx->addItem( tr( "Absolute Path" ), QgsFileWidget::Absolute );
  mStoragePathCbx->addItem( tr( "Relative to Project Path" ), QgsFileWidget::RelativeProject );
  mStoragePathCbx->addItem( tr( "Relative to Default Path" ), QgsFileWidget::RelativeDefaultPath );
  enableCbxItem( mStoragePathCbx, 2, false );

  connect( mStorageType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsExternalResourceConfigDlg::changeStorageType );
  connect( mFileWidgetGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFileWidgetButtonGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFileWidgetFilterLineEdit, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mUseLink, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFullUrl, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mRootPath, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mStorageModeCbx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mStoragePathCbx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mDocumentViewerGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mDocumentViewerContentComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),  this, [ = ]( int idx )
  { mDocumentViewerContentSettingsWidget->setEnabled( ( QgsExternalResourceWidget::DocumentViewerContent )idx != QgsExternalResourceWidget::NoContent ); } );
  connect( mDocumentViewerHeight, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mDocumentViewerWidth, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mStorageUrlExpression, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );

  mDocumentViewerContentComboBox->addItem( tr( "No Content" ), QgsExternalResourceWidget::NoContent );
  mDocumentViewerContentComboBox->addItem( tr( "Image" ), QgsExternalResourceWidget::Image );
  mDocumentViewerContentComboBox->addItem( tr( "Web View" ), QgsExternalResourceWidget::Web );
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
    const QString path = QFileInfo( QgsProject::instance()->absoluteFilePath() ).path();
    dir = QgsSettings().value( QStringLiteral( "/UI/lastExternalResourceWidgetDefaultPath" ), QDir::toNativeSeparators( QDir::cleanPath( path ) ) ).toString();
  }

  const QString rootName = QFileDialog::getExistingDirectory( this, tr( "Select a Directory" ), dir, QFileDialog::Options() );

  if ( !rootName.isNull() )
    mRootPath->setText( rootName );
}

void QgsExternalResourceConfigDlg::enableCbxItem( QComboBox *comboBox, int index, bool enabled )
{
  // https://stackoverflow.com/a/62261745
  const auto *model = qobject_cast<QStandardItemModel *>( comboBox->model() );
  assert( model );
  if ( !model )
    return;

  auto *item = model->item( index );
  assert( item );
  if ( !item )
    return;
  item->setEnabled( enabled );
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
  // Activate (or not) the RelativeDefault item if default path
  enableCbxItem( mStoragePathCbx, 2, relativePathActive );
}

QVariantMap QgsExternalResourceConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "StorageType" ), mStorageType->currentData() );
  cfg.insert( QStringLiteral( "StorageAuthConfigId" ), mAuthSettingsProtocol->configId() );
  if ( !mStorageUrl->text().isEmpty() )
    cfg.insert( QStringLiteral( "StorageUrl" ), mStorageUrl->text() );

  cfg.insert( QStringLiteral( "FileWidget" ), mFileWidgetGroupBox->isChecked() );
  cfg.insert( QStringLiteral( "FileWidgetButton" ), mFileWidgetButtonGroupBox->isChecked() );
  cfg.insert( QStringLiteral( "FileWidgetFilter" ), mFileWidgetFilterLineEdit->text() );

  if ( mUseLink->isChecked() )
  {
    cfg.insert( QStringLiteral( "UseLink" ), mUseLink->isChecked() );
    if ( mFullUrl->isChecked() )
      cfg.insert( QStringLiteral( "FullUrl" ), mFullUrl->isChecked() );
  }

  cfg.insert( QStringLiteral( "PropertyCollection" ), mPropertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );

  if ( !mRootPath->text().isEmpty() )
    cfg.insert( QStringLiteral( "DefaultRoot" ), mRootPath->text() );

  if ( !mStorageType->currentIndex() )
  {
    // Save Storage Mode
    cfg.insert( QStringLiteral( "StorageMode" ), mStorageModeCbx->currentData().toInt() );
    // Save Relative Paths option
    cfg.insert( QStringLiteral( "RelativeStorage" ), mStoragePathCbx->currentData().toInt() );
  }
  else
  {
    // Only file mode and absolute paths are supported for external storage
    cfg.insert( QStringLiteral( "StorageMode" ), static_cast<int>( QgsFileWidget::GetFile ) );
    cfg.insert( QStringLiteral( "RelativeStorage" ), static_cast<int>( QgsFileWidget::Absolute ) );
  }

  cfg.insert( QStringLiteral( "DocumentViewer" ), mDocumentViewerContentComboBox->currentData().toInt() );
  cfg.insert( QStringLiteral( "DocumentViewerHeight" ), mDocumentViewerHeight->value() );
  cfg.insert( QStringLiteral( "DocumentViewerWidth" ), mDocumentViewerWidth->value() );

  return cfg;
}


void QgsExternalResourceConfigDlg::setConfig( const QVariantMap &config )
{
  if ( config.contains( QStringLiteral( "StorageType" ) ) )
  {
    const int index = mStorageType->findData( config.value( QStringLiteral( "StorageType" ) ) );
    if ( index >= 0 )
      mStorageType->setCurrentIndex( index );
  }

  mAuthSettingsProtocol->setConfigId( config.value( QStringLiteral( "StorageAuthConfigId" ) ).toString() );
  mStorageUrl->setText( config.value( QStringLiteral( "StorageUrl" ) ).toString() );

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

  mPropertyCollection.loadVariant( config.value( QStringLiteral( "PropertyCollection" ) ), QgsWidgetWrapper::propertyDefinitions() );
  updateDataDefinedButtons();

  mRootPath->setText( config.value( QStringLiteral( "DefaultRoot" ) ).toString() );

  // relative storage
  if ( config.contains( QStringLiteral( "RelativeStorage" ) ) )
  {
    const int relative = config.value( QStringLiteral( "RelativeStorage" ) ).toInt();
    mStoragePathCbx->setCurrentIndex( relative );
  }

  // set storage mode
  if ( config.contains( QStringLiteral( "StorageMode" ) ) )
  {
    const int mode = config.value( QStringLiteral( "StorageMode" ) ).toInt();
    mStorageModeCbx->setCurrentIndex( mode );
  }

  // Document viewer
  if ( config.contains( QStringLiteral( "DocumentViewer" ) ) )
  {
    const QgsExternalResourceWidget::DocumentViewerContent content = ( QgsExternalResourceWidget::DocumentViewerContent )config.value( QStringLiteral( "DocumentViewer" ) ).toInt();
    const int idx = mDocumentViewerContentComboBox->findData( content );
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

QgsExpressionContext QgsExternalResourceConfigDlg::createExpressionContext() const
{
  QgsExpressionContext context = QgsEditorConfigWidget::createExpressionContext();
  context << QgsExpressionContextUtils::formScope( );
  context << QgsExpressionContextUtils::parentFormScope( );

  QgsExpressionContextScope *fileWidgetScope = QgsExternalStorageFileWidget::createFileWidgetScope();
  context << fileWidgetScope;

  context.setHighlightedVariables( fileWidgetScope->variableNames() );
  return context;
}

void QgsExternalResourceConfigDlg::changeStorageType( int storageTypeIndex )
{
  // first one in combo box is not an external storage
  mExternalStorageGroupBox->setVisible( storageTypeIndex > 0 );

  // for now, we store only files in external storage
  mStorageModeCbx->setVisible( !storageTypeIndex );
  mStorageModeLbl->setVisible( !storageTypeIndex );

  // Absolute path are mandatory when using external storage
  mStoragePathCbx->setVisible( !storageTypeIndex );
  mStoragePathLbl->setVisible( !storageTypeIndex );

  emit changed();
}
