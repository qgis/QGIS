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

#include "moc_qgsgui.cpp"

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
  return instance()->mNative;
}

QgsSettingsRegistryGui *QgsGui::settingsRegistryGui()
{
  return instance()->mSettingsRegistryGui;
}

QgsEditorWidgetRegistry *QgsGui::editorWidgetRegistry()
{
  return instance()->mEditorWidgetRegistry;
}

QgsRelationWidgetRegistry *QgsGui::relationWidgetRegistry()
{
  return instance()->mRelationEditorRegistry;
}

QgsMapToolShapeRegistry *QgsGui::mapToolShapeRegistry()
{
  return instance()->mShapeMapToolRegistry;
}

QgsSourceSelectProviderRegistry *QgsGui::sourceSelectProviderRegistry()
{
  return instance()->mSourceSelectProviderRegistry;
}

QgsSubsetStringEditorProviderRegistry *QgsGui::subsetStringEditorProviderRegistry()
{
  return instance()->mSubsetStringEditorProviderRegistry;
}

QgsProviderSourceWidgetProviderRegistry *QgsGui::sourceWidgetProviderRegistry()
{
  return instance()->mProviderSourceWidgetProviderRegistry;
}

QgsShortcutsManager *QgsGui::shortcutsManager()
{
  return instance()->mShortcutsManager;
}

QgsLayerTreeEmbeddedWidgetRegistry *QgsGui::layerTreeEmbeddedWidgetRegistry()
{
  return instance()->mLayerTreeEmbeddedWidgetRegistry;
}

QgsMapLayerActionRegistry *QgsGui::mapLayerActionRegistry()
{
  return instance()->mMapLayerActionRegistry;
}

QgsLayoutItemGuiRegistry *QgsGui::layoutItemGuiRegistry()
{
  return instance()->mLayoutItemGuiRegistry;
}

QgsAnnotationItemGuiRegistry *QgsGui::annotationItemGuiRegistry()
{
  return instance()->mAnnotationItemGuiRegistry;
}

QgsAdvancedDigitizingToolsRegistry *QgsGui::advancedDigitizingToolsRegistry()
{
  return instance()->mAdvancedDigitizingToolsRegistry;
}

QgsProcessingGuiRegistry *QgsGui::processingGuiRegistry()
{
  return instance()->mProcessingGuiRegistry;
}

QgsNumericFormatGuiRegistry *QgsGui::numericFormatGuiRegistry()
{
  return instance()->mNumericFormatGuiRegistry;
}

QgsCodeEditorColorSchemeRegistry *QgsGui::codeEditorColorSchemeRegistry()
{
  return instance()->mCodeEditorColorSchemeRegistry;
}

QgsProcessingFavoriteAlgorithmManager *QgsGui::processingFavoriteAlgorithmManager()
{
  return instance()->mProcessingFavoriteAlgorithmManager;
}

QgsProcessingRecentAlgorithmLog *QgsGui::processingRecentAlgorithmLog()
{
  return instance()->mProcessingRecentAlgorithmLog;
}

QgsDataItemGuiProviderRegistry *QgsGui::dataItemGuiProviderRegistry()
{
  return instance()->mDataItemGuiProviderRegistry;
}

QgsProjectStorageGuiRegistry *QgsGui::projectStorageGuiRegistry()
{
  return instance()->mProjectStorageGuiRegistry;
}

QgsProviderGuiRegistry *QgsGui::providerGuiRegistry()
{
  return instance()->mProviderGuiRegistry;
}

QgsSensorGuiRegistry *QgsGui::sensorGuiRegistry()
{
  return instance()->mSensorGuiRegistry;
}

QgsHistoryProviderRegistry *QgsGui::historyProviderRegistry()
{
  return instance()->mHistoryProviderRegistry;
}

QgsSettingsEditorWidgetRegistry *QgsGui::settingsEditorWidgetRegistry()
{
  return instance()->mSettingsEditorRegistry;
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
  return instance()->mInputControllerManager;
}

