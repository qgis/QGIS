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


/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightLabel
 * A highlight widget for labels.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsDialogHighlightLabel : public QgsOptionsDialogHighlightWidget
{
    Q_OBJECT
  public:
    //! constructs a highlight widget for a label
    QgsOptionsDialogHighlightLabel( QLabel *label );
  protected:
    virtual bool searchText( const QString &text ) override;
    virtual bool highlightText( const QString &text ) override;
    virtual void reset() override;
    QPointer<QLabel> mLabel;
    QString mStyleSheet = QStringLiteral( /*!search!*/"QLabel { background-color: yellow; color: blue;}/*!search!*/" );
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightCheckBox
 * A highlight widget for checkboxes.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsDialogHighlightCheckBox : public QgsOptionsDialogHighlightWidget
{
    Q_OBJECT
  public:
    //! constructs a highlight widget for a checkbox
    QgsOptionsDialogHighlightCheckBox( QCheckBox *checkBox );
  protected:
    virtual bool searchText( const QString &text ) override;
    virtual bool highlightText( const QString &text ) override;
    virtual void reset() override;
    QPointer<QCheckBox> mCheckBox;
    QString mStyleSheet = QStringLiteral( "/*!search!*/QCheckBox { background-color: yellow; color: blue;}/*!search!*/" );
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightButton
 * A highlight widget for buttons.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsDialogHighlightButton : public QgsOptionsDialogHighlightWidget
{
    Q_OBJECT
  public:
    //! constructs a highlight widget for a button.
    QgsOptionsDialogHighlightButton( QAbstractButton *button );
  protected:
    virtual bool searchText( const QString &text ) override;
    virtual bool highlightText( const QString &text ) override;
    virtual void reset() override;
    QPointer<QAbstractButton> mButton;
    QString mStyleSheet = QStringLiteral( "/*!search!*/QAbstractButton { background-color: yellow; color: blue;}/*!search!*/" );
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightGroupBox
 * A highlight widget for group boxes.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsDialogHighlightGroupBox : public QgsOptionsDialogHighlightWidget
{
    Q_OBJECT
  public:
    //! constructs a highlight widget for a group box.
    QgsOptionsDialogHighlightGroupBox( QGroupBox *groupBox );
  protected:
    virtual bool searchText( const QString &text ) override;
    virtual bool highlightText( const QString &text ) override;
    virtual void reset() override;
    QPointer<QGroupBox> mGroupBox;
    QString mStyleSheet = QStringLiteral( "/*!search!*/QGroupBox::title { background-color: yellow; color: blue;}/*!search!*/" );
};

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightTree
 * A highlight widget for trees.
 * This is used to search and highlight text in QgsOptionsDialogBase implementations.
 * Highlighting is only available for tree widgets only while searching can be performed
 * in any tree view or inherited class.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsDialogHighlightTree : public QgsOptionsDialogHighlightWidget
{
    Q_OBJECT
  public:
    //! constructs a highlight widget for a tree view or widget.
    QgsOptionsDialogHighlightTree( QTreeView *treeView );
  protected:
    virtual bool searchText( const QString &text ) override;
    virtual bool highlightText( const QString &text ) override;
    virtual void reset() override;
    QPointer<QTreeView> mTreeView;
    // a map to save the tree state (backouground, font, expanded) before highlighting items
    QMap<QTreeWidgetItem *, QPair<QBrush, QBrush>> mTreeInitialStyle = QMap<QTreeWidgetItem *, QPair<QBrush, QBrush>>();
    QMap<QTreeWidgetItem *, bool> mTreeInitialExpand = QMap<QTreeWidgetItem *, bool>();
};
#endif // QGSOPTIONSDIALOGHIGHLIGHTWIDGETSIMPL_H
