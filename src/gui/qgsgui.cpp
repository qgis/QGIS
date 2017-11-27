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
#else
#include "qgsnative.h"
#endif
#include "qgsshortcutsmanager.h"

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

QgsGui::~QgsGui()
{
  delete mLayoutItemGuiRegistry;
  delete mLayerTreeEmbeddedWidgetRegistry;
  delete mEditorWidgetRegistry;
  delete mMapLayerActionRegistry;
  delete mSourceSelectProviderRegistry;
  delete mShortcutsManager;
  delete mNative;
}

QgsGui::QgsGui()
{
#ifdef Q_OS_MAC
  mNative = new QgsMacNative();
#else
  mNative = new QgsNative();
#endif

  mEditorWidgetRegistry = new QgsEditorWidgetRegistry();
  mShortcutsManager = new QgsShortcutsManager();
  mLayerTreeEmbeddedWidgetRegistry = new QgsLayerTreeEmbeddedWidgetRegistry();
  mMapLayerActionRegistry = new QgsMapLayerActionRegistry();
  mSourceSelectProviderRegistry = new QgsSourceSelectProviderRegistry();
  mLayoutItemGuiRegistry = new QgsLayoutItemGuiRegistry();
}