QgsStoredQueryManager *QgsGui::storedQueryManager()
{
  return instance()->mStoredQueryManager;
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
  delete mProcessingGuiRegistry;
  delete mDataItemGuiProviderRegistry;
  delete mProcessingFavoriteAlgorithmManager;
  delete mProcessingRecentAlgorithmLog;
  delete mLayoutItemGuiRegistry;
  delete mAnnotationItemGuiRegistry;
  delete mAdvancedDigitizingToolsRegistry;
  delete mLayerTreeEmbeddedWidgetRegistry;
  delete mEditorWidgetRegistry;
  delete mMapLayerActionRegistry;
  delete mSourceSelectProviderRegistry;
  delete mHistoryProviderRegistry;
  delete mShortcutsManager;
  delete mNative;
  delete mNumericFormatGuiRegistry;
  delete mWidgetStateHelper;
  delete mProjectStorageGuiRegistry;
  delete mProviderGuiRegistry;
  delete mCodeEditorColorSchemeRegistry;
  delete mSubsetStringEditorProviderRegistry;
  delete mProviderSourceWidgetProviderRegistry;
  delete mShapeMapToolRegistry;
  delete mRelationEditorRegistry;
  delete mInputControllerManager;
  delete mSettingsRegistryGui;
  delete mSensorGuiRegistry;
  delete mStoredQueryManager;
  delete mSettingsEditorRegistry;
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
  QgsMacNative *macNative = new QgsMacNative();
  macNative->setIconPath( QgsApplication::iconsPath() + u"qgis-icon-macos.png"_s );
  mNative = macNative;
#elif defined( Q_OS_WIN )
#ifndef __MINGW32__
  mNative = new QgsWinNative();
#else
  mNative = new QgsNative();
#endif
#elif defined( Q_OS_LINUX )
  mNative = new QgsLinuxNative();
#else
  mNative = new QgsNative();
#endif

  mSettingsRegistryGui = new QgsSettingsRegistryGui();

  mSettingsEditorRegistry = new QgsSettingsEditorWidgetRegistry();

  mStoredQueryManager = new QgsStoredQueryManager();
  mCodeEditorColorSchemeRegistry = new QgsCodeEditorColorSchemeRegistry();

  // provider gui registry initialize QgsProviderRegistry too
  mSensorGuiRegistry = new QgsSensorGuiRegistry();
  mSensorGuiRegistry->populate();

  mHistoryProviderRegistry = new QgsHistoryProviderRegistry();
  mHistoryProviderRegistry->addDefaultProviders();

  mInputControllerManager = new QgsInputControllerManager();

  mProviderGuiRegistry = new QgsProviderGuiRegistry( QgsApplication::pluginPath() );
  mProjectStorageGuiRegistry = new QgsProjectStorageGuiRegistry();
  mDataItemGuiProviderRegistry = new QgsDataItemGuiProviderRegistry();
  mSourceSelectProviderRegistry = new QgsSourceSelectProviderRegistry();
  mNumericFormatGuiRegistry = new QgsNumericFormatGuiRegistry();
  mSubsetStringEditorProviderRegistry = new QgsSubsetStringEditorProviderRegistry();
  mProviderSourceWidgetProviderRegistry = new QgsProviderSourceWidgetProviderRegistry();

  mProjectStorageGuiRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );
  mDataItemGuiProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );
  mSourceSelectProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );
  mSourceSelectProviderRegistry->addProvider( new QgsLayerMetadataSourceSelectProvider() );
  mSourceSelectProviderRegistry->addProvider( new QgsStacSourceSelectProvider() );
  mSubsetStringEditorProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );
  mProviderSourceWidgetProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );

  mEditorWidgetRegistry = new QgsEditorWidgetRegistry();
  mRelationEditorRegistry = new QgsRelationWidgetRegistry();
  mShapeMapToolRegistry = new QgsMapToolShapeRegistry();
  mShortcutsManager = new QgsShortcutsManager();
  mLayerTreeEmbeddedWidgetRegistry = new QgsLayerTreeEmbeddedWidgetRegistry();
  mMapLayerActionRegistry = new QgsMapLayerActionRegistry();
  mLayoutItemGuiRegistry = new QgsLayoutItemGuiRegistry();

  mAnnotationItemGuiRegistry = new QgsAnnotationItemGuiRegistry();
  mAnnotationItemGuiRegistry->addDefaultItems();

  mAdvancedDigitizingToolsRegistry = new QgsAdvancedDigitizingToolsRegistry();
  mAdvancedDigitizingToolsRegistry->addDefaultTools();

  mWidgetStateHelper = new QgsWidgetStateHelper();
  mProcessingFavoriteAlgorithmManager = new QgsProcessingFavoriteAlgorithmManager();
  mProcessingRecentAlgorithmLog = new QgsProcessingRecentAlgorithmLog();
  mProcessingGuiRegistry = new QgsProcessingGuiRegistry();

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
      messageBar->pushMessage(
        tr( "Security warning" ),
        tr( "The loaded project contains embedded scripts which have been allowed execution." ),
        embeddedScriptMode == Qgis::EmbeddedScriptMode::Always ? Qgis::MessageLevel::Warning : Qgis::MessageLevel::Info
      );
    }
    else
    {
      messageBar->pushMessage(
        tr( "Security warning" ),
        tr( "The loaded project contains embedded scripts which have been denied execution." ),
        embeddedScriptMode == Qgis::EmbeddedScriptMode::Never ? Qgis::MessageLevel::Warning : Qgis::MessageLevel::Info
      );
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
