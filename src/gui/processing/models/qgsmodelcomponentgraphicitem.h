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
     * Returns the font used to render text in the item.
     * \see setFont()
     */
    QFont font() const;

    /**
     * Sets the \a font used to render text in the item.
     * \see font()
     */
    void setFont( const QFont &font );

    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) override;
    void hoverEnterEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverMoveEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent *event ) override;
    QVariant itemChange( GraphicsItemChange change, const QVariant &value ) override;
    QRectF boundingRect() const override;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    /**
     * Returns the rectangle representing the body of the item.
     */
    QRectF itemRect() const;

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
    QPointF linkPoint( Qt::Edge edge, int index ) const;

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

  signals:

    // TODO - rework this, should be triggered externally when the model actually changes!

    /**
     * Emitted by the item to request a repaint of the parent model scene.
     */
    void requestModelRepaint();

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

  protected slots:

    /**
     * Called when the component should be edited.
     *
     * The default implementation does nothing.
     */
    virtual void editComponent() {}

    /**
     * Called when the component should be deleted.
     *
     * The default implementation does nothing.
     */
    virtual void deleteComponent() {}

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
     * Returns a QPicture version of the item's icon, if available.
     */
    virtual QPicture iconPicture() const;

    /**
     * Returns a QPixmap version of the item's icon, if available.
     */
    virtual QPixmap iconPixmap() const;

    /**
     * Updates the position stored in the model for the associated comment
     */
    virtual void updateStoredComponentPosition( const QPointF &pos ) = 0;

  private:

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

    static constexpr double DEFAULT_BUTTON_WIDTH = 16;
    static constexpr double DEFAULT_BUTTON_HEIGHT = 16;
    QSizeF mButtonSize { DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT };

    QFont mFont;

    bool mIsHovering = false;

};

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

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPicture iconPicture() const override;
    void updateStoredComponentPosition( const QPointF &pos ) override;

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

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPixmap iconPixmap() const override;
    QPicture iconPicture() const override;

    int linkPointCount( Qt::Edge edge ) const override;
    QString linkPointText( Qt::Edge edge, int index ) const override;
    void updateStoredComponentPosition( const QPointF &pos ) override;

  protected slots:

    void deleteComponent() override;

  private slots:
    void deactivateAlgorithm();
    void activateAlgorithm();

  private:
    QPicture mPicture;
    QPixmap mPixmap;
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

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPicture iconPicture() const override;
    void updateStoredComponentPosition( const QPointF &pos ) override;

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

  protected:

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    Qt::PenStyle strokeStyle( State state ) const override;
    void updateStoredComponentPosition( const QPointF &pos ) override;

  protected slots:

    void deleteComponent() override;
    void editComponent() override;
  private:

    QgsProcessingModelComment *modelComponent();

    std::unique_ptr< QgsProcessingModelComponent > mParentComponent;
    QPointer< QgsModelComponentGraphicItem > mParentItem;


};
///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
