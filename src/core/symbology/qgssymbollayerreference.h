/***************************************************************************
 qgssymbollayerreference.h
 ---------------------
 begin                : June 2019
 copyright            : (C) 2019 by Hugo Mercier / Oslandia
 email                : infos at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLLAYERREFERENCE_H
#define QGSSYMBOLLAYERREFERENCE_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QList>
#include <QVariant>
#include <QVector>

class QgsVectorLayer;

/**
 * We may need stable references to symbol layers, when pointers to symbol layers is not usable
 * (when a symbol or a feature renderer is cloned for example).
 *
 * A symbol layer identifier consists of:
 *
 * - an identifier to its symbol (given by the QgsFeatureRenderer)
 * - a path of indexes inside its symbol and subsymbols.
 *
 * For a symbol in a QgsSingleSymbolRenderer that has two symbol layers, it will give:
 *
 * - "" for the symbol key
 * - [0] and [1] for the two symbol layer indexes
 *
 * For a QgsRuleBasedRenderer each rule key is the symbol key.
 *
 * For a symbol with a symbol layer that has a sub symbol (say a QgsArrowSymbolLayer),
 * path to symbol layers of the sub symbol are given by a list of indexes:
 *
 * - [0, 0] : first symbol layer of the sub symbol of the first symbol layer
 * - [0, 1] : second symbol layer of the sub symbol of the first symbol layer
 * - [2, 0] : first symbol layer of the sub symbol of the third symbol layer, etc.
 *
 * \ingroup core
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsSymbolLayerId
{
  public:
    QgsSymbolLayerId() {}

    /**
     * QgsSymbolLayerId constructor with a symbol key and a unique symbol layer index
     */
    QgsSymbolLayerId( QString key, int index )
      : mSymbolKey( key ), mIndexPath( { index } )
    {}

    /**
     * QgsSymbolLayerId constructor with a symbol key and an index path
     */
    QgsSymbolLayerId( QString key, const QVector<int> &indexPath )
      : mSymbolKey( key ), mIndexPath( { indexPath } )
    {}

    //! Default copy constructor
    QgsSymbolLayerId( const QgsSymbolLayerId &other ) = default;

    //! Default assignment operator
    QgsSymbolLayerId &operator=( const QgsSymbolLayerId &other ) = default;

    /**
     * Returns the key associated to the symbol
     */
    QString symbolKey() const { return mSymbolKey; }

    /**
     * Returns the symbol layer index path inside the symbol
     */
    QVector<int> symbolLayerIndexPath() const { return mIndexPath; }

    //! Equality operator
    bool operator==( const QgsSymbolLayerId &other ) const
    {
      return ( mSymbolKey == other.mSymbolKey && mIndexPath == other.mIndexPath );
    }

    //! Comparison operator, for storage in a QSet or QMap
    bool operator<( const QgsSymbolLayerId &other ) const
    {
      return ( mSymbolKey == other.mSymbolKey ) ?
             mIndexPath < other.mIndexPath
             : mSymbolKey < other.mSymbolKey;
    }

  private:
    //! Symbol unique identifier (legend key)
    QString mSymbolKey;

    //! Symbol layer index path in symbol
    QVector<int> mIndexPath;
};

/**
 * \ingroup core
 * \class QgsSymbolLayerReference
 *
 * Type used to refer to a specific symbol layer in a symbol of a layer.
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsSymbolLayerReference
{
  public:
    //! Default constructor
    QgsSymbolLayerReference() = default;

    //! Constructor
    QgsSymbolLayerReference( const QString &layerId, const QgsSymbolLayerId &symbolLayer )
      : mLayerId( layerId ), mSymbolLayerId( symbolLayer )
    {}

    /**
     * The referenced vector layer / feature renderer
     */
    QString layerId() const { return mLayerId; }

    /**
     * The symbol layer's id
     */
    QgsSymbolLayerId symbolLayerId() const { return mSymbolLayerId; }

    //! Comparison operator
    bool operator==( const QgsSymbolLayerReference &other ) const
    {
      return mLayerId == other.mLayerId &&
             mSymbolLayerId == other.mSymbolLayerId;
    }

  private:
    QString mLayerId;
    QgsSymbolLayerId mSymbolLayerId;
};

inline uint qHash( const QgsSymbolLayerId &id )
{
  return qHash( id.symbolKey() ) ^ qHash( id.symbolLayerIndexPath() );
}

inline uint qHash( const QgsSymbolLayerReference &r )
{
  return qHash( r.layerId() ) ^ qHash( r.symbolLayerId() );
}

typedef QList<QgsSymbolLayerReference> QgsSymbolLayerReferenceList;

/**
 * Utilitary function to turn a QgsSymbolLayerReferenceList into a string
 * \see stringToSymbolLayerReferenceList
 * \since QGIS 3.12
 */
CORE_EXPORT QString symbolLayerReferenceListToString( const QgsSymbolLayerReferenceList & );

/**
 * Utilitary function to parse a string originated from symbolLayerReferenceListToString
 * into a QgsSymbolLayerReferenceList
 * \see symbolLayerReferenceListToString
 * \since QGIS 3.12
 */
CORE_EXPORT QgsSymbolLayerReferenceList stringToSymbolLayerReferenceList( const QString & );

#endif
