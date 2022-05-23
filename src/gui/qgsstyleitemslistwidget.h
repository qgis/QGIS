/***************************************************************************
 qgsstyleitemslistwidget.h
 -------------------------
 begin                : June 2019
 copyright            : (C) 2019 by Nyall Dawson
 email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEITEMSLISTWIDGET_H
#define QGSSTYLEITEMSLISTWIDGET_H

#include "ui_qgsstyleitemslistwidgetbase.h"

#include "qgsstylemodel.h"
#include <QWidget>
#include <QStyledItemDelegate>
#include "qgis_gui.h"

class QgsStyle;
class QMenu;
class QgsCombinedStyleModel;


#ifndef SIP_RUN
///@cond PRIVATE
class QgsReadOnlyStyleModel : public QgsStyleProxyModel
{
    Q_OBJECT
  public:

    explicit QgsReadOnlyStyleModel( QgsStyleModel *sourceModel, QObject *parent = nullptr );
    explicit QgsReadOnlyStyleModel( QgsStyle *style, QObject *parent = nullptr );

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    explicit QgsReadOnlyStyleModel( QgsCombinedStyleModel *style, QObject *parent = nullptr );
#endif

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

};

/**
 * \ingroup gui
 * \class QgsStyleModelDelegate
 * \brief Custom delegate for formatting style models.
 * \note Not available in Python bindings
 * \since QGIS 3.26
 */
class QgsStyleModelDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsStyleModelDelegate, with the specified \a parent object.
     */
    QgsStyleModelDelegate( QObject *parent );

    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};

#endif
///@endcond

/**
 * \ingroup gui
 * \class QgsStyleItemsListWidget
 * \brief A reusable widget for showing a filtered list of entities from a QgsStyle database.
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsStyleItemsListWidget : public QWidget, private Ui::QgsStyleItemsListWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsStyleItemsListWidget, with the specified \a parent widget.
     */
    QgsStyleItemsListWidget( QWidget *parent SIP_TRANSFERTHIS );

    /**
     * Sets the \a style database associated with the widget.
     *
     * Ownership of \a style is not transferred, and the caller is responsible for ensuring that
     * it exists for the lifetime of the widget.
     */
    void setStyle( QgsStyle *style );

    /**
     * Sets the \a type of style entity to show in the widget.
     *
     * \see setSymbolType()
     */
    void setEntityType( QgsStyle::StyleEntity type );

    /**
     * Sets the \a types of style entity to show in the widget.
     *
     * \note Not available in Python bindings.
     */
    void setEntityTypes( const QList<QgsStyle::StyleEntity> &filters ) SIP_SKIP;

    /**
     * Sets the \a type of symbols to show in the widget.
     *
     * \see setEntityType()
     */
    void setSymbolType( Qgis::SymbolType type );

    /**
     * Sets the layer \a type to show in the widget. Set \a type to QgsWkbTypes::UnknownGeometry if no
     * layer type filter is desired.
     *
     * This setting only applies to label settings and 3d style entities.
     */
    void setLayerType( QgsWkbTypes::GeometryType type );

    /**
     * Returns the current tag filter set for the widget, if any is set.
     */
    QString currentTagFilter() const;

#ifndef SIP_RUN

    /**
     * Returns a pointer to the widget's current advanced menu.
     * \see setAdvancedMenu()
     * \note Not available in Python bindings.
     */
    QMenu *advancedMenu();

    /**
     * Sets the widget's advanced \a menu, which is shown when the user clicks
     * the "Advanced" button in the widget's GUI.
     *
     * Ownership of \a menu is NOT transferred to the widget.
     *
     * \see advancedMenu()
     * \note Not available in Python bindings.
     */
    void setAdvancedMenu( QMenu *menu );

    /**
     * Sets whether the advanced button should be shown in the widget. By default
     * the button is hidden.
     *
     * \see setAdvancedMenu()
     * \note Not available in Python bindings.
     */
    void showAdvancedButton( bool enabled );
#endif

    /**
     * Returns the name of the item currently selected in the widget.
     * \see currentEntityType()
     */
    QString currentItemName() const;

    /**
     * Returns the type of the item currently selected in the widget.
     * \see currentItemName()
     */
    QgsStyle::StyleEntity currentEntityType() const;

  protected:

    void showEvent( QShowEvent *event ) override;

  signals:

    /**
     * Emitted when the selected item is changed in the widget.
     * \param name Newly selected item name
     * \param type Newly selected item type
     */
    void selectionChanged( const QString &name, QgsStyle::StyleEntity type );

    /**
     * Emitted when the selected item is changed in the widget.
     * \param name Newly selected item name
     * \param type Newly selected item type
     * \param stylePath file path to associated style database
     *
     * \since QGIS 3.26
     */
    void selectionChangedWithStylePath( const QString &name, QgsStyle::StyleEntity type, const QString &stylePath );

    /**
     * Emitted when the user has opted to save a new entity to the style
     * database, by clicking the "Save" button in the widget.
     *
     * It is the caller's responsibility to handle this in an appropriate
     * manner given the context of the widget.
     */
    void saveEntity();

  private slots:
    void groupsCombo_currentIndexChanged( int index );
    void updateModelFilters();
    void onSelectionChanged( const QModelIndex &index );
    void populateGroups();
    void openStyleManager();

  private:
    QgsStyle *mStyle = nullptr;
    QgsStyleProxyModel *mModel = nullptr;
    QgsStyleModelDelegate *mDelegate = nullptr;
    bool mUpdatingGroups = false;
};

#endif //QGSSTYLEITEMSLISTWIDGET_H



