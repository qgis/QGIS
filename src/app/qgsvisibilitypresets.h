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

#include "qgsvisibilitypresetcollection.h"
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
 * Contains methods for app-specific visibility preset functions.
 */
class APP_EXPORT QgsVisibilityPresets : public QObject
{
    Q_OBJECT
  public:

    /** Returns the instance QgsVisibilityPresets.
     */
    static QgsVisibilityPresets* instance();

    //! Add a new preset using the current state of project's layer tree
    void addPreset( const QString& name );
    //! Update existing preset using the current state of project's layer tree
    void updatePreset( const QString& name );

    //! Return list of layer IDs that should be visible for particular preset.
    //! The order will match the layer order from the map canvas
    QStringList orderedPresetVisibleLayers( const QString& name ) const;

    //! Convenience menu that lists available presets and actions for management
    QMenu* menu();

  protected slots:

    //! Handles adding a preset to the project's collection
    void addPreset();

    //! Handles apply a preset to the map canvas
    void presetTriggered();

    //! Handles replacing a preset's state
    void replaceTriggered();

    //! Handles removal of current preset from the project's collection
    void removeCurrentPreset();

    //! Handles creation of preset menu
    void menuAboutToShow();

  protected:
    QgsVisibilityPresets(); // singleton

    //! Applies current layer state to a preset record
    void applyStateToLayerTreeGroup( QgsLayerTreeGroup* parent, const QgsVisibilityPresetCollection::PresetRecord& rec );
    //! Applies layer checked legend symbols to a preset record
    void addPerLayerCheckedLegendSymbols( QgsVisibilityPresetCollection::PresetRecord& rec );
    //! Applies current layer styles to a preset record
    void addPerLayerCurrentStyle( QgsVisibilityPresetCollection::PresetRecord& rec );

    //! Returns the current state of the map canvas as a preset record
    QgsVisibilityPresetCollection::PresetRecord currentState();

    //! Applies a preset for the project's collection to the canvas
    void applyState( const QString& presetName );

    static QgsVisibilityPresets* sInstance;

    QMenu* mMenu;
    QMenu* mReplaceMenu;
    QAction* mMenuSeparator;
    QAction* mActionAddPreset;
    QAction* mActionRemoveCurrentPreset;
    QList<QAction*> mMenuPresetActions;
    QList<QAction*> mMenuReplaceActions;
};


#endif // QGSVISIBILITYPRESETS_H
