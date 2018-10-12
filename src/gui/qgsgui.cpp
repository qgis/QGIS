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

QgsGui::~QgsGui()
{
  delete mProcessingGuiRegistry;
  delete mProcessingRecentAlgorithmLog;
  delete mLayoutItemGuiRegistry;
  delete mLayerTreeEmbeddedWidgetRegistry;
  delete mEditorWidgetRegistry;
  delete mMapLayerActionRegistry;
  delete mSourceSelectProviderRegistry;
  delete mShortcutsManager;
  delete mNative;
  delete mWidgetStateHelper;
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

  mEditorWidgetRegistry = new QgsEditorWidgetRegistry();
  mShortcutsManager = new QgsShortcutsManager();
  mLayerTreeEmbeddedWidgetRegistry = new QgsLayerTreeEmbeddedWidgetRegistry();
  mMapLayerActionRegistry = new QgsMapLayerActionRegistry();
  mSourceSelectProviderRegistry = new QgsSourceSelectProviderRegistry();
  mLayoutItemGuiRegistry = new QgsLayoutItemGuiRegistry();
  mWidgetStateHelper = new QgsWidgetStateHelper();
  mProcessingRecentAlgorithmLog = new QgsProcessingRecentAlgorithmLog();
  mProcessingGuiRegistry = new QgsProcessingGuiRegistry();
}
