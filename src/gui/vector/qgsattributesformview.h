/***************************************************************************
    qgsattributesformview.h
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESFORMVIEW_H
#define QGSATTRIBUTESFORMVIEW_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgsattributesformmodel.h"

class QgsAttributesFormTreeViewIndicator;

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
     * Returns the list of indicators associated with a given \a index.
     *
     * \since QGIS 4.0
     */
    const QList<QgsAttributesFormTreeViewIndicator *> indicators( const QModelIndex &index ) const;

    /**
     * Returns the list of indicators associated with a given \a item.
     *
     * \since QGIS 4.0
     */
    const QList<QgsAttributesFormTreeViewIndicator *> indicators( QgsAttributesFormItem *item ) const;

    /**
     * Adds the \a indicator to the given \a item.
     *
     * \since QGIS 4.0
     */
    void addIndicator( QgsAttributesFormItem *item, QgsAttributesFormTreeViewIndicator *indicator );

    /**
     * Removes the \a indicator from the given \a item.
     *
     * \since QGIS 4.0
     */
    void removeIndicator( QgsAttributesFormItem *item, QgsAttributesFormTreeViewIndicator *indicator );

    /**
     * Removes all indicators in the current view.
     *
     * \since QGIS 4.0
     */
    void removeAllIndicators();

    /**
     * Returns the underlying QgsAttributesFormModel model where the view gets data from.
     *
     * \since QGIS 4.0
     */
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
