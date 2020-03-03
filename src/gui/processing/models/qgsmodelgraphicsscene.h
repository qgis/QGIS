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
#include "qgsprocessingcontext.h"
#include <QGraphicsScene>

class QgsProcessingModelAlgorithm;
class QgsModelComponentGraphicItem;
class QgsProcessingModelParameter;
class QgsProcessingModelChildAlgorithm;
class QgsProcessingModelOutput;

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

    void mousePressEvent( QGraphicsSceneMouseEvent *event ) override;

  signals:

    /**
     * Emitted when a change in the model requires a full rebuild of the scene.
     */
    void rebuildRequired();

    /**
     * Emitted whenever a component of the model is changed.
     */
    void componentChanged();

  protected:

    virtual QgsModelComponentGraphicItem *createParameterGraphicItem( QgsProcessingModelParameter *param ) const SIP_FACTORY;
    virtual QgsModelComponentGraphicItem *createChildAlgGraphicItem( QgsProcessingModelChildAlgorithm *child ) const  SIP_FACTORY;
    virtual QgsModelComponentGraphicItem *createOutputGraphicItem( QgsProcessingModelOutput *output ) const SIP_FACTORY;

  private:

    void createItems( QgsProcessingContext &context );

    struct LinkSource
    {
      QgsModelComponentGraphicItem *item = nullptr;
      Qt::Edge edge = Qt::LeftEdge;
      int linkIndex = -1;
    };
    QList< LinkSource > linkSourcesForParameterValue( const QVariant &value, const QString &childId, QgsProcessingContext &context ) const;


    Flags mFlags = nullptr;

    QgsProcessingModelAlgorithm *mModel = nullptr;

    QMap< QString, QgsModelComponentGraphicItem * > mParameterItems;
    QMap< QString, QgsModelComponentGraphicItem * > mChildAlgorithmItems;
    QMap< QString, QMap< QString, QgsModelComponentGraphicItem * > > mOutputItems;

};

Q_DECLARE_METATYPE( QgsModelGraphicsScene::Flags )

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
