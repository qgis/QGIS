#ifndef QGSATTRIBUTESFORMVIEW_H
#define QGSATTRIBUTESFORMVIEW_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgsattributesformmodel.h"
#include "qgsattributesformtreeviewindicator.h"
#include "qgsexpressioncontextgenerator.h"

#include <QTreeView>


/**
 * \brief Graphical representation for the attribute drag and drop editor.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormBaseView : public QTreeView, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormBaseView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesFormBaseView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    /**
     * Returns the source model index corresponding to the first selected row.
     *
     * \note The first selected row is the first one the user selected, and not necessarily the one closer to the header.
     */
    QModelIndex firstSelectedIndex() const;

    // QgsExpressionContextGenerator interface
    QgsExpressionContext createExpressionContext() const override;

    /**
     * Returns list of indicators associated with a particular index.
     */
    const QList<QgsAttributesFormTreeViewIndicator *> indicators( const QModelIndex &index ) const;

    /**
     * Returns list of indicators associated with a particular item.
     */
    const QList<QgsAttributesFormTreeViewIndicator *> indicators( QgsAttributesFormItem *item ) const;

    void addIndicator( QgsAttributesFormItem *item, QgsAttributesFormTreeViewIndicator *indicator );

    void removeIndicator( QgsAttributesFormItem *item, QgsAttributesFormTreeViewIndicator *indicator );

    void removeAllIndicators();

    QgsAttributesFormModel *sourceModel() const;

  public slots:
    /**
     * Selects the first item that matches a \a itemType and a \a itemId.
     *
     * Helps to keep in sync selection from both Attribute Widget view and Form Layout view.
     */
    void selectFirstMatchingItem( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId );

    /**
     * Sets the filter text to the underlying proxy model.
     *
     * \param text Filter text to be used to filter source model items.
     */
    void setFilterText( const QString &text );

  protected:
    QgsVectorLayer *mLayer = nullptr;
    QgsAttributesFormProxyModel *mModel = nullptr;

    //! Storage of indicators used with the tree view
    QHash< QgsAttributesFormItem *, QList< QgsAttributesFormTreeViewIndicator * > > mIndicators;
};


/**
 * \brief Graphical representation for the available widgets while configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesAvailableWidgetsView : public QgsAttributesFormBaseView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesAvailableWidgetsView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesAvailableWidgetsView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesFormProxyModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

    //! Access the underlying QgsAttributesAvailableWidgetsModel source model
    QgsAttributesAvailableWidgetsModel *availableWidgetsModel() const;
};


/**
 * \brief Graphical representation for the form layout while configuring attributes forms.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsAttributesFormLayoutView : public QgsAttributesFormBaseView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormLayoutView, with the given \a parent.
     *
     * The given \a layer is used to build an expression context with the layer scope.
     */
    explicit QgsAttributesFormLayoutView( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Overridden setModel() from base class. Only QgsAttributesFormProxyModel is an acceptable model.
    void setModel( QAbstractItemModel *model ) override;

  protected:
    // Drag and drop support (to handle internal moves)
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:
    void onItemDoubleClicked( const QModelIndex &index );
    void handleExternalDroppedItem( QModelIndex &index );
    void handleInternalDroppedItem( QModelIndex &index );
};


#endif // QGSATTRIBUTESFORMVIEW_H
