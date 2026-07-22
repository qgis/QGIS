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
#include "qgsprocessingcontext.h"

#include <QFont>
#include <QGraphicsObject>
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
class QgsModelDesignerSocketGraphicItem;
class QgsModelGraphicsView;
class QgsModelViewMouseEvent;
class QgsProcessingModelGroupBox;
class QgsModelArrowItem;

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
      Normal,   //!< Normal state
      Selected, //!< Item is selected
      Hover,    //!< Cursor is hovering over an deselected item
    };

    //! Available flags
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
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
    QgsModelComponentGraphicItem( QgsProcessingModelComponent *component SIP_TRANSFER, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent SIP_TRANSFERTHIS );

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
     * Returns the model associated with this item.
     */
    const QgsProcessingModelAlgorithm *model() const SIP_SKIP;

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
     * Returns the color of the link at the specified \a index on the specified \a edge.
     *
     * \since QGIS 4.0
     */
    virtual QColor linkColor( Qt::Edge edge, int index ) const;

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
     * Returns the output socket graphics items at the specified \a index.
     *
     * May return NULLPTR if no corresponding output socket exists.
     * \since QGIS 3.44
     */
    QgsModelDesignerSocketGraphicItem *outSocketAt( int index ) const;


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

    /**
     * Returns the list of incoming arrow items terminating at this item.
     *
     * \see outgoingArrows()
     * \since QGIS 4.2
     */
    QList< QgsModelArrowItem * > incomingArrows();

    /**
     * Returns the list of outgoing arrow items originating at this item.
     *
     * \see incomingArrows()
     * \since QGIS 4.2
     */
    QList< QgsModelArrowItem * > outgoingArrows();

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
    void aboutToChange( const QString &text, const QString &id = QString() );

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
     * Paints the background part of the graphic item.
     *
     * Subclasses may override this to customize the background appearance.
     */
    virtual void paintBackground( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr );

    /**
     * Paints the outline part of the graphic item.
     */
    void paintOutline( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr );

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
     * Returns the optional color for an outline effect around the item.
     *
     * Returns an invalid color if the outline effect is not required.
     */
    virtual QColor outlineColor() const { return QColor(); }

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

    /**
     * The fallback color if the parameter or output does not have a specific color.
     *
     * \since QGIS 4.0
     */
    SIP_SKIP static constexpr QColor FALLBACK_COLOR = QColor( 128, 128, 128 ); /* mid gray */

  private:
    QSizeF itemSize() const;

    void updateToolTip( const QPointF &pos );

    void fold( Qt::Edge edge, bool folded );

    std::unique_ptr<QgsProcessingModelComponent> mComponent;
    QgsProcessingModelAlgorithm *mModel = nullptr;

    bool mInitialized = false;
    QgsModelDesignerFoldButtonGraphicItem *mExpandTopButton = nullptr;
    QgsModelDesignerFoldButtonGraphicItem *mExpandBottomButton = nullptr;

    QString mLabel;

    QgsModelDesignerFlatButtonGraphicItem *mEditButton = nullptr;
    QgsModelDesignerFlatButtonGraphicItem *mDeleteButton = nullptr;

    QList< QgsModelDesignerSocketGraphicItem * > mInSockets;
    QList< QgsModelDesignerSocketGraphicItem * > mOutSockets;


    static constexpr double MIN_COMPONENT_WIDTH = 70;
    static constexpr double MIN_COMPONENT_HEIGHT = 30;

    static constexpr double DEFAULT_BUTTON_WIDTH = 16;
    static constexpr double DEFAULT_BUTTON_HEIGHT = 16;
    static constexpr double BUTTON_MARGIN = 2;
    static constexpr double SOCKET_MARGIN = 25; //! Margin from the edge of the component to socket
    static constexpr double TEXT_MARGIN = 4;
    static constexpr double RECT_PEN_SIZE = 2;
    static constexpr double RECT_OUTLINE_SIZE = 10;
    QSizeF mButtonSize { DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT };

    QFont mFont;

    bool mIsHovering = false;
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
    QgsModelParameterGraphicItem( QgsProcessingModelParameter *parameter SIP_TRANSFER, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent SIP_TRANSFERTHIS );

    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;

    QColor linkColor( Qt::Edge edge, int index ) const override;

  protected:
    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QPicture iconPicture() const override;

    int linkPointCount( Qt::Edge edge ) const override;
    QString linkPointText( Qt::Edge edge, int index ) const override;
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
    QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child SIP_TRANSFER, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent SIP_TRANSFERTHIS );
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;

    /**
     * Sets the \a results obtained for this child algorithm for the last model execution through the dialog.
     */
    void setResults( const QgsProcessingModelChildAlgorithmResult &results );

    /**
     * Sets the feature count for the source attached to the specified input.
     *
     * This can be used to dynamically update the feature count badge for the matching arrow item.
     *
     * \since QGIS 4.2
     */
    void setSourceFeatureCount( const QString &parameterName, long long featureCount );

    /**
     * Sets the feature count for the sink attached to the specified output.
     *
     * This can be used to dynamically update the feature count badge for the matching arrow item.
     *
     * \since QGIS 4.2
     */
    void setSinkFeatureCount( const QString &outputName, long long featureCount );

    /**
     * Returns the \a results for this child algorithm for the last model execution through the dialog.
     *
     * \since QGIS 4.0
     */
    QgsProcessingModelChildAlgorithmResult results() { return mResults; };

    /**
     * Sets the child's \a progress.
     */
    void setProgress( double progress );

    /**
     * Flags the algorithm as having started.
     *
     * \since QGIS 4.2
     */
    void setStarted();

    /**
     * Flags the algorithm as possibly being outdated (i.e. previous results are invalid due to changes elsewhere in the model).
     *
     * \since QGIS 4.2
     */
    void setOutdated();

    /**
     * Returns the index for the input with the specified parameter name, or -1 if the parameter could not be matched.
     *
     * \see indexForOutput()
     * \since QGIS 4.2
     */
    int indexForInput( const QString &parameterName ) const;

    /**
     * Returns the index for the output with the specified name, or -1 if the output could not be matched.
     *
     * \see indexForInput()
     * \since QGIS 4.2
     */
    int indexForOutput( const QString &output ) const;

  signals:

    /**
     * Emitted when the user opts to run the model from this child algorithm.
     *
     * \since QGIS 3.38
    */
    void runFromHere();

    /**
     * Emitted when the user opts to run selected steps from the model.
     *
     * \since QGIS 3.38
    */
    void runSelected();

    /**
     * Emitted when the user opts to view previous results from this child algorithm.
     *
     * \since QGIS 3.38
     */
    void showPreviousResults();

    /**
    * Emitted when the user opts to view the previous log from this child algorithm.
    *
    * \since QGIS 3.38
    */
    void showLog();

    /**
     * Requests that any associated configuration dock widget is rebuilt to reflect the
     * current state of the child algorithm.
     *
     * \since QGIS 4.2
     */
    void rebuildConfigurationDockWidget();

  protected:
    void paintBackground( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    QColor fillColor( State state ) const override;
    QColor strokeColor( State state ) const override;
    QColor textColor( State state ) const override;
    QColor outlineColor() const override;
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
    bool mStarted = false;
    bool mOutdated = false;
    QgsProcessingModelChildAlgorithmResult mResults;
    double mProgress = -1;
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
    QgsModelOutputGraphicItem( QgsProcessingModelOutput *output SIP_TRANSFER, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent SIP_TRANSFERTHIS );

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
    QgsModelCommentGraphicItem( QgsProcessingModelComment *comment SIP_TRANSFER, QgsModelComponentGraphicItem *parentItem, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent SIP_TRANSFERTHIS );
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

    std::unique_ptr<QgsProcessingModelComponent> mParentComponent;
    QPointer<QgsModelComponentGraphicItem> mParentItem;
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
    QgsModelGroupBoxGraphicItem( QgsProcessingModelGroupBox *box SIP_TRANSFER, QgsProcessingModelAlgorithm *model, QGraphicsItem *parent SIP_TRANSFERTHIS );
    ~QgsModelGroupBoxGraphicItem() override;
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event ) override;
    bool canDeleteComponent() override;

    /**
     * Applies edits to the item, using an updated \a groupBox definition.
     *
     * \since QGIS 4.0
     */
    void applyEdit( const QgsProcessingModelGroupBox &groupBox );

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
