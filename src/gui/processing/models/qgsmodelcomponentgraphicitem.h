/***************************************************************************
                             qgsmodelcomponentgraphicitem.h
                             ----------------------------------
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

#ifndef QGSMODELCOMPONENTGRAPHICITEM_H
#define QGSMODELCOMPONENTGRAPHICITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsObject>
#include <QFont>
#include <QPicture>
#include <QPointer>

class QgsProcessingModelComponent;
class QgsProcessingModelParameter;
class QgsProcessingModelChildAlgorithm;
class QgsProcessingModelOutput;
class QgsProcessingModelComment;
class QgsProcessingModelAlgorithm;
class QgsModelDesignerFlatButtonGraphicItem;
class QgsModelDesignerFoldButtonGraphicItem;
class QgsModelGraphicsView;
class QgsModelViewMouseEvent;
class QgsProcessingModelGroupBox;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Base class for graphic items representing model components in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelComponentGraphicItem : public QGraphicsObject
{
    Q_OBJECT

  public:

    //! Available item states
    enum State
    {
      Normal, //!< Normal state
      Selected, //!< Item is selected
      Hover, //!< Cursor is hovering over an deselected item
    };

    //! Available flags
    enum Flag
    {
      // For future API flexibility only and to avoid sip issues, remove when real entries are added to flags.
      Unused = 1 << 0, //!< Temporary unused entry
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsModelComponentGraphicItem for the specified \a component, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a component is transferred to the item.
     */
    QgsModelComponentGraphicItem( QgsProcessingModelComponent *component SIP_TRANSFER,
                                  QgsProcessingModelAlgorithm *model,
                                  QGraphicsItem *parent SIP_TRANSFERTHIS );

    ~QgsModelComponentGraphicItem() override;

    /**
     * Returns item flags.
     */
    virtual Flags flags() const;

    /**
     * Returns the model component associated with this item.
     */
    QgsProcessingModelComponent *component();

    /**
     * Returns the model component associated with this item.
     */
    const QgsProcessingModelComponent *component() const SIP_SKIP;

    /**
     * Returns the model associated with this item.
     */
    QgsProcessingModelAlgorithm *model();

    /**
     * Returns the associated view.
     */
    QgsModelGraphicsView *view();

    /**
     * Returns the font used to render text in the item.
     * \see setFont()
     */
    QFont font() const;

    /**
     * Sets the \a font used to render text in the item.
     * \see font()
     */
    void setFont( const QFont &font );

    /**
     * Moves the component by the specified \a dx and \a dy.
     *
     * \warning Call this method, not QGraphicsItem::moveBy!
     */
    void moveComponentBy( qreal dx, qreal dy );

    /**
     * Shows a preview of moving the item from its stored position by \a dx, \a dy.
     */
    void previewItemMove( qreal dx, qreal dy );

    /**
     * Sets a new scene \a rect for the item.
     */
    void setItemRect( QRectF rect );

#ifndef SIP_RUN

    /**
     * Shows a preview of setting a new \a rect for the item.
     */
    QRectF previewItemRectChange( QRectF rect );

    /**
     * Sets a new scene \a rect for the item.
     */
    void finalizePreviewedItemRectChange( QRectF rect );

    /**
     * Handles a model hover enter \a event.
     */
    virtual void modelHoverEnterEvent( QgsModelViewMouseEvent *event );

    /**
     * Handles a model hover move \a event.
     */
    virtual void modelHoverMoveEvent( QgsModelViewMouseEvent *event );

    /**
     * Handles a model hover leave \a event.
     */
    virtual void modelHoverLeaveEvent( QgsModelViewMouseEvent *event );

    /**
     * Handles a model double-click \a event.
     */
    virtual void modelDoubleClickEvent( QgsModelViewMouseEvent *event );
