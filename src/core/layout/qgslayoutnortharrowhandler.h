/***************************************************************************
                         qgslayoutnortharrowhandler.h
                         -------------------
begin                : April 2020
copyright            : (C) 2020 by Nyall Dawson
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
#ifndef QGSLAYOUTNORTHARROWHANDLER_H
#define QGSLAYOUTNORTHARROWHANDLER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QPointer>

class QgsLayoutItemMap;

/**
 * \ingroup core
 * \brief An object which handles north-arrow type behavior for layout items.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsLayoutNorthArrowHandler: public QObject
{
    Q_OBJECT
  public:

    //! Method for syncing rotation to a map's North direction
    enum NorthMode
    {
      GridNorth = 0, //!< Align to grid north
      TrueNorth, //!< Align to true north
    };

    /**
     * Constructor for QgsLayoutNorthArrowHandler, with the specified parent \a object.
     */
    QgsLayoutNorthArrowHandler( QObject *parent SIP_TRANSFERTHIS );

    /**
     * Returns the rotation to be used for the arrow, in degrees clockwise.
     */
    double arrowRotation() const { return mArrowRotation; }

    /**
     * Sets the linked \a map item.
     *
     * \see linkedMap()
     */
    void setLinkedMap( QgsLayoutItemMap *map );

    /**
     * Returns the linked rotation map, if set. An NULLPTR means arrow calculation is
     * disabled.
     *
     * \see setLinkedMap()
     */
    QgsLayoutItemMap *linkedMap() const;

    /**
     * Returns the mode used to calculate the arrow rotation.
     * \see setNorthMode()
     * \see northOffset()
     */
    NorthMode northMode() const { return mNorthMode; }

    /**
     * Sets the \a mode used to calculate the arrow rotation.
     * \see northMode()
     * \see setNorthOffset()
     */
    void setNorthMode( NorthMode mode );

    /**
     * Returns the offset added to the arrows's rotation from a map's North.
     * \see setNorthOffset()
     * \see northMode()
     */
    double northOffset() const { return mNorthOffset; }

    /**
     * Sets the \a offset added to the arrows's rotation from a map's North.
     * \see northOffset()
     * \see setNorthMode()
     */
    void setNorthOffset( double offset );

  signals:
    //! Emitted on arrow rotation change
    void arrowRotationChanged( double newRotation );

  private:

    //! Arrow rotation
    double mArrowRotation = 0;

    QString mRotationMapUuid;
    //! Map that sets the rotation (or NULLPTR if this picture uses map independent rotation)
    QPointer< QgsLayoutItemMap > mRotationMap;

    //! Mode used to align to North
    NorthMode mNorthMode = GridNorth;
    //! Offset for north arrow
    double mNorthOffset = 0.0;

    void disconnectMap( QgsLayoutItemMap *map );

  private slots:

    void updateMapRotation();


};

#endif // QGSLAYOUTNORTHARROWHANDLER_H
