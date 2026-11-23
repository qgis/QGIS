/***************************************************************************
                             qgsfilterablecombobox.h
                             ------------------------
    Date                 : November 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILTERABLECOMBOBOX_H
#define QGSFILTERABLECOMBOBOX_H

#include "qgis.h"
#include "qgis_gui.h"

#include <QComboBox>

class QgsFilterLineEdit;
class QCompleter;

/**
 * \ingroup gui
 * \brief A QComboBox with entries that can be filtered by partial string match.
 *
 * Combobox entries will be filtered when the widget is focused and the user
 * starts to enter a text string. The filtering is case-insensitive, and filters
 * on any partial match.
 *
 * The filtering is transient, and will be immediately cleared when the widget
 * is defocused. This ensures that the user is forced to have a valid selection
 * from the combo box entries and cannot have the widget set to an ad-hoc string.
 *
 * \since QGIS 4.6
 */
class GUI_EXPORT QgsFilterableComboBox : public QComboBox
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsFilterableComboBox, with the specified \a parent widget.
     */
    QgsFilterableComboBox( QWidget *parent = nullptr );

  signals:

    /**
     * Emitted when a item is preselected (eg hovered with the mouse).
     *
     * \a param data preselected item data.
     *
     * \see itemDataPreselectionCleared()
     */
    void itemDataPreselected( const QVariant &data );

    /**
     * Emitted when no item is preselected anymore.
     *
     * \see itemDataPreselected()
     */
    void itemDataPreselectionCleared();

  protected:
    bool eventFilter( QObject *obj, QEvent *event ) override;

  private slots:

    void openCompleterPopup();
    void onItemHighlightedByCompleter( const QModelIndex &index );
    void onItemHighlighted( int index );
    void clearPreselection();

  private:
    QgsFilterLineEdit *mLineEdit = nullptr;
    QCompleter *mCompleter = nullptr;
    QPersistentModelIndex mLastValidIndex;
    bool mHasPreselectedItem = false;
};

#endif // QGSFILTERABLECOMBOBOX_H
