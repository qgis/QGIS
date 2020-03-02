/***************************************************************************
                             qgsmodelgraphicsscene.h
                             -----------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELGRAPHICSCENE_H
#define QGSMODELGRAPHICSCENE_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsScene>


///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief QGraphicsScene subclass representing the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

  public:

    //! Z values for scene items
    enum ZValues
    {
      ArrowLink = 0, //!< An arrow linking model items
      ModelComponent = 1, //!< Model components (e.g. algorithms, inputs and outputs)
    };

    //! Flags for controlling how the scene is rendered and scene behavior
    enum Flag
    {
      FlagHideControls = 1 << 1,  //!< If set, item interactive controls will be hidden
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsModelGraphicsScene with the specified \a parent object.
     */
    QgsModelGraphicsScene( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the combination of \a flags controlling how the scene is rendered and behaves.
     * \see setFlag()
     * \see flags()
     */
    void setFlags( QgsModelGraphicsScene::Flags flags ) { mFlags = flags; }

    /**
     * Enables or disables a particular \a flag for the scene. Other existing
     * flags are not affected.
     * \see setFlags()
     * \see flags()
     */
    void setFlag( QgsModelGraphicsScene::Flag flag, bool on = true );

    /**
     * Returns the current combination of flags set for the scene.
     * \see setFlags()
     * \see setFlag()
     */
    QgsModelGraphicsScene::Flags flags() const { return mFlags; }

  private:

    Flags mFlags = nullptr;

};

Q_DECLARE_METATYPE( QgsModelGraphicsScene::Flags )

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
