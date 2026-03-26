/***************************************************************************
                         qgsgui.cpp
                         ----------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsgui.h"

#include "callouts/qgscalloutwidget.h"
#include "qgsadvanceddigitizingtoolsregistry.h"
#include "qgsannotationitemguiregistry.h"
#include "qgscalloutsregistry.h"
#include "qgseditorwidgetregistry.h"
#include "qgslayertreeembeddedwidgetregistry.h"
#include "qgslayoutitemguiregistry.h"
#include "qgsmaplayeractionregistry.h"
#include "qgssourceselectproviderregistry.h"

#include <QMessageBox>
#include <QScreen>
#include <QString>

#include "moc_qgsgui.cpp"

using namespace Qt::StringLiterals;

#ifdef Q_OS_MACOS
#include "qgsmacnative.h"
#elif defined( Q_OS_WIN )
#ifndef __MINGW32__
#include "qgswinnative.h"
#else
#include "qgsnative.h"
#endif
#elif defined( Q_OS_LINUX )
#include "qgslinuxnative.h"
#else
#include "qgsnative.h"
#endif
#include "qgsprocessingguiregistry.h"
#include "qgsshortcutsmanager.h"
#include "qgswidgetstatehelper_p.h"
#include "qgslogger.h"
#include "qgsprocessingfavoritealgorithmmanager.h"
#include "qgsprocessingrecentalgorithmlog.h"
#include "qgswindowmanagerinterface.h"
#include "qgssettings.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsproviderguiregistry.h"
#include "qgsproject.h"
#include "qgsprojectstorageguiregistry.h"
#include "qgsprojecttrustdialog.h"
#include "qgsprojectutils.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsnumericformatguiregistry.h"
#include "qgscodeeditorcolorschemeregistry.h"
#include "qgssubsetstringeditorproviderregistry.h"
#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsrelationwidgetregistry.h"
#include "qgsmaptoolshaperegistry.h"
#include "qgssettingsregistrygui.h"
#include "qgshistoryproviderregistry.h"
#include "qgslayermetadatasourceselectprovider.h"
#include "qgsinputcontrollermanager.h"
#include "qgssensorguiregistry.h"
#include "qgshistoryentry.h"
#include "qgsstacsourceselectprovider.h"
#include "qgsstoredquerymanager.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingseditorwidgetregistry.h"
#include "qgssettingsregistrycore.h"
#include "qgsplotregistry.h"
#include "qgsplotwidget.h"

#include <QFileInfo>
#include <QPushButton>
#include <QToolButton>

QgsGui *QgsGui::instance()
{
  static QgsGui *sInstance( new QgsGui() );
  return sInstance;
}

QgsNative *QgsGui::nativePlatformInterface()
{
  return instance()->mNative.get();
}

QgsSettingsRegistryGui *QgsGui::settingsRegistryGui()
{
  return instance()->mSettingsRegistryGui.get();
}

QgsEditorWidgetRegistry *QgsGui::editorWidgetRegistry()
{
  return instance()->mEditorWidgetRegistry.get();
}

QgsRelationWidgetRegistry *QgsGui::relationWidgetRegistry()
{
  return instance()->mRelationEditorRegistry.get();
}

QgsMapToolShapeRegistry *QgsGui::mapToolShapeRegistry()
{
  return instance()->mShapeMapToolRegistry.get();
}

QgsSourceSelectProviderRegistry *QgsGui::sourceSelectProviderRegistry()
{
  return instance()->mSourceSelectProviderRegistry.get();
}

QgsSubsetStringEditorProviderRegistry *QgsGui::subsetStringEditorProviderRegistry()
{
  return instance()->mSubsetStringEditorProviderRegistry.get();
}

QgsProviderSourceWidgetProviderRegistry *QgsGui::sourceWidgetProviderRegistry()
{
  return instance()->mProviderSourceWidgetProviderRegistry.get();
}

QgsShortcutsManager *QgsGui::shortcutsManager()
{
  return instance()->mShortcutsManager.get();
}

QgsLayerTreeEmbeddedWidgetRegistry *QgsGui::layerTreeEmbeddedWidgetRegistry()
{
  return instance()->mLayerTreeEmbeddedWidgetRegistry.get();
}

QgsMapLayerActionRegistry *QgsGui::mapLayerActionRegistry()
{
  return instance()->mMapLayerActionRegistry.get();
}

QgsLayoutItemGuiRegistry *QgsGui::layoutItemGuiRegistry()
{
  return instance()->mLayoutItemGuiRegistry.get();
}

QgsAnnotationItemGuiRegistry *QgsGui::annotationItemGuiRegistry()
{
  return instance()->mAnnotationItemGuiRegistry.get();
}

QgsAdvancedDigitizingToolsRegistry *QgsGui::advancedDigitizingToolsRegistry()
{
  return instance()->mAdvancedDigitizingToolsRegistry.get();
}

QgsProcessingGuiRegistry *QgsGui::processingGuiRegistry()
{
  return instance()->mProcessingGuiRegistry.get();
}

QgsNumericFormatGuiRegistry *QgsGui::numericFormatGuiRegistry()
{
  return instance()->mNumericFormatGuiRegistry.get();
}

QgsCodeEditorColorSchemeRegistry *QgsGui::codeEditorColorSchemeRegistry()
{
  return instance()->mCodeEditorColorSchemeRegistry.get();
}

QgsProcessingFavoriteAlgorithmManager *QgsGui::processingFavoriteAlgorithmManager()
{
  return instance()->mProcessingFavoriteAlgorithmManager.get();
}

QgsProcessingRecentAlgorithmLog *QgsGui::processingRecentAlgorithmLog()
{
  return instance()->mProcessingRecentAlgorithmLog.get();
}

QgsDataItemGuiProviderRegistry *QgsGui::dataItemGuiProviderRegistry()
{
  return instance()->mDataItemGuiProviderRegistry.get();
}

QgsProjectStorageGuiRegistry *QgsGui::projectStorageGuiRegistry()
{
  return instance()->mProjectStorageGuiRegistry.get();
}

QgsProviderGuiRegistry *QgsGui::providerGuiRegistry()
{
  return instance()->mProviderGuiRegistry.get();
}

QgsSensorGuiRegistry *QgsGui::sensorGuiRegistry()
{
  return instance()->mSensorGuiRegistry.get();
}

QgsHistoryProviderRegistry *QgsGui::historyProviderRegistry()
{
  return instance()->mHistoryProviderRegistry.get();
}

QgsSettingsEditorWidgetRegistry *QgsGui::settingsEditorWidgetRegistry()
{
  return instance()->mSettingsEditorRegistry.get();
}

void QgsGui::enableAutoGeometryRestore( QWidget *widget, const QString &key )
{
  if ( widget->objectName().isEmpty() )
  {
    QgsDebugError( u"WARNING: No object name set. Best for it to be set objectName when using QgsGui::enableAutoGeometryRestore"_s );
  }
  instance()->mWidgetStateHelper->registerWidget( widget, key );
}

QgsWindowManagerInterface *QgsGui::windowManager()
{
  return instance()->mWindowManager.get();
}

QgsInputControllerManager *QgsGui::inputControllerManager()
{
  return instance()->mInputControllerManager.get();
}

QgsStoredQueryManager *QgsGui::storedQueryManager()
{
  return instance()->mStoredQueryManager.get();
}

void QgsGui::setWindowManager( QgsWindowManagerInterface *manager )
{
  instance()->mWindowManager.reset( manager );
}

QgsGui::HigFlags QgsGui::higFlags()
{
  if ( QgsApplication::settingsLocaleUserLocale->value().startsWith( "en"_L1 ) )
  {
    return HigMenuTextIsTitleCase | HigDialogTitleIsTitleCase;
  }
  else
  {
    return QgsGui::HigFlags();
  }
}

QgsGui::~QgsGui()
{
  // we reset explicit registry because order matters
  mProcessingGuiRegistry.reset();
  mDataItemGuiProviderRegistry.reset();
  mProcessingFavoriteAlgorithmManager.reset();
  mProcessingRecentAlgorithmLog.reset();
  mLayoutItemGuiRegistry.reset();
  mAnnotationItemGuiRegistry.reset();
  mAdvancedDigitizingToolsRegistry.reset();
  mLayerTreeEmbeddedWidgetRegistry.reset();
  mEditorWidgetRegistry.reset();
  mMapLayerActionRegistry.reset();
  mSourceSelectProviderRegistry.reset();
  mHistoryProviderRegistry.reset();
  mShortcutsManager.reset();
  mNative.reset();
  mNumericFormatGuiRegistry.reset();
  mWidgetStateHelper.reset();
  mProjectStorageGuiRegistry.reset();
  mProviderGuiRegistry.reset();
  mCodeEditorColorSchemeRegistry.reset();
  mSubsetStringEditorProviderRegistry.reset();
  mProviderSourceWidgetProviderRegistry.reset();
  mShapeMapToolRegistry.reset();
  mRelationEditorRegistry.reset();
  mInputControllerManager.reset();
  mSettingsRegistryGui.reset();
  mSensorGuiRegistry.reset();
  mStoredQueryManager.reset();
  mSettingsEditorRegistry.reset();
}

QColor QgsGui::sampleColor( QPoint point )
{
  QScreen *screen = findScreenAt( point );
  if ( !screen )
  {
    return QColor();
  }

  const int x = point.x() - screen->geometry().left();
  const int y = point.y() - screen->geometry().top();
  const QPixmap snappedPixmap = screen->grabWindow( 0, x, y, 1, 1 );
  const QImage snappedImage = snappedPixmap.toImage();
  return snappedImage.pixel( 0, 0 );
}

QScreen *QgsGui::findScreenAt( QPoint point )
{
  const QList<QScreen *> screens = QGuiApplication::screens();
  for ( QScreen *screen : screens )
  {
    if ( screen->geometry().contains( point ) )
    {
      return screen;
    }
  }
  return nullptr;
}

QgsGui::QgsGui()
{
#ifdef Q_OS_MAC
  auto macNative = std::make_unique<QgsMacNative>();
  macNative->setIconPath( QgsApplication::iconsPath() + u"qgis-icon-macos.png"_s );
  mNative = std::move( macNative );
#elif defined( Q_OS_WIN )
#ifndef __MINGW32__
  mNative = std::make_unique<QgsWinNative>();
#else
  mNative = std::make_unique<QgsNative>();
#endif
#elif defined( Q_OS_LINUX )
  mNative = std::make_unique<QgsLinuxNative>();
#else
  mNative = std::make_unique<QgsNative>();
#endif

  mSettingsRegistryGui = std::make_unique<QgsSettingsRegistryGui>();

  mSettingsEditorRegistry = std::make_unique<QgsSettingsEditorWidgetRegistry>();

  mStoredQueryManager = std::make_unique<QgsStoredQueryManager>();
  mCodeEditorColorSchemeRegistry = std::make_unique<QgsCodeEditorColorSchemeRegistry>();

  // provider gui registry initialize QgsProviderRegistry too
  mSensorGuiRegistry = std::make_unique<QgsSensorGuiRegistry>();
  mSensorGuiRegistry->populate();

  mHistoryProviderRegistry = std::make_unique<QgsHistoryProviderRegistry>();
  mHistoryProviderRegistry->addDefaultProviders();

  mInputControllerManager = std::make_unique<QgsInputControllerManager>();

  mProviderGuiRegistry = std::make_unique<QgsProviderGuiRegistry>( QgsApplication::pluginPath() );
  mProjectStorageGuiRegistry = std::make_unique<QgsProjectStorageGuiRegistry>();
  mDataItemGuiProviderRegistry = std::make_unique<QgsDataItemGuiProviderRegistry>();
  mSourceSelectProviderRegistry = std::make_unique<QgsSourceSelectProviderRegistry>();
  mNumericFormatGuiRegistry = std::make_unique<QgsNumericFormatGuiRegistry>();
  mSubsetStringEditorProviderRegistry = std::make_unique<QgsSubsetStringEditorProviderRegistry>();
  mProviderSourceWidgetProviderRegistry = std::make_unique<QgsProviderSourceWidgetProviderRegistry>();

  mProjectStorageGuiRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry.get() );
  mDataItemGuiProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry.get() );
  mSourceSelectProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry.get() );
  mSourceSelectProviderRegistry->addProvider( new QgsLayerMetadataSourceSelectProvider() );
  mSourceSelectProviderRegistry->addProvider( new QgsStacSourceSelectProvider() );
  mSubsetStringEditorProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry.get() );
  mProviderSourceWidgetProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry.get() );

  mEditorWidgetRegistry = std::make_unique<QgsEditorWidgetRegistry>();
  mRelationEditorRegistry = std::make_unique<QgsRelationWidgetRegistry>();
  mShapeMapToolRegistry = std::make_unique<QgsMapToolShapeRegistry>();
  mShortcutsManager = std::make_unique<QgsShortcutsManager>();
  mLayerTreeEmbeddedWidgetRegistry = std::make_unique<QgsLayerTreeEmbeddedWidgetRegistry>();
  mMapLayerActionRegistry = std::make_unique<QgsMapLayerActionRegistry>();
  mLayoutItemGuiRegistry = std::make_unique<QgsLayoutItemGuiRegistry>();

  mAnnotationItemGuiRegistry = std::make_unique<QgsAnnotationItemGuiRegistry>();
  mAnnotationItemGuiRegistry->addDefaultItems();

  mAdvancedDigitizingToolsRegistry = std::make_unique<QgsAdvancedDigitizingToolsRegistry>();
  mAdvancedDigitizingToolsRegistry->addDefaultTools();

  mWidgetStateHelper = std::make_unique<QgsWidgetStateHelper>();
  mProcessingFavoriteAlgorithmManager = std::make_unique<QgsProcessingFavoriteAlgorithmManager>();
  mProcessingRecentAlgorithmLog = std::make_unique<QgsProcessingRecentAlgorithmLog>();
  mProcessingGuiRegistry = std::make_unique<QgsProcessingGuiRegistry>();

  qRegisterMetaType<QgsHistoryEntry>( "QgsHistoryEntry" );
}

bool QgsGui::allowExecutionOfEmbeddedScripts( QgsProject *project, QgsMessageBar *messageBar )
{
  const Qgis::EmbeddedScriptMode embeddedScriptMode = QgsSettingsRegistryCore::settingsCodeExecutionBehaviorUndeterminedProjects->value();
  Qgis::ProjectTrustStatus trustStatus = QgsProjectUtils::checkUserTrust( project );
  if ( trustStatus == Qgis::ProjectTrustStatus::Undetermined && embeddedScriptMode == Qgis::EmbeddedScriptMode::Ask )
  {
    QgsProjectTrustDialog dialog( project );
    dialog.exec();
    trustStatus = QgsProjectUtils::checkUserTrust( project );
  }

  if ( messageBar )
  {
    if ( trustStatus == Qgis::ProjectTrustStatus::Trusted )
    {
      messageBar
        ->pushMessage( tr( "Security warning" ), tr( "The loaded project contains embedded scripts which have been allowed execution." ), embeddedScriptMode == Qgis::EmbeddedScriptMode::Always ? Qgis::MessageLevel::Warning : Qgis::MessageLevel::Info );
    }
    else
    {
      messageBar
        ->pushMessage( tr( "Security warning" ), tr( "The loaded project contains embedded scripts which have been denied execution." ), embeddedScriptMode == Qgis::EmbeddedScriptMode::Never ? Qgis::MessageLevel::Warning : Qgis::MessageLevel::Info );
    }
  }

  return trustStatus == Qgis::ProjectTrustStatus::Trusted;
}

void QgsGui::initCalloutWidgets()
{
  static std::once_flag initialized;
  std::call_once( initialized, []() {
    auto _initCalloutWidgetFunction = []( const QString &name, QgsCalloutWidgetFunc f ) {
      QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();

      QgsCalloutAbstractMetadata *abstractMetadata = registry->calloutMetadata( name );
      if ( !abstractMetadata )
      {
        QgsDebugError( u"Failed to find callout entry in registry: %1"_s.arg( name ) );
      }
      QgsCalloutMetadata *metadata = dynamic_cast<QgsCalloutMetadata *>( abstractMetadata );
      if ( !metadata )
      {
        QgsDebugError( u"Failed to cast callout's metadata: "_s.arg( name ) );
      }
      else
      {
        metadata->setWidgetFunction( f );
      }
    };

    _initCalloutWidgetFunction( u"simple"_s, QgsSimpleLineCalloutWidget::create );
    _initCalloutWidgetFunction( u"manhattan"_s, QgsManhattanLineCalloutWidget::create );
    _initCalloutWidgetFunction( u"curved"_s, QgsCurvedLineCalloutWidget::create );
    _initCalloutWidgetFunction( u"balloon"_s, QgsBalloonCalloutWidget::create );
  } );
}

void QgsGui::initPlotWidgets()
{
  static std::once_flag initialized;
  std::call_once( initialized, []() {
    auto _initPlotWidgetFunction = []( const QString &name, QgsPlotWidgetCreateFunc f ) {
      QgsPlotRegistry *registry = QgsApplication::plotRegistry();

      QgsPlotAbstractMetadata *abstractMetadata = registry->plotMetadata( name );
      if ( !abstractMetadata )
      {
        QgsDebugError( u"Failed to find plot entry in registry: %1"_s.arg( name ) );
      }
      QgsPlotMetadata *metadata = dynamic_cast<QgsPlotMetadata *>( abstractMetadata );
      if ( !metadata )
      {
        QgsDebugError( u"Failed to cast plot's metadata: "_s.arg( name ) );
      }
      else
      {
        metadata->setWidgetCreateFunction( std::move( f ) );
      }
    };

    _initPlotWidgetFunction( u"bar"_s, QgsBarChartPlotWidget::create );
    _initPlotWidgetFunction( u"line"_s, QgsLineChartPlotWidget::create );
    _initPlotWidgetFunction( u"pie"_s, QgsPieChartPlotWidget::create );
  } );
}

bool QgsGui::hasWebEngine()
{
#ifdef HAVE_WEBENGINE
  return true;
#else
  return false;
#endif
}

///@cond PRIVATE
void QgsGui::emitOptionsChanged()
{
  emit optionsChanged();
}
///@endcond
