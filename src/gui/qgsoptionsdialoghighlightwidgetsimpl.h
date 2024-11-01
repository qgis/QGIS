/***************************************************************************
    qgsoptionsdialoghighlightwidgetsimpl.h
     -------------------------------
    Date                 : February 2018
    Copyright            : (C) 2018 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOPTIONSDIALOGHIGHLIGHTWIDGETSIMPL_H
#define QGSOPTIONSDIALOGHIGHLIGHTWIDGETSIMPL_H

#include <QObject>
#include <QMap>
#include <QBrush>


#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsoptionsdialoghighlightwidget.h"

class QLabel;
class QCheckBox;
class QAbstractButton;
class QGroupBox;
class QTreeView;
class QTreeWidgetItem;
class QTableView;


/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightLabel
 * \brief A highlight widget for labels.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 */
class GUI_EXPORT QgsOptionsDialogHighlightLabel : public QgsOptionsDialogHighlightWidget
{
  public:
    //! constructs a highlight widget for a label
    QgsOptionsDialogHighlightLabel( QLabel *label );

  protected:
    bool searchText( const QString &text ) override;
    bool highlightText( const QString &text ) override;
    void reset() override;
    QPointer<QLabel> mLabel;
    QString mStyleSheet;
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightCheckBox
 * \brief A highlight widget for checkboxes.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 */
class GUI_EXPORT QgsOptionsDialogHighlightCheckBox : public QgsOptionsDialogHighlightWidget
{
  public:
    //! constructs a highlight widget for a checkbox
    QgsOptionsDialogHighlightCheckBox( QCheckBox *checkBox );

  protected:
    bool searchText( const QString &text ) override;
    bool highlightText( const QString &text ) override;
    void reset() override;
    QPointer<QCheckBox> mCheckBox;
    QString mStyleSheet;
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightButton
 * \brief A highlight widget for buttons.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 */
class GUI_EXPORT QgsOptionsDialogHighlightButton : public QgsOptionsDialogHighlightWidget
{
  public:
    //! constructs a highlight widget for a button.
    QgsOptionsDialogHighlightButton( QAbstractButton *button );

  protected:
    bool searchText( const QString &text ) override;
    bool highlightText( const QString &text ) override;
    void reset() override;
    QPointer<QAbstractButton> mButton;
    QString mStyleSheet;
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightGroupBox
 * \brief A highlight widget for group boxes.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 */
class GUI_EXPORT QgsOptionsDialogHighlightGroupBox : public QgsOptionsDialogHighlightWidget
{
  public:
    //! constructs a highlight widget for a group box.
    QgsOptionsDialogHighlightGroupBox( QGroupBox *groupBox );

  protected:
    bool searchText( const QString &text ) override;
    bool highlightText( const QString &text ) override;
    void reset() override;
    QPointer<QGroupBox> mGroupBox;
    QString mStyleSheet;
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightTree
 * \brief A highlight widget for trees.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * Highlighting is only available for tree widgets only while searching can be performed
 * in any tree view or inherited class.
 */
class GUI_EXPORT QgsOptionsDialogHighlightTree : public QgsOptionsDialogHighlightWidget
{
  public:
    //! constructs a highlight widget for a tree view or widget.
    QgsOptionsDialogHighlightTree( QTreeView *treeView );

  protected:
    bool searchText( const QString &text ) override;
    bool highlightText( const QString &text ) override;
    void reset() override;
    QPointer<QTreeView> mTreeView;
    // a map to save the tree state (backouground, font, expanded) before highlighting items
    QMap<QTreeWidgetItem *, bool> mTreeInitialExpand = QMap<QTreeWidgetItem *, bool>();
    QMap<QTreeWidgetItem *, bool> mTreeInitialVisible = QMap<QTreeWidgetItem *, bool>();
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightTable
 * \brief A highlight widget for table widgets.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsOptionsDialogHighlightTable : public QgsOptionsDialogHighlightWidget
{
  public:
    //! constructs a highlight widget for a table view or widget.
    QgsOptionsDialogHighlightTable( QTableView *tableView );

  protected:
    bool searchText( const QString &text ) override;
    bool highlightText( const QString &text ) override;
    void reset() override;
    QPointer<QTableView> mTableView;
};
#endif // QGSOPTIONSDIALOGHIGHLIGHTWIDGETSIMPL_H
