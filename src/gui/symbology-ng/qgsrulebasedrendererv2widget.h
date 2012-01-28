/***************************************************************************
    qgsrulebasedrendererv2widget.h - Settings widget for rule-based renderer
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRULEBASEDRENDERERV2WIDGET_H
#define QGSRULEBASEDRENDERERV2WIDGET_H

#include "qgsrendererv2widget.h"

#include "qgsrulebasedrendererv2.h"
class QMenu;

///////

#include <QAbstractItemModel>

/*
Tree model for the rules:

(invalid)  == root node
 +--- top level rule
 +--- top level rule
*/
class QgsRuleBasedRendererV2Model : public QAbstractItemModel
{
  public:
    QgsRuleBasedRendererV2Model( QgsRuleBasedRendererV2* r );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex & = QModelIndex() ) const;
    //! provide model index for parent's child item
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    //! provide parent model index
    virtual QModelIndex parent( const QModelIndex &index ) const;

    // editing support
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    // drag'n'drop support
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() );

    // new methods

    QgsRuleBasedRendererV2::Rule* ruleForIndex( const QModelIndex& index ) const;

    void insertRule( const QModelIndex& parent, int before, QgsRuleBasedRendererV2::Rule* newrule );
    void updateRule( const QModelIndex& parent, int row );
    void removeRule( const QModelIndex& index );

    void willAddRules( const QModelIndex& parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

  protected:
    QgsRuleBasedRendererV2* mR;
};


///////

#include "ui_qgsrulebasedrendererv2widget.h"

class GUI_EXPORT QgsRuleBasedRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsRuleBasedRendererV2Widget
{
    Q_OBJECT

  public:

    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsRuleBasedRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsRuleBasedRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

  public slots:

    void addRule();
    void editRule();
    void editRule( const QModelIndex& index );
    void removeRule();

    void refineRuleScales();
    void refineRuleCategories();
    void refineRuleRanges();

    void setRenderingOrder();

    void currentRuleChanged( const QModelIndex& current = QModelIndex(), const QModelIndex& previous = QModelIndex() );

  protected:

    void refineRule( int type );
    void refineRuleCategoriesGui( const QModelIndexList& index );
    void refineRuleRangesGui( const QModelIndexList& index );
    void refineRuleScalesGui( const QModelIndexList& index );

    QgsRuleBasedRendererV2::Rule* currentRule();

    QList<QgsSymbolV2*> selectedSymbols();
    void refreshSymbolView();

    QgsRuleBasedRendererV2* mRenderer;
    QgsRuleBasedRendererV2Model* mModel;

    QMenu* mRefineMenu;
};

///////

#include <QDialog>

#include "ui_qgsrendererrulepropsdialogbase.h"

class GUI_EXPORT QgsRendererRulePropsDialog : public QDialog, private Ui::QgsRendererRulePropsDialog
{
    Q_OBJECT

  public:
    QgsRendererRulePropsDialog( QgsRuleBasedRendererV2::Rule* rule, QgsVectorLayer* layer, QgsStyleV2* style );
    ~QgsRendererRulePropsDialog();

    QgsRuleBasedRendererV2::Rule* rule() { return mRule; }

  public slots:
    void testFilter();
    void buildExpression();
    void accept();

  protected:
    QgsRuleBasedRendererV2::Rule* mRule; // borrowed
    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;

    QgsSymbolV2SelectorDialog* mSymbolSelector;
    QgsSymbolV2* mSymbol; // a clone of original symbol
};


#endif // QGSRULEBASEDRENDERERV2WIDGET_H
