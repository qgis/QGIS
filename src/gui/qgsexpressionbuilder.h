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

class QgsExpressionItem : public QStandardItem
{
    public:
        enum ItemType
        {
            Header,
            Field,
            ExpressionNode
        };

        QgsExpressionItem(QString label, QString expressionText, QString helpText, QgsExpressionItem::ItemType itemType = ExpressionNode)
            : QStandardItem(label)
        {
            mExpressionText = expressionText;
            mHelpText = helpText;
            mType = itemType;
        }

        QgsExpressionItem(QString label, QString expressionText, QgsExpressionItem::ItemType itemType = ExpressionNode)
            : QStandardItem(label)
        {
            mExpressionText = expressionText;
            mType = itemType;
        }

        QString getExpressionText() {   return mExpressionText;  }

        QString getHelpText() {  return mHelpText;  }

        void setHelpText(QString helpText) { mHelpText = helpText; }

        QgsExpressionItem::ItemType getItemType() { return mType ; }

    private:
        QString mExpressionText;
        QString mHelpText;
        QgsExpressionItem::ItemType mType;
};

class QgsExpressionBuilderWidget : public QWidget, private Ui::QgsExpressionBuilder {
    Q_OBJECT
public:
    QgsExpressionBuilderWidget(QgsVectorLayer * layer);
    ~QgsExpressionBuilderWidget();

    void loadFieldNames();
    void fillFieldValues(int fieldIndex, int countLimit);
    QString getExpressionString();
    void setExpressionString(const QString expressionString);
    void registerItem(QString group, QString label,QString expressionText,
                      QString helpText = "",QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode);

public slots:
    void on_mAllPushButton_clicked();
    void on_expressionTree_clicked(const QModelIndex &index);
    void on_expressionTree_doubleClicked(const QModelIndex &index);

private:
    QgsVectorLayer *mLayer;
    QStandardItemModel *mModel;
    QMap<QString, QgsExpressionItem*> mExpressionGroups;
};

#endif // QGSEXPRESSIONBUILDER_H
