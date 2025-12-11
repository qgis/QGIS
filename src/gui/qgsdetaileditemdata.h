/***************************************************************************
     qgsdetaileditemdata.h  -  A data representation for a rich QItemData subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDETAILEDITEMDATA_H
#define QGSDETAILEDITEMDATA_H


#include "qgis_gui.h"

#include <QMetaType>
#include <QPixmap>
#include <QString>

/**
 * \ingroup gui
 * \brief The data only representation of a
 * QgsDetailedItemWidget, designed to be used in custom views.
 */
class GUI_EXPORT QgsDetailedItemData
{
  public:
    QgsDetailedItemData() = default;

    /**
     * Sets the \a title for the item.
     * \see title()
     */
    void setTitle( const QString &title );

    /**
     * Sets the detailed description for the item.
     * \see detail()
     */
    void setDetail( const QString &detail );

    /**
     * Sets the item's \a category.
     * \see category()
     */
    void setCategory( const QString &category );

    /**
     * Sets the item's \a icon.
     * \see icon()
     */
    void setIcon( const QPixmap &icon );

    /**
     * Sets whether the item is checkable.
     * \see isCheckable()
     */
    void setCheckable( bool flag );

    /**
     * Sets whether the item is checked.
     * \see isChecked()
     */
    void setChecked( bool flag );

    /**
     * Sets whether the item is enabled.
     * \see isEnabled()
     */
    void setEnabled( bool flag );

    /**
     * This is a hint to the delegate to render using
     * a widget rather than manually painting every
     * part of the list item.
     * \note the delegate may completely ignore this
     * depending on the delegate implementation.
     * \see isRenderedAsWidget()
     */
    void setRenderAsWidget( bool flag );

    /**
     * Returns the item's title.
     * \see setTitle()
     */
    [[nodiscard]] QString title() const;

    /**
     * Returns the detailed description for the item.
     * \see setDetail()
     */
    [[nodiscard]] QString detail() const;

    /**
     * Returns the item's category.
     * \see setCategory()
     */
    [[nodiscard]] QString category() const;

    /**
     * Returns the item's icon.
     * \see setIcon()
     */
    [[nodiscard]] QPixmap icon() const;

    /**
     * Returns TRUE if the item is checkable.
     * \see setCheckable()
     */
    [[nodiscard]] bool isCheckable() const;

    /**
     * Returns TRUE if the item is checked.
     * \see setChecked()
     */
    [[nodiscard]] bool isChecked() const;

    /**
     * Returns TRUE if the item is enabled.
     * \see setEnabled()
     */
    [[nodiscard]] bool isEnabled() const;

    /**
     * Returns TRUE if the item will be rendered using a widget.
     * \see setRenderAsWidget()
     */
    [[nodiscard]] bool isRenderedAsWidget() const;

  private:
    QString mTitle;
    QString mDetail;
    QString mCategory;
    QString mLibraryName;
    QPixmap mPixmap;
    bool mCheckableFlag = false;
    bool mCheckedFlag = false;
    bool mEnabledFlag = true;
    bool mRenderAsWidgetFlag = false;
};

// Make QVariant aware of this data type (see qtdocs star
// rating delegate example for more details)
Q_DECLARE_METATYPE( QgsDetailedItemData )
#endif //QGSDETAILEDITEMDATA_H
