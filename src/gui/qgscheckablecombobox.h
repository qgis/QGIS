/***************************************************************************
                              qgscheckablecombobox.h
                              ------------------------
  begin                : March 21, 2017
  copyright            : (C) 2017 by Alexander Bruy
  email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCHECKABLECOMBOBOX_H
#define QGSCHECKABLECOMBOBOX_H

#include <QComboBox>
#include <QMenu>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include "qgis.h"
#include "qgis_gui.h"

class QEvent;

/**
 * \class QgsCheckableItemModel
 * \ingroup gui
 * QStandardItemModel subclass which makes all items checkable
 * by default.
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
#ifndef SIP_RUN
class QgsCheckableItemModel : public QStandardItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCheckableItemModel.
     * \param parent parent object
     */
    QgsCheckableItemModel( QObject *parent = nullptr );

    /**
     * Returns a combination of the item flags: items are enabled
     * (ItemIsEnabled), selectable (ItemIsSelectable) and checkable
     * (ItemIsUserCheckable).
     * \param index item index
     */
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Returns the data stored under the given role for the item
     * referred to by the index.
     * \param index item index
     * \param role data role
     */
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    /**
     * Sets the role data for the item at index to value.
     * \param index item index
     * \param value data value
     * \param role data role
     * \returns true on success, false otherwise
     */
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

  signals:

    /**
     * This signal is emitted whenever the items checkstate has changed.
     */
    void itemCheckStateChanged();
};


/**
 * \class QgsCheckBoxDelegate
 * \ingroup gui
 * QStyledItemDelegate subclass for QgsCheckableComboBox. Needed for
 * correct drawing of the checkable items on Mac and GTK.
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
class QgsCheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCheckBoxDelegate.
     * \param parent parent object
     */
    QgsCheckBoxDelegate( QObject *parent = nullptr );

    /**
     * Renders the delegate using the given painter and style option
     * for the item specified by index.
     * \param painter painter to use
     * \param option style option
     * \param index item index
     */
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};
#endif

/**
 * \class QgsCheckableComboBox
 * \ingroup gui
 * QComboBox subclass which allows selecting multiple items.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsCheckableComboBox : public QComboBox
{
    Q_OBJECT

    Q_PROPERTY( QString separator READ separator WRITE setSeparator )
    Q_PROPERTY( QString defaultText READ defaultText WRITE setDefaultText )
    Q_PROPERTY( QStringList checkedItems READ checkedItems WRITE setCheckedItems )

  public:

    /**
     * Constructor for QgsCheckableComboBox.
     */
    QgsCheckableComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns separator used to separate items in the display text.
     * \see setSeparator()
     */
    QString separator() const;

    /**
     * Set separator used to separate items in the display text.
     * \param separator separator to use
     * \see separator()
     */
    void setSeparator( const QString &separator );

    /**
     * Returns default text which will be displayed in the widget
     * when no items selected.
     * \see setDefaultText()
     */
    QString defaultText() const;

    /**
     * Set default text which will be displayed in the widget when
     * no items selected.
     * \param text default text
     * \see defaultText()
     */
    void setDefaultText( const QString &text );

    /**
     * Returns currently checked items.
     * \see setCheckedItems()
     */
    QStringList checkedItems() const;

    /**
     * Returns the checked state of the item identified by index
     * \param index item index
     * \see setItemCheckState()
     * \see toggleItemCheckState()
     */
    Qt::CheckState itemCheckState( int index ) const;

    /**
     * Sets the item check state to state
     * \param index item index
     * \param state check state
     * \see itemCheckState()
     * \see toggleItemCheckState()
     */
    void setItemCheckState( int index, Qt::CheckState state );

    /**
     * Toggles the item check state
     * \param index item index
     * \see itemCheckState()
     * \see setItemCheckState()
     */
    void toggleItemCheckState( int index );

    /**
     * Hides the list of items in the combobox if it is currently
     * visible and resets the internal state.
     */
    void hidePopup() override;

    /**
     * Filters events to enable context menu
     */
    bool eventFilter( QObject *object, QEvent *event ) override;

  signals:

    /**
     * This signal is emitted whenever the checked items list changed.
     */
    void checkedItemsChanged( const QStringList &items );

  public slots:

    /**
     * Set items which should be checked/selected.
     * \param items items to select
     * \see checkedItems()
     */
    void setCheckedItems( const QStringList &items );

  protected:

    /**
     * Handler for widget resizing
     */
    void resizeEvent( QResizeEvent *event ) override;

  protected slots:

    /**
     * Display context menu which allows selecting/deselecting
     * all items at once.
     */
    void showContextMenu( QPoint pos );

    /**
     * Selects all items.
     */
    void selectAllOptions();

    /**
     * Removes selection from all items.
     */
    void deselectAllOptions();

  private:
    void updateCheckedItems();
    void updateDisplayText();

    QString mSeparator;
    QString mDefaultText;

    bool mSkipHide = false;

    QMenu *mContextMenu = nullptr;
    QAction *mSelectAllAction = nullptr;
    QAction *mDeselectAllAction = nullptr;
};

#endif // QGSCHECKABLECOMBOBOX_H
