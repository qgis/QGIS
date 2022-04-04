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
#include <QDesktopWidget>
#include <QMessageBox>

#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"
#include "qgslayertreeembeddedwidgetregistry.h"
#include "qgsmaplayeractionregistry.h"
#include "qgssourceselectproviderregistry.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutviewrubberband.h"
#include "qgsannotationitemguiregistry.h"
#ifdef Q_OS_MACX
#include "qgsmacnative.h"
#elif defined (Q_OS_WIN)
#ifndef __MINGW32__
#include "qgswinnative.h"
#else
#include "qgsnative.h"
#endif
#elif defined (Q_OS_LINUX)
#include "qgslinuxnative.h"
#else
#include "qgsnative.h"
#endif
#include "qgsprocessingguiregistry.h"
#include "qgsshortcutsmanager.h"
#include "qgswidgetstatehelper_p.h"
#include "qgslogger.h"
#include "qgsprocessingrecentalgorithmlog.h"
#include "qgswindowmanagerinterface.h"
#include "qgssettings.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsgdalguiprovider.h"
#include "qgsogrguiprovider.h"
#include "qgsproviderregistry.h"
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

QgsHistoryProviderRegistry *QgsGui::historyProviderRegistry()
{
  return instance()->mHistoryProviderRegistry;
}

void QgsGui::enableAutoGeometryRestore( QWidget *widget, const QString &key )
{
  if ( widget->objectName().isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "WARNING: No object name set. Best for it to be set objectName when using QgsGui::enableAutoGeometryRestore" ) );
  }
  instance()->mWidgetStateHelper->registerWidget( widget, key );
}

QgsWindowManagerInterface *QgsGui::windowManager()
{
  return instance()->mWindowManager.get();
}

void QgsGui::setWindowManager( QgsWindowManagerInterface *manager )
{
  instance()->mWindowManager.reset( manager );
}

QgsGui::HigFlags QgsGui::higFlags()
{
  if ( QgsApplication::settingsLocaleUserLocale.value().startsWith( QLatin1String( "en" ) ) )
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
  delete mProcessingRecentAlgorithmLog;
  delete mLayoutItemGuiRegistry;
  delete mAnnotationItemGuiRegistry;
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
  delete mSettingsRegistryGui;
}

QColor QgsGui::sampleColor( QPoint point )
{
  QScreen *screen = findScreenAt( point );
  if ( ! screen )
  {
    return QColor();
  }
  const QPixmap snappedPixmap = screen->grabWindow( QApplication::desktop()->winId(), point.x(), point.y(), 1, 1 );
  const QImage snappedImage = snappedPixmap.toImage();
  return snappedImage.pixel( 0, 0 );
}

QScreen *QgsGui::findScreenAt( QPoint point )
{
  const QList< QScreen * > screens = QGuiApplication::screens();
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
#elif defined (Q_OS_WIN)
#ifndef __MINGW32__
  mNative = new QgsWinNative();
#else
  mNative = new QgsNative();
#endif
#elif defined(Q_OS_LINUX)
  mNative = new QgsLinuxNative();
#else
  mNative = new QgsNative();
#endif

  mSettingsRegistryGui = new QgsSettingsRegistryGui();

  mCodeEditorColorSchemeRegistry = new QgsCodeEditorColorSchemeRegistry();

  // provider gui registry initialize QgsProviderRegistry too
  mHistoryProviderRegistry = new QgsHistoryProviderRegistry();
  mHistoryProviderRegistry->addDefaultProviders();

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

  mWidgetStateHelper = new QgsWidgetStateHelper();
  mProcessingRecentAlgorithmLog = new QgsProcessingRecentAlgorithmLog();
  mProcessingGuiRegistry = new QgsProcessingGuiRegistry();
}

bool QgsGui::pythonMacroAllowed( void ( *lambda )(), QgsMessageBar *messageBar )
{
  const Qgis::PythonMacroMode macroMode = QgsSettings().enumValue( QStringLiteral( "qgis/enableMacros" ), Qgis::PythonMacroMode::Ask );

  switch ( macroMode )
  {
    case Qgis::PythonMacroMode::SessionOnly:
    case Qgis::PythonMacroMode::Always:
      if ( lambda )
        lambda();
      return true;
    case Qgis::PythonMacroMode::Never:
    case Qgis::PythonMacroMode::NotForThisSession:
      if ( messageBar )
      {
        messageBar->pushMessage( tr( "Python Macros" ),
                                 tr( "Python macros are currently disabled and will not be run" ),
                                 Qgis::MessageLevel::Warning );
      }
      return false;
    case Qgis::PythonMacroMode::Ask:
      if ( !lambda )
      {
        QMessageBox msgBox( QMessageBox::Information, tr( "Python Macros" ),
                            tr( "Python macros are currently disabled. Do you allow this macro to run?" ) );
        QAbstractButton *stopSessionButton = msgBox.addButton( tr( "Disable for this Session" ), QMessageBox::DestructiveRole );
        msgBox.addButton( tr( "No" ), QMessageBox::NoRole );
        QAbstractButton *yesButton = msgBox.addButton( tr( "Yes" ), QMessageBox::YesRole );
        msgBox.exec();

        QAbstractButton *clicked = msgBox.clickedButton();
        if ( clicked == stopSessionButton )
        {
          QgsSettings().setEnumValue( QStringLiteral( "qgis/enableMacros" ), Qgis::PythonMacroMode::NotForThisSession );
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
            messageBar );

          connect( btnEnableMacros, &QToolButton::clicked, messageBar, [ = ]()
          {
            lambda();
            messageBar->popWidget( macroMsg );
          } );

          // display the macros notification widget
          messageBar->pushItem( macroMsg );
        }

        return false;
      }
  }
  return false;
}

///@cond PRIVATE
void QgsGui::emitOptionsChanged()
{
  emit optionsChanged();
}
///@endcond
