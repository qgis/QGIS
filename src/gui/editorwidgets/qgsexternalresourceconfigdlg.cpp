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

#include "qgsapplication.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexternalresourcewidget.h"
#include "qgsexternalstorage.h"
#include "qgsexternalstoragefilewidget.h"
#include "qgsexternalstorageregistry.h"
#include "qgsproject.h"
#include "qgspropertyoverridebutton.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QComboBox>
#include <QFileDialog>

#include "moc_qgsexternalresourceconfigdlg.cpp"

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
  mAuthSettingsProtocol->removeBasicSettings();
  mExternalStorageGroupBox->setVisible( false );

  initializeDataDefinedButton( mStorageUrlPropertyOverrideButton, QgsEditorWidgetWrapper::Property::StorageUrl );
  mStorageUrlPropertyOverrideButton->registerVisibleWidget( mStorageUrlExpression );
  mStorageUrlPropertyOverrideButton->registerExpressionWidget( mStorageUrlExpression );
  mStorageUrlPropertyOverrideButton->registerVisibleWidget( mStorageUrl, false );
  mStorageUrlPropertyOverrideButton->registerExpressionContextGenerator( this );

  // By default, uncheck some options
  mUseLink->setChecked( false );
  mFullUrl->setChecked( false );

  const QString defpath = QgsProject::instance()->fileName().isEmpty() ? QDir::homePath() : QFileInfo( QgsProject::instance()->absoluteFilePath() ).path();

  mRootPath->setPlaceholderText( QgsSettings().value( u"/UI/lastExternalResourceWidgetDefaultPath"_s, QDir::toNativeSeparators( QDir::cleanPath( defpath ) ) ).toString() );

  connect( mRootPathButton, &QToolButton::clicked, this, &QgsExternalResourceConfigDlg::chooseDefaultPath );

  initializeDataDefinedButton( mRootPathPropertyOverrideButton, QgsEditorWidgetWrapper::Property::RootPath );
  mRootPathPropertyOverrideButton->registerVisibleWidget( mRootPathExpression );
  mRootPathPropertyOverrideButton->registerExpressionWidget( mRootPathExpression );
  mRootPathPropertyOverrideButton->registerVisibleWidget( mRootPath, false );
  mRootPathPropertyOverrideButton->registerEnabledWidget( mRootPathButton, false );

  initializeDataDefinedButton( mDocumentViewerContentPropertyOverrideButton, QgsEditorWidgetWrapper::Property::DocumentViewerContent );
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
  connect( mDocumentViewerContentComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    const QgsExternalResourceWidget::DocumentViewerContent content = static_cast<QgsExternalResourceWidget::DocumentViewerContent>( mDocumentViewerContentComboBox->currentData().toInt() );
    const bool hasSizeSettings = ( content != QgsExternalResourceWidget::NoContent && content != QgsExternalResourceWidget::Audio );
    mDocumentViewerContentSettingsWidget->setEnabled( hasSizeSettings );
  } );
  connect( mDocumentViewerHeight, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mDocumentViewerWidth, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mStorageUrlExpression, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );

  mDocumentViewerContentComboBox->addItem( tr( "No Content" ), QgsExternalResourceWidget::NoContent );
  mDocumentViewerContentComboBox->addItem( tr( "Image" ), QgsExternalResourceWidget::Image );
  mDocumentViewerContentComboBox->addItem( tr( "Audio" ), QgsExternalResourceWidget::Audio );
  mDocumentViewerContentComboBox->addItem( tr( "Video" ), QgsExternalResourceWidget::Video );
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
    dir = QgsSettings().value( u"/UI/lastExternalResourceWidgetDefaultPath"_s, QDir::toNativeSeparators( QDir::cleanPath( path ) ) ).toString();
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

  cfg.insert( u"StorageType"_s, mStorageType->currentData() );
  cfg.insert( u"StorageAuthConfigId"_s, mAuthSettingsProtocol->configId() );
  if ( !mStorageUrl->text().isEmpty() )
    cfg.insert( u"StorageUrl"_s, mStorageUrl->text() );

  cfg.insert( u"FileWidget"_s, mFileWidgetGroupBox->isChecked() );
  cfg.insert( u"FileWidgetButton"_s, mFileWidgetButtonGroupBox->isChecked() );
  cfg.insert( u"FileWidgetFilter"_s, mFileWidgetFilterLineEdit->text() );

  if ( mUseLink->isChecked() )
  {
    cfg.insert( u"UseLink"_s, mUseLink->isChecked() );
    if ( mFullUrl->isChecked() )
      cfg.insert( u"FullUrl"_s, mFullUrl->isChecked() );
  }

  cfg.insert( u"PropertyCollection"_s, mPropertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );

  if ( !mRootPath->text().isEmpty() )
    cfg.insert( u"DefaultRoot"_s, mRootPath->text() );

  if ( !mStorageType->currentIndex() )
  {
    // Save Storage Mode
    cfg.insert( u"StorageMode"_s, mStorageModeCbx->currentData().toInt() );
    // Save Relative Paths option
    cfg.insert( u"RelativeStorage"_s, mStoragePathCbx->currentData().toInt() );
  }
  else
  {
    // Only file mode and absolute paths are supported for external storage
    cfg.insert( u"StorageMode"_s, static_cast<int>( QgsFileWidget::GetFile ) );
    cfg.insert( u"RelativeStorage"_s, static_cast<int>( QgsFileWidget::Absolute ) );
  }

  cfg.insert( u"DocumentViewer"_s, mDocumentViewerContentComboBox->currentData().toInt() );
  cfg.insert( u"DocumentViewerHeight"_s, mDocumentViewerHeight->value() );
  cfg.insert( u"DocumentViewerWidth"_s, mDocumentViewerWidth->value() );

  return cfg;
}


