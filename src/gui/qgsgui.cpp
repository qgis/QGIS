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


QgsGui *QgsGui::instance()
{
  static QgsGui *sInstance( new QgsGui() );
  return sInstance;
}

QgsNative *QgsGui::nativePlatformInterface()
{
  return instance()->mNative;
}

QgsEditorWidgetRegistry *QgsGui::editorWidgetRegistry()
{
  return instance()->mEditorWidgetRegistry;
}

QgsSourceSelectProviderRegistry *QgsGui::sourceSelectProviderRegistry()
{
  return instance()->mSourceSelectProviderRegistry;
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

QgsProcessingGuiRegistry *QgsGui::processingGuiRegistry()
{
  return instance()->mProcessingGuiRegistry;
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
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "locale/userLocale" ), QString() ).toString().startsWith( QLatin1String( "en" ) ) )
  {
    return HigMenuTextIsTitleCase | HigDialogTitleIsTitleCase;
  }
  else
  {
    return nullptr;
  }
}

QgsGui::~QgsGui()
{
  delete mProcessingGuiRegistry;
  delete mDataItemGuiProviderRegistry;
  delete mProcessingRecentAlgorithmLog;
  delete mLayoutItemGuiRegistry;
  delete mLayerTreeEmbeddedWidgetRegistry;
  delete mEditorWidgetRegistry;
  delete mMapLayerActionRegistry;
  delete mSourceSelectProviderRegistry;
  delete mShortcutsManager;
  delete mNative;
  delete mWidgetStateHelper;
  delete mProjectStorageGuiRegistry;
  delete mProviderGuiRegistry;
}

QColor QgsGui::sampleColor( QPoint point )
{
  QScreen *screen = findScreenAt( point );
  if ( ! screen )
  {
    return QColor();
  }
  QPixmap snappedPixmap = screen->grabWindow( QApplication::desktop()->winId(), point.x(), point.y(), 1, 1 );
  QImage snappedImage = snappedPixmap.toImage();
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

  // provider gui registry initialize QgsProviderRegistry too
  mProviderGuiRegistry = new QgsProviderGuiRegistry( QgsApplication::pluginPath() );
  mProjectStorageGuiRegistry = new QgsProjectStorageGuiRegistry();
  mDataItemGuiProviderRegistry = new QgsDataItemGuiProviderRegistry();
  mSourceSelectProviderRegistry = new QgsSourceSelectProviderRegistry();

  mProjectStorageGuiRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );
  mDataItemGuiProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );
  mSourceSelectProviderRegistry->initializeFromProviderGuiRegistry( mProviderGuiRegistry );

  mEditorWidgetRegistry = new QgsEditorWidgetRegistry();
  mShortcutsManager = new QgsShortcutsManager();
  mLayerTreeEmbeddedWidgetRegistry = new QgsLayerTreeEmbeddedWidgetRegistry();
  mMapLayerActionRegistry = new QgsMapLayerActionRegistry();
  mLayoutItemGuiRegistry = new QgsLayoutItemGuiRegistry();
  mWidgetStateHelper = new QgsWidgetStateHelper();
  mProcessingRecentAlgorithmLog = new QgsProcessingRecentAlgorithmLog();
  mProcessingGuiRegistry = new QgsProcessingGuiRegistry();
}

bool QgsGui::pythonMacroAllowed( void ( *lambda )(), QgsMessageBar *messageBar )
{
  Qgis::PythonMacroMode macroMode = QgsSettings().enumValue( QStringLiteral( "qgis/enableMacros" ), Qgis::PythonMacroMode::Ask );

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
        QAbstractButton *stopSessionButton = msgBox.addButton( tr( "Don't Ask Anymore" ), QMessageBox::DestructiveRole );
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
            Qgis::Warning,
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
