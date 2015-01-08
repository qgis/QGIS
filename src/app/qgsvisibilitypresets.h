/***************************************************************************
  qgsvisibilitypresets.h
  --------------------------------------
  Date                 : September 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVISIBILITYPRESETS_H
#define QGSVISIBILITYPRESETS_H

#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>

class QAction;
class QDomDocument;
class QMenu;

class QgsLayerTreeNode;
class QgsLayerTreeGroup;

/**
 * Controller class that allows creation of visibility presets consisting of currently visible
 * map layers in map canvas.
 */
class QgsVisibilityPresets : public QObject
{
    Q_OBJECT
  public:

    typedef struct PresetRecord
    {
      bool operator==( const PresetRecord& other ) const
      {
        return mVisibleLayerIDs == other.mVisibleLayerIDs
               && mPerLayerCheckedLegendSymbols == other.mPerLayerCheckedLegendSymbols
               && mPerLayerCurrentStyle == other.mPerLayerCurrentStyle;
      }
      bool operator!=( const PresetRecord& other ) const
      {
        return !( *this == other );
      }

      //! List of layers that are visible
      QSet<QString> mVisibleLayerIDs;
      //! For layers that have checkable legend symbols and not all symbols are checked - list which ones are
      QMap<QString, QSet<QString> > mPerLayerCheckedLegendSymbols;
      //! For layers that use multiple styles - which one is currently selected
      QMap<QString, QString> mPerLayerCurrentStyle;
    } PresetRecord;


    static QgsVisibilityPresets* instance();

    //! Add a new preset using the current state of project's layer tree
    void addPreset( const QString& name );
    //! Update existing preset using the current state of project's layer tree
    void updatePreset( const QString& name );
    //! Remove existing preset
    void removePreset( const QString& name );

    //! Remove all presets
    void clear();

    //! Return list of existing preset names
    QStringList presets() const;

    //! Return recorded state of a preset
    PresetRecord presetState( const QString& presetName ) const { return mPresets[presetName]; }

    //! Return list of layer IDs that should be visible for particular preset
    QStringList presetVisibleLayers( const QString& name ) const;

    //! Apply check states of legend nodes of a given layer as defined in the preset
    void applyPresetCheckedLegendNodesToLayer( const QString& name, const QString& layerID );

    //! Convenience menu that lists available presets and actions for management
    QMenu* menu();

    //! Create preset record given a list of visible layers (needs to store per-layer checked legend symbols)
    PresetRecord currentStateFromLayerList( const QStringList& layerIDs );

  signals:
    void presetsChanged();

  protected slots:
    void addPreset();
    void presetTriggerred();
    void removeCurrentPreset();
    void menuAboutToShow();

    void readProject( const QDomDocument& doc );
    void writeProject( QDomDocument& doc );

    void registryLayersRemoved( QStringList layerIDs );

  protected:
    QgsVisibilityPresets(); // singleton

    typedef QMap<QString, PresetRecord> PresetRecordMap;

    void addVisibleLayersToPreset( QgsLayerTreeGroup* parent, PresetRecord& rec );
    void applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const PresetRecord& rec );
    void addPerLayerCheckedLegendSymbols( PresetRecord& rec );
    void addPerLayerCurrentStyle( PresetRecord& rec );

    PresetRecord currentState();
    void applyState( const QString& presetName );

    static QgsVisibilityPresets* sInstance;

    PresetRecordMap mPresets;

    QMenu* mMenu;
    QAction* mMenuSeparator;
    QAction* mActionRemoveCurrentPreset;
    QList<QAction*> mMenuPresetActions;
};


#endif // QGSVISIBILITYPRESETS_H
