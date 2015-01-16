/***************************************************************************
    qgsrulebasedrendererv2widget.h - Settings widget for rule-based renderer
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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

/* Features count fro rule */
struct QgsRuleBasedRendererV2Count
{
  int count; // number of features
  int duplicateCount; // number of features present also in other rule(s)
  // map of feature counts in other rules
  QMap<QgsRuleBasedRendererV2::Rule*, int> duplicateCountMap;
};

/*
Tree model for the rules:

(invalid)  == root node
 +--- top level rule
 +--- top level rule
*/
class GUI_EXPORT QgsRuleBasedRendererV2Model : public QAbstractItemModel
{
  public:
    QgsRuleBasedRendererV2Model( QgsRuleBasedRendererV2* r );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const override;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const override;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    virtual int columnCount( const QModelIndex & = QModelIndex() ) const override;
    //! provide model index for parent's child item
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    //! provide parent model index
    virtual QModelIndex parent( const QModelIndex &index ) const override;

    // editing support
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;

    // drag'n'drop support
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() ) override;

    // new methods

    QgsRuleBasedRendererV2::Rule* ruleForIndex( const QModelIndex& index ) const;

    void insertRule( const QModelIndex& parent, int before, QgsRuleBasedRendererV2::Rule* newrule );
    void updateRule( const QModelIndex& parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex& index );
    void removeRule( const QModelIndex& index );

    void willAddRules( const QModelIndex& parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

    //! @note not available in python bindungs
    void setFeatureCounts( QMap<QgsRuleBasedRendererV2::Rule*, QgsRuleBasedRendererV2Count> theCountMap );
    void clearFeatureCounts();

  protected:
    QgsRuleBasedRendererV2* mR;
    QMap<QgsRuleBasedRendererV2::Rule*, QgsRuleBasedRendererV2Count> mFeatureCountMap;
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

    virtual QgsFeatureRendererV2* renderer() override;

  public slots:

    void addRule();
    void editRule();
    void editRule( const QModelIndex& index );
    void removeRule();
    void countFeatures();
    void clearFeatureCounts() { mModel->clearFeatureCounts(); }

    void refineRuleScales();
    void refineRuleCategories();
    void refineRuleRanges();

    void setRenderingOrder();

    void currentRuleChanged( const QModelIndex& current = QModelIndex(), const QModelIndex& previous = QModelIndex() );

    void saveSectionWidth( int section, int oldSize, int newSize );
    void restoreSectionWidths();

  protected:
    void refineRule( int type );
    void refineRuleCategoriesGui( const QModelIndexList& index );
    void refineRuleRangesGui( const QModelIndexList& index );
    void refineRuleScalesGui( const QModelIndexList& index );

    QgsRuleBasedRendererV2::Rule* currentRule();

    QList<QgsSymbolV2*> selectedSymbols() override;
    QgsRuleBasedRendererV2::RuleList selectedRules();
    void refreshSymbolView() override;
    void keyPressEvent( QKeyEvent* event ) override;

    QgsRuleBasedRendererV2* mRenderer;
    QgsRuleBasedRendererV2Model* mModel;

    QMenu* mRefineMenu;
    QAction* mDeleteAction;

    QgsRuleBasedRendererV2::RuleList mCopyBuffer;

  protected slots:
    void copy() override;
    void paste() override;
};

///////

#include <QDialog>

#include "ui_qgsrendererrulepropsdialogbase.h"

class GUI_EXPORT QgsRendererRulePropsDialog : public QDialog, private Ui::QgsRendererRulePropsDialog
{
    Q_OBJECT

  public:
    QgsRendererRulePropsDialog( QgsRuleBasedRendererV2::Rule* rule, QgsVectorLayer* layer, QgsStyleV2* style, QWidget* parent = 0 );
    ~QgsRendererRulePropsDialog();

    QgsRuleBasedRendererV2::Rule* rule() { return mRule; }

  public slots:
    void testFilter();
    void buildExpression();
    void accept() override;

  protected:
    QgsRuleBasedRendererV2::Rule* mRule; // borrowed
    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;

    QgsSymbolV2SelectorDialog* mSymbolSelector;
    QgsSymbolV2* mSymbol; // a clone of original symbol
};


#endif // QGSRULEBASEDRENDERERV2WIDGET_H
