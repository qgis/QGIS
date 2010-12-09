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

#include <QTreeWidget>

class GUI_EXPORT QgsRendererRulesTreeWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    QgsRendererRulesTreeWidget( QWidget* parent = 0 );

    void setRenderer( QgsRuleBasedRendererV2* r );

    enum Grouping { NoGrouping, GroupingByScale, GroupingByFilter };

    void setGrouping( Grouping g );

    void populateRules();

  protected:
    void populateRulesNoGrouping();
    void populateRulesGroupByScale();
    void populateRulesGroupByFilter();

    QString formatScaleRange( int minDenom, int maxDenom );

    QgsRuleBasedRendererV2* mR;
    Grouping mGrouping;
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
    void removeRule();

    void setGrouping();

    void refineRuleScales();
    void refineRuleCategories();
    void refineRuleRanges();

  protected:

    void refineRule( int type );
    QList<QgsRuleBasedRendererV2::Rule> refineRuleCategoriesGui( QgsRuleBasedRendererV2::Rule& initialRule );
    QList<QgsRuleBasedRendererV2::Rule> refineRuleRangesGui( QgsRuleBasedRendererV2::Rule& initialRule );
    QList<QgsRuleBasedRendererV2::Rule> refineRuleScalesGui( QgsRuleBasedRendererV2::Rule& initialRule );

    QgsRuleBasedRendererV2* mRenderer;

    QMenu* mRefineMenu;
};

///////

#include <QDialog>

#include "ui_qgsrendererrulepropsdialogbase.h"

class GUI_EXPORT QgsRendererRulePropsDialog : public QDialog, private Ui::QgsRendererRulePropsDialog
{
    Q_OBJECT

  public:
    QgsRendererRulePropsDialog( const QgsRuleBasedRendererV2::Rule& rule, QgsVectorLayer* layer, QgsStyleV2* style );

    void updateRuleFromGui();
    const QgsRuleBasedRendererV2::Rule& rule() { return mRule; }

  public slots:
    void testFilter();
    void buildExpression();

  protected:
    QgsRuleBasedRendererV2::Rule mRule;
    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;
};


#endif // QGSRULEBASEDRENDERERV2WIDGET_H
