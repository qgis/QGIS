/***************************************************************************
  qgsrulebased3drendererwidget.h
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRULEBASED3DRENDERERWIDGET_H
#define QGSRULEBASED3DRENDERERWIDGET_H

#include <QWidget>

#include "qgspanelwidget.h"

#include <memory>

#include "ui_qgsrulebased3drendererwidget.h"

#include "qgsrulebased3drenderer.h"

class QgsVectorLayer;
class QgsRuleBased3DRenderer;

#include <QAbstractItemModel>


class QgsRuleBased3DRendererModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    QgsRuleBased3DRendererModel( QgsRuleBased3DRenderer::Rule *rootRule, QObject *parent = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
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

    QgsRuleBased3DRenderer::Rule *ruleForIndex( const QModelIndex &index ) const;

    void insertRule( const QModelIndex &parent, int before, QgsRuleBased3DRenderer::Rule *newrule );
    void updateRule( const QModelIndex &parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex &index );
    void removeRule( const QModelIndex &index );

    void willAddRules( const QModelIndex &parent, int count ); // call beginInsertRows
    void finishedAddingRules();                                // call endInsertRows

  protected:
    QgsRuleBased3DRenderer::Rule *mRootRule = nullptr;
};


class QgsRuleBased3DRendererWidget : public QgsPanelWidget, private Ui::QgsRuleBased3DRendererWidget
{
    Q_OBJECT

  public:
    QgsRuleBased3DRendererWidget( QWidget *parent = nullptr );
    ~QgsRuleBased3DRendererWidget() override;

    //! load renderer from the layer
    void setLayer( QgsVectorLayer *layer );
    //! no transfer of ownership
    QgsRuleBased3DRenderer::Rule *rootRule() { return mRootRule; }

    void setDockMode( bool dockMode ) override;

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
    QgsRuleBased3DRenderer::Rule *currentRule();

  private:
    QgsVectorLayer *mLayer = nullptr;

    QgsRuleBased3DRenderer::Rule *mRootRule = nullptr;
    QgsRuleBased3DRendererModel *mModel = nullptr;

    QAction *mCopyAction = nullptr;
    QAction *mPasteAction = nullptr;
    QAction *mDeleteAction = nullptr;
};


//////

class QgsSymbol3DWidget;

#include "ui_qgs3drendererrulepropswidget.h"

class Qgs3DRendererRulePropsWidget : public QgsPanelWidget, private Ui::Qgs3DRendererRulePropsWidget
{
    Q_OBJECT

  public:
    enum Mode
    {
      Adding,
      Editing
    };

    Qgs3DRendererRulePropsWidget( QgsRuleBased3DRenderer::Rule *rule, QgsVectorLayer *layer, QWidget *parent = nullptr );
    ~Qgs3DRendererRulePropsWidget() override;

    QgsRuleBased3DRenderer::Rule *rule() { return mRule; }

  public slots:
    void testFilter();
    void buildExpression();

    /**
     * Apply any changes from the widget to the set rule.
     */
    void apply();

  protected:
    QgsRuleBased3DRenderer::Rule *mRule; // borrowed
    QgsVectorLayer *mLayer = nullptr;

    QgsSymbol3DWidget *mSymbolWidget = nullptr;
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol; // a clone of original symbol
};

#endif // QGSRULEBASED3DRENDERERWIDGET_H