#endif
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) override;
    void hoverEnterEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverMoveEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent *event ) override;
    QVariant itemChange( GraphicsItemChange change, const QVariant &value ) override;
    QRectF boundingRect() const override;
    bool contains( const QPointF &point ) const override;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    /**
     * Returns the rectangle representing the body of the item.
     */
    QRectF itemRect( bool storedRect = false ) const;

    /**
     * Returns the item's label text.
     *
     * \see setLabel()
     */
    QString label() const;

    /**
     * Returns the item's \a label text.
     *
     * \see label()
     */
    void setLabel( const QString &label );

    /**
     * Returns the item's current state.
     */
    State state() const;

    /**
     * Returns the number of link points associated with the component on the specified \a edge.
     */
    virtual int linkPointCount( Qt::Edge edge ) const;

    /**
     * Returns the text to use for the link point with the specified \a index on the specified \a edge.
     */
    virtual QString linkPointText( Qt::Edge edge, int index ) const;

    /**
     * Returns the location of the link point with the specified \a index on the specified \a edge.
     */
    QPointF linkPoint( Qt::Edge edge, int index, bool incoming ) const;

    /**
     * Returns the best link point to use for a link originating at a specified \a other item.
     *
     * \param other item at other end of link
     * \param edge item edge for calculated best link point
     * \returns calculated link point in item coordinates.
     */
    QPointF calculateAutomaticLinkPoint( QgsModelComponentGraphicItem *other, Qt::Edge &edge SIP_OUT ) const;

    /**
     * Returns the best link point to use for a link originating at a specified \a other point.
     *
     * \param other point for other end of link (in scene coordinates)
     * \param edge item edge for calculated best link point
     * \returns calculated link point in item coordinates.
     */
    QPointF calculateAutomaticLinkPoint( const QPointF &point, Qt::Edge &edge SIP_OUT ) const;

    /**
     * Called when the comment attached to the item should be edited.
     *
     * The default implementation does nothing.
     */
    virtual void editComment() {}

    /**
     * Returns TRUE if the component can be deleted.
     */
    virtual bool canDeleteComponent() { return false; }

    /**
     * Called when the component should be deleted.
     *
     * The default implementation does nothing.
     */
    virtual void deleteComponent() {}

  signals:

    // TODO - rework this, should be triggered externally when the model actually changes!

    /**
     * Emitted by the item to request a repaint of the parent model scene.
     */
    void requestModelRepaint();

    /**
     * Emitted when the definition of the associated component is about to be changed
     * by the item.
     *
     * The \a text argument gives the translated text describing the change about to occur, and the
     * optional \a id can be used to group the associated undo commands.
     */
    void aboutToChange( const QString &text, int id = 0 );

    /**
     * Emitted when the definition of the associated component is changed
     * by the item.
     */
    void changed();

    /**
     * Emitted when item requests that all connected arrows are repainted.
     */
    void repaintArrows();

    /**
     * Emitted when item requires that all connected arrow paths are recalculated.
     */
    void updateArrowPaths();

    /**
     * Emitted when the item's size or position changes.
     */
    void sizePositionChanged();

  protected slots:

    /**
     * Called when the component should be edited.
     *
     * The default implementation does nothing.
     */
    virtual void editComponent() {}

  protected:

    /**
     * Truncates a \a text string so that it fits nicely within the item's width,
     * accounting for margins and interactive buttons.
     */
    QString truncatedTextForItem( const QString &text ) const;

    /**
     * Returns the fill color for the item for the specified \a state.
     */
    virtual QColor fillColor( State state ) const = 0;

    /**
     * Returns the stroke color for the item for the specified \a state.
     */
    virtual QColor strokeColor( State state ) const = 0;

    /**
     * Returns the label text color for the item for the specified \a state.
     */
    virtual QColor textColor( State state ) const = 0;

    /**
     * Returns the stroke style to use while rendering the outline of the item.
     */
    virtual Qt::PenStyle strokeStyle( State state ) const;

    /**
     * Returns the title alignment
     */
    virtual Qt::Alignment titleAlignment() const;

    /**
     * Returns a QPicture version of the item's icon, if available.
     */
    virtual QPicture iconPicture() const;

    /**
     * Returns a QPixmap version of the item's icon, if available.
     */
    virtual QPixmap iconPixmap() const;

    /**
     * Updates the position and size stored in the model for the associated comment
     */
    virtual void updateStoredComponentPosition( const QPointF &pos, const QSizeF &size ) = 0;

    /**
     * Updates the item's button positions, based on the current item rect.
     */
    void updateButtonPositions();

  private:

    QSizeF itemSize() const;

    void updateToolTip( const QPointF &pos );

    void fold( Qt::Edge edge, bool folded );

    std::unique_ptr< QgsProcessingModelComponent > mComponent;
    QgsProcessingModelAlgorithm *mModel = nullptr;

    bool mInitialized = false;
    QgsModelDesignerFoldButtonGraphicItem *mExpandTopButton = nullptr;
    QgsModelDesignerFoldButtonGraphicItem *mExpandBottomButton = nullptr;

    QString mLabel;

    QgsModelDesignerFlatButtonGraphicItem *mEditButton = nullptr;
    QgsModelDesignerFlatButtonGraphicItem *mDeleteButton = nullptr;

    static constexpr double MIN_COMPONENT_WIDTH = 70;
    static constexpr double MIN_COMPONENT_HEIGHT = 30;

    static constexpr double DEFAULT_BUTTON_WIDTH = 16;
    static constexpr double DEFAULT_BUTTON_HEIGHT = 16;
    static constexpr double BUTTON_MARGIN = 2;
    static constexpr double TEXT_MARGIN = 4;
    static constexpr double RECT_PEN_SIZE = 2;
    QSizeF mButtonSize { DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT };

    QFont mFont;

    bool mIsHovering = false;
    bool mIsMoving = false;
    QSizeF mTempSize;

};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsModelComponentGraphicItem::Flags )

