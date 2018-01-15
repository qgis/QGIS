/***************************************************************************
     qgsdetaileditemdata.h  -  A data represenation for a rich QItemData subclass
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


#include <QMetaType>
#include <QString>
#include <QPixmap>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * This class is the data only representation of a
 * QgsDetailedItemWidget, designed to be used in custom views.
 */
class GUI_EXPORT QgsDetailedItemData
{
  public:

    /**
     * Constructor for QgsDetailedItemData.
     */
    QgsDetailedItemData() = default;

    void setTitle( const QString &title );
    void setDetail( const QString &detail );
    void setCategory( const QString &category );
    void setIcon( const QPixmap &icon );
    void setCheckable( const bool flag );
    void setChecked( const bool flag );
    void setEnabled( bool flag );

    /**
     * This is a hint to the delegate to render using
     * a widget rather than manually painting every
     * part of the list item.
     * \note the delegate may completely ignore this
     * depending on the delegate implementation.
     */
    void setRenderAsWidget( bool flag );

    QString title() const;
    QString detail() const;
    QString category() const;
    QPixmap icon() const;
    bool isCheckable() const;
    bool isChecked() const;
    bool isEnabled() const;
    bool isRenderedAsWidget() const;

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
