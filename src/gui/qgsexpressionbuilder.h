/***************************************************************************
    qgisexpressionbuilder.h - A genric expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2006 by Nathan Woodrow
    Email                : nathan.woodrow at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONBUILDER_H
#define QGSEXPRESSIONBUILDER_H

#include <QWidget>
#include "ui_qgsexpressionbuilder.h"
#include "qgsvectorlayer.h"

#include "QStandardItemModel"
#include "QStandardItem"
#include "QSortFilterProxyModel"

class QgsExpressionItemSearhProxy : public QSortFilterProxyModel
{
    public:
        QgsExpressionItemSearhProxy()
        {
        }

        bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
        {
            if (source_parent == qobject_cast<QStandardItemModel*>(sourceModel())->invisibleRootItem()->index())
                return true;

            QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
        }
};

class QgsExpressionItem : public QStandardItem
{
    public:
        enum ItemType
        {
            Header,
            Field,
            ExpressionNode
        };

        QgsExpressionItem(QString label,
                          QString expressionText,
                          QString helpText,
                          QgsExpressionItem::ItemType itemType = ExpressionNode)
            : QStandardItem(label)
        {
            mExpressionText = expressionText;
            mHelpText = helpText;
            mType = itemType;
        }

        QgsExpressionItem(QString label,
                          QString expressionText,
                          QgsExpressionItem::ItemType itemType = ExpressionNode)
            : QStandardItem(label)
        {
            mExpressionText = expressionText;
            mType = itemType;
        }

        QString getExpressionText() {   return mExpressionText;  }

        /** Get the help text that is associated with this expression item.
          *
          * @return The help text.
          */
        QString getHelpText() {  return mHelpText;  }
        /** Set the help text for the current item
          *
          * @note The help text can be set as a html string.
          */
        void setHelpText(QString helpText) { mHelpText = helpText; }

        /** Get the type of expression item eg header, field, ExpressionNode.
          *
          * @return The QgsExpressionItem::ItemType
          */
        QgsExpressionItem::ItemType getItemType() { return mType ; }

    private:
        QString mExpressionText;
        QString mHelpText;
        QgsExpressionItem::ItemType mType;
};

/** A reusable widget that can be used to build a expression string. */
class QgsExpressionBuilderWidget : public QWidget, private Ui::QgsExpressionBuilder {
    Q_OBJECT
public:
    QgsExpressionBuilderWidget(QWidget *parent);
    ~QgsExpressionBuilderWidget();

    /** Sets layer in order to get the fields and values
      * @note this needs to be called before calling loadFieldNames().
      */
    void setLayer( QgsVectorLayer* layer );

    /** Loads all the field names from the layer.
      * @remarks Should this really be public couldn't we just do this for the user?
      */
    void loadFieldNames();

    /** Gets the expression string that has been set in the expression area.
      * @returns The expression as a string. */
    QString getExpressionString();

    /** Sets the expression string for the widget */
    void setExpressionString(const QString expressionString);

    /** Registers a node item for the expression builder.
      * @param group The group the item will be show in the tree view.  If the group doesn't exsit it will be created.
      * @param label The label that is show to the user for the item in the tree.
      * @param expressionText The text that is inserted into the expression area when the user double clicks on the item.
      * @param helpText The help text that the user will see when item is selected.
      * @param type The type of the expression item.
      */
    void registerItem(QString group, QString label,QString expressionText,
                      QString helpText = "",
                      QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode);

    /** Does the expression used in the widget have any errors
      * @note Users of this widget can check this to see if they should let the
      * user move forward.
      */
    bool hasExpressionError();

public slots:
    void on_expressionTree_clicked(const QModelIndex &index);
    void on_expressionTree_doubleClicked(const QModelIndex &index);
    void on_txtExpressionString_textChanged();
    void on_txtSearchEdit_textChanged();
    void showContextMenu( const QPoint & );
    void loadSampleValues();
    void loadAllValues();

signals:
    void expressionParsed(bool isVaild);

private:
    void fillFieldValues(int fieldIndex, int countLimit);

    QgsVectorLayer *mLayer;
    QStandardItemModel *mModel;
    QgsExpressionItemSearhProxy *mProxyModel;
    QMap<QString, QgsExpressionItem*> mExpressionGroups;
};

#endif // QGSEXPRESSIONBUILDER_H
