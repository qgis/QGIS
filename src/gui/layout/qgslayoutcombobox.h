/***************************************************************************
   qgslayoutcombobox.h
    --------------------------------------
   Date                 : March 2019
   Copyright            : (C) 2019 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSLAYOUTCOMBOBOX_H
#define QGSLAYOUTCOMBOBOX_H

#include <QComboBox>
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgslayoutmanager.h"

/**
 * \class QgsLayoutComboBox
 * \ingroup gui
 * \brief The QgsLayoutComboBox class is a combo box which displays available layouts from a QgsLayoutManager.
 * \since QGIS 3.8
 */
class GUI_EXPORT QgsLayoutComboBox : public QComboBox
{
    Q_OBJECT

  public:
    /**
     * QgsLayoutComboBox creates a combo box to display a list of items in a
     * layout \a manager. The layouts can optionally be filtered by type.
     */
    explicit QgsLayoutComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsLayoutManager *manager = nullptr );

    /**
     * Sets the layout \a manager containing the layouts to list in the combo box.
     */
    void setLayoutManager( QgsLayoutManager *manager );

    /**
     * Returns the current filters used for filtering available layouts.
     *
     * \see setFilters()
     */
    QgsLayoutManagerProxyModel::Filters filters() const;

    /**
     * Sets the current \a filters used for filtering available layouts.
     *
     * \see filters()
     */
    void setFilters( QgsLayoutManagerProxyModel::Filters filters );

    /**
     * Sets whether an optional empty layout ("not set") option is present in the combobox.
     * \see allowEmptyLayout()
     */
    void setAllowEmptyLayout( bool allowEmpty );

    /**
     * Returns TRUE if the combobox includes the empty layout ("not set") choice.
     * \see setAllowEmptyLayout()
     */
    bool allowEmptyLayout() const;

    /**
     * Returns the layout currently selected in the combo box.
     */
    QgsMasterLayoutInterface *currentLayout() const;

    /**
     * Returns the layout at the specified \a index.
     */
    QgsMasterLayoutInterface *layout( int index ) const;

  public slots:

    /**
     * Sets the currently selected \a layout in the combo box.
     */
    void setCurrentLayout( QgsMasterLayoutInterface *layout );

  signals:

    //! Emitted whenever the currently selected layout changes
    void layoutChanged( QgsMasterLayoutInterface *layout );

  private slots:
    void indexChanged( int i );
    void rowsChanged();

  private:
    QgsLayoutManagerModel *mModel = nullptr;
    QgsLayoutManagerProxyModel *mProxyModel = nullptr;
};

#endif // QGSLAYOUTCOMBOBOX_H
