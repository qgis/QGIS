/***************************************************************************
    qgsrulebasedlabelingwidget.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRULEBASEDLABELINGWIDGET_H
#define QGSRULEBASEDLABELINGWIDGET_H

#include <QWidget>

#include "qgspanelwidget.h"

#include "ui_qgsrulebasedlabelingwidget.h"

#include "qgsrulebasedlabeling.h"
#include "qgis_app.h"

class QgsMapCanvas;
class QgsVectorLayer;


class APP_EXPORT QgsRuleBasedLabelingModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    QgsRuleBasedLabelingModel( QgsRuleBasedLabeling::Rule *rootRule, QObject *parent = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    //! provide model index for parent's child item
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    //! provide parent model index
    QModelIndex parent( const QModelIndex &index ) const override;

    // editing support
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    // drag'n'drop support
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    // new methods

    QgsRuleBasedLabeling::Rule *ruleForIndex( const QModelIndex &index ) const;

    void insertRule( const QModelIndex &parent, int before, QgsRuleBasedLabeling::Rule *newrule );
    void updateRule( const QModelIndex &parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex &index );
    void removeRule( const QModelIndex &index );

    void willAddRules( const QModelIndex &parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

  protected:
    QgsRuleBasedLabeling::Rule *mRootRule = nullptr;
};


class QgsLabelingRulePropsWidget;


class QgsRuleBasedLabelingWidget : public QgsPanelWidget, private Ui::QgsRuleBasedLabelingWidget
{
    Q_OBJECT
  public:
    QgsRuleBasedLabelingWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );
    ~QgsRuleBasedLabelingWidget() override;

    //! Gives access to the internal root of the rule tree
    const QgsRuleBasedLabeling::Rule *rootRule() const { return mRootRule; }

  protected slots:
    void addRule();
    void editRule();
    void editRule( const QModelIndex &index );
    void removeRule();
    void copy();
    void paste();

  private slots:
    void ruleWidgetPanelAccepted( QgsPanelWidget *panel );
    void liveUpdateRuleFromPanel();

  protected:
    QgsRuleBasedLabeling::Rule *currentRule();

  protected:
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QgsRuleBasedLabeling::Rule *mRootRule = nullptr;
    QgsRuleBasedLabelingModel *mModel = nullptr;

    QAction *mCopyAction = nullptr;
    QAction *mPasteAction = nullptr;
    QAction *mDeleteAction = nullptr;
};


//////

class QgsLabelingGui;

#include "ui_qgslabelingrulepropswidget.h"

class APP_EXPORT QgsLabelingRulePropsWidget : public QgsPanelWidget, private Ui::QgsLabelingRulePropsWidget
{
    Q_OBJECT

  public:
    enum Mode
    {
      Adding,
      Editing
    };

    QgsLabelingRulePropsWidget( QgsRuleBasedLabeling::Rule *rule, QgsVectorLayer *layer,
                                QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr );
    ~QgsLabelingRulePropsWidget() override;

    QgsRuleBasedLabeling::Rule *rule() { return mRule; }

    void setDockMode( bool dockMode ) override;

  public slots:
    void testFilter();
    void buildExpression();

    /**
     * Apply any changes from the widget to the set rule.
     */
    void apply();

  protected:
    QgsRuleBasedLabeling::Rule *mRule; // borrowed
    QgsVectorLayer *mLayer = nullptr;

    QgsLabelingGui *mLabelingGui = nullptr;
    QgsPalLayerSettings *mSettings; // a clone of original settings

    QgsMapCanvas *mMapCanvas = nullptr;
};


#endif // QGSRULEBASEDLABELINGWIDGET_H
