/***************************************************************************
    qgsattributesformtreeviewindicatorprovider.h
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

#ifndef QGSATTRIBUTESFORMTREEVIEWINDICATORPROVIDER_H
#define QGSATTRIBUTESFORMTREEVIEWINDICATORPROVIDER_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgsattributesformview.h"
#include "qgsattributesformtreeviewindicator.h"

#include <QObject>

/**
 * Provides an interface for attributes form tree indicator providers.
 *
 * Subclasses must override:
 *
 * - acceptsItem()
 * - iconName()
 * - tooltipText()
 *
 * \ingroup gui
 * \since QGIS 4.0
 */
class QgsAttributesFormTreeViewIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsAttributesFormTreeViewIndicatorProvider.
     *
     * The provider gets parented to the \a view.
     */
    explicit QgsAttributesFormTreeViewIndicatorProvider( QgsAttributesFormBaseView *view );

    //! Returns whether the provider is enabled or not.
    bool isEnabled();

    /**
     * Enables or disables the provider.
     *
     * \param enabled   Boolead value which specifies whether to enable (TRUE) or disable (FALSE) the provider.
     */
    void setEnabled( bool enabled );

  public slots:
    /**
     * Updates the state of a the indicator for the given \a item.
     *
     * If the indicator does not exist, it will be created.
     * If the indicator exists, but is no longer accepted, it will be removed.
     */
    void updateItemIndicator( QgsAttributesFormItem *item );

  protected slots:
    //! Connects to signals of new items added to the tree
    void onAddedChildren( QgsAttributesFormItem *item, int indexFrom, int indexTo );

  private:
    // Subclasses MUST override:
    //! Item filter: items that pass the test will get the indicator.
    virtual bool acceptsItem( QgsAttributesFormItem *item ) = 0;
    //! Returns the icon name for the given \a item, icon name is passed to QgsApplication::getThemeIcon().
    virtual QString iconName( QgsAttributesFormItem *item ) = 0;
    //! Returns the tooltip text for the given \a item.
    virtual QString tooltipText( QgsAttributesFormItem *item ) = 0;
    // End MUST overrides

    //! Indicator factory
    std::unique_ptr<QgsAttributesFormTreeViewIndicator> newIndicator( QgsAttributesFormItem *item );

    //! Removes the indicator from the given \a item.
    void removeItemIndicator( QgsAttributesFormItem *item );

  protected:
    QgsAttributesFormBaseView *mAttributesFormTreeView = nullptr;
    QSet<QgsAttributesFormTreeViewIndicator *> mIndicators;
    bool mEnabled = false;
};


/**
 * Provides field constraint indicators for attribute form views.
 *
 * \ingroup gui
 * \since QGIS 4.0
 */
class QgsFieldConstraintIndicatorProvider : public QgsAttributesFormTreeViewIndicatorProvider
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsFieldConstraintIndicatorProvider.
     */
    explicit QgsFieldConstraintIndicatorProvider( QgsAttributesFormBaseView *view );

  private:
    bool acceptsItem( QgsAttributesFormItem *item ) override;
    QString iconName( QgsAttributesFormItem *item ) override;
    QString tooltipText( QgsAttributesFormItem *item ) override;
};


/**
 * Provides default value indicators for attribute form views.
 *
 * \ingroup gui
 * \since QGIS 4.0
 */
class QgsFieldDefaultValueIndicatorProvider : public QgsAttributesFormTreeViewIndicatorProvider
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsFieldDefaultValueIndicatorProvider.
     */
    explicit QgsFieldDefaultValueIndicatorProvider( QgsAttributesFormBaseView *view );

  private:
    bool acceptsItem( QgsAttributesFormItem *item ) override;
    QString iconName( QgsAttributesFormItem *item ) override;
    QString tooltipText( QgsAttributesFormItem *item ) override;
};

#endif // QGSATTRIBUTESFORMTREEVIEWINDICATORPROVIDER_H
