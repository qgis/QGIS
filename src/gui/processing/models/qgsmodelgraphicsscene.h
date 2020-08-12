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
class QgsModelChildAlgorithmGraphicItem;
class QgsProcessingModelGroupBox;
class QgsMessageBar;

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
      GroupBox = 0, //!< A logical group box
      ArrowLink = 1, //!< An arrow linking model items
      ModelComponent = 2, //!< Model components (e.g. algorithms, inputs and outputs)
      MouseHandles = 99, //!< Mouse handles
      RubberBand = 100, //!< Rubber band item
      ZSnapIndicator = 101, //!< Z-value for snapping indicator

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

    QgsProcessingModelAlgorithm *model();

    void setModel( QgsProcessingModelAlgorithm *model );

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

    /**
     * Returns list of selected component items.
     */
    QList<QgsModelComponentGraphicItem *> selectedComponentItems();

    /**
     * Returns the topmost component item at a specified \a position.
     */
    QgsModelComponentGraphicItem *componentItemAt( QPointF position ) const;

    /**
     * Returns the graphic item corresponding to the specified group box \a uuid.
     */
    QgsModelComponentGraphicItem *groupBoxItem( const QString &uuid );

    /**
     * Selects all the components in the scene.
     */
    void selectAll();

    /**
     * Clears any selected items in the scene.
     *
     * Call this method rather than QGraphicsScene::clearSelection, as the latter does
     * not correctly emit signals to allow the scene's model to update.
    */
    void deselectAll();

    /**
     * Clears any selected items and sets \a item as the current selection.
    */
    void setSelectedItem( QgsModelComponentGraphicItem *item );

    /**
     * Sets the results for child algorithms for the last model execution.
     */
    void setChildAlgorithmResults( const QVariantMap &results );

    /**
     * Sets the inputs for child algorithms for the last model execution.
     */
    void setChildAlgorithmInputs( const QVariantMap &inputs );

    /**
     * Returns the message bar associated with the scene.
     *
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar() const;

    /**
     * Sets the message \a bar associated with the scene.
     *
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Shows a warning message, allowing users to click a button to see the full details (\a longMessage).
     */
    void showWarning( const QString &shortMessage, const QString &title, const QString &longMessage, Qgis::MessageLevel level = Qgis::Warning );

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

    /**
     * Emitted whenever the selected item changes.
     * If NULLPTR, no item is selected.
     */
    void selectedItemChanged( QgsModelComponentGraphicItem *selected );

  protected:

    /**
     * Creates a new graphic item for a model parameter.
     */
    virtual QgsModelComponentGraphicItem *createParameterGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelParameter *param ) const SIP_FACTORY;

    /**
     * Creates a new graphic item for a model child algorithm.
     */
    virtual QgsModelChildAlgorithmGraphicItem *createChildAlgGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelChildAlgorithm *child ) const  SIP_FACTORY;

    /**
     * Creates a new graphic item for a model output.
     */
    virtual QgsModelComponentGraphicItem *createOutputGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelOutput *output ) const SIP_FACTORY;

    /**
     * Creates a new graphic item for a model comment.
     */
    virtual QgsModelComponentGraphicItem *createCommentGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelComment *comment,
        QgsModelComponentGraphicItem *parentItem ) const SIP_FACTORY;

    /**
     * Creates a new graphic item for a model group box.
     */
    QgsModelComponentGraphicItem *createGroupBoxGraphicItem( QgsProcessingModelAlgorithm *model, QgsProcessingModelGroupBox *box ) const SIP_FACTORY;

  private:

    struct LinkSource
    {
      QgsModelComponentGraphicItem *item = nullptr;
      Qt::Edge edge = Qt::LeftEdge;
      int linkIndex = -1;
    };
    QList< LinkSource > linkSourcesForParameterValue( QgsProcessingModelAlgorithm *model, const QVariant &value, const QString &childId, QgsProcessingContext &context ) const;

    void addCommentItemForComponent( QgsProcessingModelAlgorithm *model, const QgsProcessingModelComponent &component, QgsModelComponentGraphicItem *parentItem );

    Flags mFlags = Flags();

    QgsProcessingModelAlgorithm *mModel = nullptr;

    QMap< QString, QgsModelComponentGraphicItem * > mParameterItems;
    QMap< QString, QgsModelChildAlgorithmGraphicItem * > mChildAlgorithmItems;
    QMap< QString, QMap< QString, QgsModelComponentGraphicItem * > > mOutputItems;
    QMap< QString, QgsModelComponentGraphicItem * > mGroupBoxItems;
    QVariantMap mChildResults;
    QVariantMap mChildInputs;

    QgsMessageBar *mMessageBar = nullptr;

};

Q_DECLARE_METATYPE( QgsModelGraphicsScene::Flags )

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