/**
 * \ingroup gui
 * \brief A graphic item representing a model parameter (input) in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelParameterGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelParameterGraphicItem for the specified \a parameter, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a parameter is transferred to the item.
     */
    QgsModelParameterGraphicItem( QgsProcessingModelParameter *parameter SIP_TRANSFER,
                                  QgsProcessingModelAlgorithm *model,
                                  QGraphicsItem *parent SIP_TRANSFERTHIS );

    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPicture iconPicture() const override;
    void updateStoredComponentPosition( const QPointF &pos, const QSizeF &size ) override;

  protected slots:

    void deleteComponent() override;

  private:
    QPicture mPicture;

};

/**
 * \ingroup gui
 * \brief A graphic item representing a child algorithm in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelChildAlgorithmGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelChildAlgorithmGraphicItem for the specified \a child, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a child is transferred to the item.
     */
    QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child SIP_TRANSFER,
                                       QgsProcessingModelAlgorithm *model,
                                       QGraphicsItem *parent SIP_TRANSFERTHIS );
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;

    /**
     * Sets the results obtained for this child algorithm for the last model execution through the dialog.
     */
    void setResults( const QVariantMap &results );

    /**
     * Sets the inputs used for this child algorithm for the last model execution through the dialog.
     */
    void setInputs( const QVariantMap &inputs );

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPixmap iconPixmap() const override;
    QPicture iconPicture() const override;

    int linkPointCount( Qt::Edge edge ) const override;
    QString linkPointText( Qt::Edge edge, int index ) const override;
    void updateStoredComponentPosition( const QPointF &pos, const QSizeF &size ) override;

  protected slots:

    void deleteComponent() override;

  private slots:
    void deactivateAlgorithm();
    void activateAlgorithm();

  private:
    QPicture mPicture;
    QPixmap mPixmap;
    QVariantMap mResults;
    QVariantMap mInputs;
    bool mIsValid = true;
};


/**
 * \ingroup gui
 * \brief A graphic item representing a model output in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelOutputGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelOutputGraphicItem for the specified \a output, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a output is transferred to the item.
     */
    QgsModelOutputGraphicItem( QgsProcessingModelOutput *output SIP_TRANSFER,
                               QgsProcessingModelAlgorithm *model,
                               QGraphicsItem *parent SIP_TRANSFERTHIS );

    bool canDeleteComponent() override;

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPicture iconPicture() const override;
    void updateStoredComponentPosition( const QPointF &pos, const QSizeF &size ) override;

  protected slots:

    void deleteComponent() override;

  private:

    QPicture mPicture;
};



/**
 * \ingroup gui
 * \brief A graphic item representing a model comment in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelCommentGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelCommentGraphicItem for the specified \a comment, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a output is transferred to the item.
     */
    QgsModelCommentGraphicItem( QgsProcessingModelComment *comment SIP_TRANSFER,
                                QgsModelComponentGraphicItem *parentItem,
                                QgsProcessingModelAlgorithm *model,
                                QGraphicsItem *parent SIP_TRANSFERTHIS );
    ~QgsModelCommentGraphicItem() override;
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;

    /**
     * Returns the parent model component item.
     */
    QgsModelComponentGraphicItem *parentComponentItem() const;

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    Qt::PenStyle strokeStyle( State state ) const override;
    void updateStoredComponentPosition( const QPointF &pos, const QSizeF &size ) override;

  protected slots:

    void deleteComponent() override;
    void editComponent() override;
  private:

    QgsProcessingModelComment *modelComponent();

    std::unique_ptr< QgsProcessingModelComponent > mParentComponent;
    QPointer< QgsModelComponentGraphicItem > mParentItem;


};


/**
 * \ingroup gui
 * \brief A graphic item representing a group box in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelGroupBoxGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelGroupBoxGraphicItem for the specified group \a box, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a output is transferred to the item.
     */
    QgsModelGroupBoxGraphicItem( QgsProcessingModelGroupBox *box SIP_TRANSFER,
                                 QgsProcessingModelAlgorithm *model,
                                 QGraphicsItem *parent SIP_TRANSFERTHIS );
    ~QgsModelGroupBoxGraphicItem() override;
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;
  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    Qt::PenStyle strokeStyle( State state ) const override;
    Qt::Alignment titleAlignment() const override;
    void updateStoredComponentPosition( const QPointF &pos, const QSizeF &size ) override;

  protected slots:

    void deleteComponent() override;
    void editComponent() override;
  private:


};

///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
