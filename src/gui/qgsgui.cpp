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


#include <QScreen>
#include <QMessageBox>

#include "qgsgui.h"
#include "moc_qgsgui.cpp"
#include "qgseditorwidgetregistry.h"
#include "qgslayertreeembeddedwidgetregistry.h"
#include "qgsmaplayeractionregistry.h"
#include "qgssourceselectproviderregistry.h"
#include "qgslayoutitemguiregistry.h"
#include "qgsannotationitemguiregistry.h"
#include "qgsadvanceddigitizingtoolsregistry.h"
#include "qgscalloutsregistry.h"
#include "callouts/qgscalloutwidget.h"
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
#include "qgsprojectstorageguiregistry.h"
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

#include "qgssettingseditorwidgetregistry.h"


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
    QgsDebugError( QStringLiteral( "WARNING: No object name set. Best for it to be set objectName when using QgsGui::enableAutoGeometryRestore" ) );
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

void QgsGui::setWindowManager( QgsWindowManagerInterface *manager )
{
  instance()->mWindowManager.reset( manager );
}

QgsGui::HigFlags QgsGui::higFlags()
{
  if ( QgsApplication::settingsLocaleUserLocale->value().startsWith( QLatin1String( "en" ) ) )
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
  macNative->setIconPath( QgsApplication::iconsPath() + QStringLiteral( "qgis-icon-macos.png" ) );
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

bool QgsGui::pythonEmbeddedInProjectAllowed( void ( *lambda )(), QgsMessageBar *messageBar, Qgis::PythonEmbeddedType embeddedType )
{
  const Qgis::PythonEmbeddedMode pythonEmbeddedMode = QgsSettings().enumValue( QStringLiteral( "qgis/enablePythonEmbedded" ), Qgis::PythonEmbeddedMode::Ask );

  switch ( pythonEmbeddedMode )
  {
    case Qgis::PythonEmbeddedMode::SessionOnly:
    case Qgis::PythonEmbeddedMode::Always:
      if ( embeddedType == Qgis::PythonEmbeddedType::Macro )
      {
        if ( lambda )
          lambda();
      }
      // If this is the case, expression functions
      // are loaded directly by the QGIS project.
      return true;
    case Qgis::PythonEmbeddedMode::Never:
    case Qgis::PythonEmbeddedMode::NotForThisSession:
      if ( messageBar )
      {
        switch ( embeddedType )
        {
          case Qgis::PythonEmbeddedType::Macro:
            messageBar->pushMessage( tr( "Python Macros" ), tr( "Python macros are currently disabled and will not be run" ), Qgis::MessageLevel::Warning );
            break;
          case Qgis::PythonEmbeddedType::ExpressionFunction:
            messageBar->pushMessage( tr( "Python Expressions" ), tr( "Python expressions from project are currently disabled and will not be loaded" ), Qgis::MessageLevel::Warning );
            break;
        }
      }
      return false;
    case Qgis::PythonEmbeddedMode::Ask:
      if ( embeddedType == Qgis::PythonEmbeddedType::Macro )
      {
        if ( !lambda )
        {
          QMessageBox msgBox( QMessageBox::Information, tr( "Python Macros" ), tr( "Python macros are currently disabled. Do you allow this macro to run?" ) );
          QAbstractButton *stopSessionButton = msgBox.addButton( tr( "Disable for this Session" ), QMessageBox::DestructiveRole );
          msgBox.addButton( tr( "No" ), QMessageBox::NoRole );
          QAbstractButton *yesButton = msgBox.addButton( tr( "Yes" ), QMessageBox::YesRole );
          msgBox.exec();

          QAbstractButton *clicked = msgBox.clickedButton();
          if ( clicked == stopSessionButton )
          {
            QgsSettings().setEnumValue( QStringLiteral( "qgis/enablePythonEmbedded" ), Qgis::PythonEmbeddedMode::NotForThisSession );
          }
          return clicked == yesButton;
        }
        else
        {
          // create the notification widget for macros
          Q_ASSERT( messageBar );
          if ( messageBar )
          {
            QToolButton *btnEnableMacros = new QToolButton();
            btnEnableMacros->setText( tr( "Enable Macros" ) );
            btnEnableMacros->setStyleSheet( QStringLiteral( "background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;" ) );
            btnEnableMacros->setCursor( Qt::PointingHandCursor );
            btnEnableMacros->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

            QgsMessageBarItem *macroMsg = new QgsMessageBarItem(
              tr( "Security warning" ),
              tr( "Python macros cannot currently be run." ),
              btnEnableMacros,
              Qgis::MessageLevel::Warning,
              0,
              messageBar
            );

            connect( btnEnableMacros, &QToolButton::clicked, messageBar, [=]() {
              lambda();
              messageBar->popWidget( macroMsg );
            } );

            // display the macros notification widget
            messageBar->pushItem( macroMsg );
          }

          return false;
        }
      }
      else if ( embeddedType == Qgis::PythonEmbeddedType::ExpressionFunction )
      {
        // create the notification widget for expressions from project
        Q_ASSERT( messageBar );
        if ( messageBar )
        {
          QToolButton *btnEnableExpressionsFromProject = new QToolButton();
          btnEnableExpressionsFromProject->setText( tr( "Enable python expressions from project" ) );
          btnEnableExpressionsFromProject->setStyleSheet( QStringLiteral( "background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;" ) );
          btnEnableExpressionsFromProject->setCursor( Qt::PointingHandCursor );
          btnEnableExpressionsFromProject->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

          QgsMessageBarItem *expressionFromProjectMsg = new QgsMessageBarItem(
            tr( "Security warning" ),
            tr( "Python expressions from project cannot currently be loaded." ),
            btnEnableExpressionsFromProject,
            Qgis::MessageLevel::Warning,
            0,
            messageBar
          );

          connect( btnEnableExpressionsFromProject, &QToolButton::clicked, messageBar, [=]() {
            QgsProject::instance()->loadFunctionsFromProject( true );
            messageBar->popWidget( expressionFromProjectMsg );
          } );

          // display the notification widget
          messageBar->pushItem( expressionFromProjectMsg );
        }

        return false;
      }
  }
  return false;
}

void QgsGui::initCalloutWidgets()
{
  static std::once_flag initialized;
  std::call_once( initialized, [=]() {
    auto _initCalloutWidgetFunction = []( const QString &name, QgsCalloutWidgetFunc f ) {
      QgsCalloutRegistry *registry = QgsApplication::calloutRegistry();

      QgsCalloutAbstractMetadata *abstractMetadata = registry->calloutMetadata( name );
      if ( !abstractMetadata )
      {
        QgsDebugError( QStringLiteral( "Failed to find callout entry in registry: %1" ).arg( name ) );
      }
      QgsCalloutMetadata *metadata = dynamic_cast<QgsCalloutMetadata *>( abstractMetadata );
      if ( !metadata )
      {
        QgsDebugError( QStringLiteral( "Failed to cast callout's metadata: " ).arg( name ) );
      }
      else
      {
        metadata->setWidgetFunction( f );
      }
    };

    _initCalloutWidgetFunction( QStringLiteral( "simple" ), QgsSimpleLineCalloutWidget::create );
    _initCalloutWidgetFunction( QStringLiteral( "manhattan" ), QgsManhattanLineCalloutWidget::create );
    _initCalloutWidgetFunction( QStringLiteral( "curved" ), QgsCurvedLineCalloutWidget::create );
    _initCalloutWidgetFunction( QStringLiteral( "balloon" ), QgsBalloonCalloutWidget::create );
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
