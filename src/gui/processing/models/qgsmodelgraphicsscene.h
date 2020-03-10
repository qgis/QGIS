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
class QgsProcessingModelComponent;
class QgsProcessingModelComment;

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
      RubberBand = 100, //!< Rubber band item
    };

    //! Flags for controlling how the scene is rendered and scene behavior
    enum Flag
    {
      FlagHideControls = 1 << 1,  //!< If set, item interactive controls will be hidden
      FlagHideComments = 1 << 2, //!< If set, comments will be hidden
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

    /**
     * Populates the scene by creating items representing the specified \a model.
     */
    void createItems( QgsProcessingModelAlgorithm *model, QgsProcessingContext &context );

  signals:

    /**
     * Emitted when a change in the model requires a full rebuild of the scene.
     */
    void rebuildRequired();

    /**
     * Emitted whenever a component of the model is about to be changed.
     *
    * The \a text argument gives the translated text describing the change about to occur, and the
    * optional \a id can be used to group the associated undo commands.
     */
    void componentAboutToChange( const QString &text, int id = 0 );

    /**
     * Emitted whenever a component of the model is changed.
     */
    void componentChanged();

  protected:

    /**
     * Creates a new graphic item for a model parameter.
     */
    virtual QgsModelComponentGraphicItem *createParameterGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelParameter *param ) const SIP_FACTORY;

    /**
     * Creates a new graphic item for a model child algorithm.
     */
    virtual QgsModelComponentGraphicItem *createChildAlgGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelChildAlgorithm *child ) const  SIP_FACTORY;

    /**
     * Creates a new graphic item for a model output.
     */
    virtual QgsModelComponentGraphicItem *createOutputGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelOutput *output ) const SIP_FACTORY;

    /**
     * Creates a new graphic item for a model comment.
     */
    virtual QgsModelComponentGraphicItem *createCommentGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelComment *comment,
        QgsModelComponentGraphicItem *parentItem ) const SIP_FACTORY;


  private:

    struct LinkSource
    {
      QgsModelComponentGraphicItem *item = nullptr;
      Qt::Edge edge = Qt::LeftEdge;
      int linkIndex = -1;
    };
    QList< LinkSource > linkSourcesForParameterValue( QgsProcessingModelAlgorithm *model, const QVariant &value, const QString &childId, QgsProcessingContext &context ) const;

    void addCommentItemForComponent( QgsProcessingModelAlgorithm *model, const QgsProcessingModelComponent &component, QgsModelComponentGraphicItem *parentItem );

    Flags mFlags = nullptr;

    QMap< QString, QgsModelComponentGraphicItem * > mParameterItems;
    QMap< QString, QgsModelComponentGraphicItem * > mChildAlgorithmItems;
    QMap< QString, QMap< QString, QgsModelComponentGraphicItem * > > mOutputItems;

};

Q_DECLARE_METATYPE( QgsModelGraphicsScene::Flags )

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
