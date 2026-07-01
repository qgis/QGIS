/***************************************************************************
    qgsaiagentpolicy.h
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIAGENTPOLICY_H
#define QGSAIAGENTPOLICY_H

#include "qgis_app.h"

#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>

using namespace Qt::StringLiterals;

struct APP_EXPORT QgsAiManagedAgentPreset
{
    QString mode;
    QString label;
    QStringList allowedTools;
    QStringList allowedModels;
};

struct APP_EXPORT QgsAiManagedAgentPolicy
{
    QString tier;
    QStringList modes;
    QStringList allowedTools;
    QStringList allowedModels;
    QList<QgsAiManagedAgentPreset> presets;

    bool isEmpty() const { return tier.isEmpty() && modes.isEmpty() && allowedTools.isEmpty() && allowedModels.isEmpty() && presets.isEmpty(); }

    QStringList allowedToolsForPreset( const QString &presetMode ) const
    {
      for ( const QgsAiManagedAgentPreset &preset : presets )
      {
        if ( preset.mode == presetMode )
          return preset.allowedTools;
      }
      return allowedTools;
    }

    QStringList allowedModelsForPreset( const QString &presetMode ) const
    {
      for ( const QgsAiManagedAgentPreset &preset : presets )
      {
        if ( preset.mode == presetMode )
          return preset.allowedModels;
      }
      return allowedModels;
    }
};

inline QString QgsAiRuntimeModeForAgent( const QString &agentName )
{
  if ( agentName == "planner"_L1 )
    return u"plan"_s;
  if ( agentName == "reviewer"_L1 )
    return u"ask"_s;
  if ( agentName == "ask_before_edits"_L1 )
    return u"ask_before_edits"_s;
  return u"auto_edit"_s;
}

inline QString QgsAiPresetModeForAgent( const QString &agentName )
{
  if ( agentName == "planner"_L1 )
    return u"planner"_s;
  if ( agentName == "reviewer"_L1 || agentName == "ask_before_edits"_L1 )
    return u"reviewer"_s;
  return u"editor"_s;
}

Q_DECLARE_METATYPE( QgsAiManagedAgentPreset )
Q_DECLARE_METATYPE( QgsAiManagedAgentPolicy )
Q_DECLARE_METATYPE( QList<QgsAiManagedAgentPreset> )

#endif // QGSAIAGENTPOLICY_H
