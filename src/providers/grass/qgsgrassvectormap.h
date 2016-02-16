/***************************************************************************
                            qgsgrassvectormap.cpp
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRASSVECTORMAP_H
#define QGSGRASSVECTORMAP_H

#include <QDateTime>
#include <QObject>

#include "qgsabstractgeometryv2.h"

#include "qgsgrass.h"
#include "qgsgrassvectormaplayer.h"

class QgsGrassUndoCommand;

class GRASS_LIB_EXPORT QgsGrassVectorMap : public QObject
{
    Q_OBJECT
  public:
    enum TopoSymbol
    {
      TopoUndefined = 0,
      TopoPoint,
      TopoLine,
      TopoBoundaryError, // both sides  topology broken
      TopoBoundaryErrorLeft, // left side topology broken
      TopoBoundaryErrorRight, // right side topology broken
      TopoBoundaryOk,
      TopoCentroidIn,
      TopoCentroidOut,
      TopoCentroidDupl,
      TopoNode0,
      TopoNode1,
      TopoNode2
    };

    QgsGrassVectorMap( const QgsGrassObject & grassObject );
    ~QgsGrassVectorMap();

    QgsGrassObject grassObject() const { return mGrassObject; }
    struct Map_info *map() { return mMap; }
    bool isValid() const { return mValid; }
    bool isFrozen() const { return mFrozen; }
    bool isEdited() const { return mIsEdited; }
    bool isOpen() const { return mOpen; }
    int version() const { return mVersion; }
    int oldNumLines() const { return mOldNumLines; }
    // number of instances using this map
    int userCount() const;
    /** Get current number of lines.
     *   @return number of lines */
    int numLines();
    int numAreas();
    // 3D map with z coordinates
    bool is3d() { return mIs3d; }

    // Lock open / close
    void lockOpenClose();
    void unlockOpenClose();

    // Lock open / close
    void lockOpenCloseLayer();
    void unlockOpenCloseLayer();

    // Lock reading and writing
    void lockReadWrite();
    void unlockReadWrite();

    QHash<int, int> & oldLids() { return mOldLids; }
    QHash<int, int> & newLids() { return mNewLids; }
    QHash<int, QgsAbstractGeometryV2*> & oldGeometries() { return mOldGeometries; }
    QHash<int, int> & oldTypes() { return mOldTypes; }
    QHash<QgsFeatureId, int> & newCats() { return mNewCats; }
    QMap<int, QList<QgsGrassUndoCommand *> > & undoCommands() { return mUndoCommands; }

    /** Get geometry of line.
     * @return geometry (point,line or polygon(GV_FACE)) or 0 */
    QgsAbstractGeometryV2 * lineGeometry( int id );
    QgsAbstractGeometryV2 * nodeGeometry( int id );
    QgsAbstractGeometryV2 * areaGeometry( int id );

    /** Open map if not yet open. Open/close lock */
    bool open();

    /** Close map. All iterators are closed first. Open/close lock. */
    void close();

    /** Open GRASS map, no open/close locking */
    bool openMap();

    /** Close GRASS map, no open/close locking */
    void closeMap();

    /** Reload layers from (reopened) map. The layers keep field/type. */
    void reloadLayers();

    bool startEdit();
    bool closeEdit( bool newMap );
    void clearUndoCommands();

    /** Get layer, layer is created and loaded if not yet.
     *  @param field
     *  @return pointer to layer or 0 if layer doe not exist */
    QgsGrassVectorMapLayer * openLayer( int field );

    /** Close layer and release cached data if there are no more users and close map
     *  if there are no more map users.
     *  @param layer */
    void closeLayer( QgsGrassVectorMapLayer * layer );

    /** Update map. Close and reopen vector and refresh layers.
     *  Instances of QgsGrassProvider are not updated and should call update() method */
    void update();

    /** The map is outdated. The map was for example rewritten by GRASS module outside QGIS.
     *  This function checks internal timestamp stored in QGIS.
     */
    bool mapOutdated();

    /** The attributes are outdated. The table was for example updated by GRASS module outside QGIS.
     *  This function checks internal timestamp stored in QGIS.
     */
    bool attributesOutdated();

    /** Map descripton for debugging */
    QString toString();

    /** Get topology symbol code
     * @param lid line or area number
     * @param type geometry type */
    TopoSymbol topoSymbol( int lid );

    static QString topoSymbolFieldName() { return "topo_symbol" ; }

    void printDebug();

  signals:
    /** Ask all iterators to cancel iteration when possible. Connected to iterators with
     * Qt::DirectConnection (non blocking) */
    void cancelIterators();

    /** Close all iterators. Connected to iterators in different threads with Qt::BlockingQueuedConnection */
    void closeIterators();

    /** Emitted when data were reloaded */
    void dataChanged();

  private:
    /** Close iterators, blocking */
    void closeAllIterators();

    QgsGrassObject mGrassObject;
    // true if map is open, once the map is closed, valid is set to false and no more used
    bool mValid;
    // Indicates if map is open, it may be open but invalide
    bool mOpen;
    // Vector temporally disabled. Necessary for GRASS Tools on Windows
    bool mFrozen;
    // true if the map is opened in update mode
    bool mIsEdited;
    // version, increased by each closeEdit() and updateMap()
    int mVersion;
    // last modified time of the vector directory, when the map was opened
    QDateTime mLastModified;
    // last modified time of the vector 'dbln' file, when the map was opened
    // or attributes were updated. The 'dbln' file is updated by v.to.db etc.
    QDateTime mLastAttributesModified;
    // when attributes are changed
    // map header
    struct  Map_info *mMap;
    // Is 3D, has z coordinates
    bool mIs3d;
    // Vector layers
    QList<QgsGrassVectorMapLayer*> mLayers;
    // Number of lines in vector before editing started
    int mOldNumLines;
    // Original line ids of rewritten GRASS lines (new lid -> old lid)
    QHash<int, int> mOldLids;
    // Current line ids for old line ids (old lid -> new lid)
    QHash<int, int> mNewLids;
    // Hash of original lines' geometries of lines which were changed, keys are GRASS lid
    QHash<int, QgsAbstractGeometryV2*> mOldGeometries;
    // Hash of original lines' geometries GRASS types of lines which were changed, keys are GRASS lid
    QHash<int, int> mOldTypes;
    // New categories attached to new features or old features without category
    // fid -> cat, the fid may be old fid without category or new (negative) feature id
    QHash<QgsFeatureId, int> mNewCats;

    // Map of undo commands with undo stack index as key.
    QMap<int, QList<QgsGrassUndoCommand *> > mUndoCommands;

    // Mutex used to avoid concurrent read/write, used only in editing mode
    QMutex mReadWriteMutex;

    // Open / close mutex
    QMutex mOpenCloseMutex;

    // Open / close mutex
    QMutex mOpenCloseLayerMutex;
};

class GRASS_LIB_EXPORT QgsGrassVectorMapStore
{
  public:
    QgsGrassVectorMapStore();
    ~QgsGrassVectorMapStore();

    static QgsGrassVectorMapStore *instance();

    // Default instance may be overridden explicitly to avoid (temporarily) to share maps by providers
    // This is only used for editing test to have an independent map
    static void setStore( QgsGrassVectorMapStore * store ) { mStore = store; }

    /** Open map.
     *  @param grassObject
     *  @return map, the map may be invalide  */
    QgsGrassVectorMap * openMap( const QgsGrassObject & grassObject );

  private:
    /** Open vector maps */
    QList<QgsGrassVectorMap*> mMaps;

    // Lock open/close map
    QMutex mMutex;

    static QgsGrassVectorMapStore * mStore;
};

#endif // QGSGRASSVECTORMAP_H