void QgsExternalResourceConfigDlg::setConfig( const QVariantMap &config )
{
  if ( config.contains( u"StorageType"_s ) )
  {
    const int index = mStorageType->findData( config.value( u"StorageType"_s ) );
    if ( index >= 0 )
      mStorageType->setCurrentIndex( index );
  }

  mAuthSettingsProtocol->setConfigId( config.value( u"StorageAuthConfigId"_s ).toString() );
  mStorageUrl->setText( config.value( u"StorageUrl"_s ).toString() );

  if ( config.contains( u"FileWidget"_s ) )
  {
    mFileWidgetGroupBox->setChecked( config.value( u"FileWidget"_s ).toBool() );
  }
  if ( config.contains( u"FileWidget"_s ) )
  {
    mFileWidgetButtonGroupBox->setChecked( config.value( u"FileWidgetButton"_s ).toBool() );
  }
  if ( config.contains( u"FileWidgetFilter"_s ) )
  {
    mFileWidgetFilterLineEdit->setText( config.value( u"FileWidgetFilter"_s ).toString() );
  }

  if ( config.contains( u"UseLink"_s ) )
  {
    mUseLink->setChecked( config.value( u"UseLink"_s ).toBool() );
    if ( config.contains( u"FullUrl"_s ) )
      mFullUrl->setChecked( true );
  }

  mPropertyCollection.loadVariant( config.value( u"PropertyCollection"_s ), QgsWidgetWrapper::propertyDefinitions() );
  updateDataDefinedButtons();

  mRootPath->setText( config.value( u"DefaultRoot"_s ).toString() );

  // relative storage
  if ( config.contains( u"RelativeStorage"_s ) )
  {
    const int relative = config.value( u"RelativeStorage"_s ).toInt();
    mStoragePathCbx->setCurrentIndex( relative );
  }

  // set storage mode
  if ( config.contains( u"StorageMode"_s ) )
  {
    const int mode = config.value( u"StorageMode"_s ).toInt();
    mStorageModeCbx->setCurrentIndex( mode );
  }

  // Document viewer
  if ( config.contains( u"DocumentViewer"_s ) )
  {
    const QgsExternalResourceWidget::DocumentViewerContent content = ( QgsExternalResourceWidget::DocumentViewerContent ) config.value( u"DocumentViewer"_s ).toInt();
    const int idx = mDocumentViewerContentComboBox->findData( content );
    if ( idx >= 0 )
    {
      mDocumentViewerContentComboBox->setCurrentIndex( idx );
    }
    if ( config.contains( u"DocumentViewerHeight"_s ) )
    {
      mDocumentViewerHeight->setValue( config.value( u"DocumentViewerHeight"_s ).toInt() );
    }
    if ( config.contains( u"DocumentViewerWidth"_s ) )
    {
      mDocumentViewerWidth->setValue( config.value( u"DocumentViewerWidth"_s ).toInt() );
    }
  }
}

QgsExpressionContext QgsExternalResourceConfigDlg::createExpressionContext() const
{
  QgsExpressionContext context = QgsEditorConfigWidget::createExpressionContext();
  context << QgsExpressionContextUtils::formScope();
  context << QgsExpressionContextUtils::parentFormScope();

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
