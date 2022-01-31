/***************************************************************************
    qgsrulebasedrendererwidget.h - Settings widget for rule-based renderer
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

#ifndef QGSRULEBASEDRENDERERWIDGET_H
#define QGSRULEBASEDRENDERERWIDGET_H

#include "qgsrendererwidget.h"
#include "qgis_sip.h"

#include "qgsrulebasedrenderer.h"
class QMenu;
class QgsSymbolSelectorWidget;

///////

#include <QAbstractItemModel>

/* Features count for rule */
struct QgsRuleBasedRendererCount SIP_SKIP
{
  int count; // number of features
  int duplicateCount; // number of features present also in other rule(s)
  // map of feature counts in other rules
  QHash<QgsRuleBasedRenderer::Rule *, int> duplicateCountMap;
};

/**
 * \ingroup gui
 * \brief Tree model for the rules:
 *
 * (invalid)  == root node
 * +--- top level rule
 * +--- top level rule
 */
class GUI_EXPORT QgsRuleBasedRendererModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRuleBasedRendererModel, for the specified \a renderer.
     */
    QgsRuleBasedRendererModel( QgsRuleBasedRenderer *renderer, QObject *parent );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
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

    QgsRuleBasedRenderer::Rule *ruleForIndex( const QModelIndex &index ) const;

    void insertRule( const QModelIndex &parent, int before, QgsRuleBasedRenderer::Rule *newrule );
    void updateRule( const QModelIndex &parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex &index );
    void removeRule( const QModelIndex &index );

    /**
     * Sets the \a symbol for the rule at the specified \a index. Ownership of the symbols is
     * transferred to the renderer.
     *
     * \since QGIS 3.10
     */
    void setSymbol( const QModelIndex &index, QgsSymbol *symbol SIP_TRANSFER );

    void willAddRules( const QModelIndex &parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

    //! \note not available in Python bindungs
    void setFeatureCounts( const QHash<QgsRuleBasedRenderer::Rule *, QgsRuleBasedRendererCount> &countMap ) SIP_SKIP;
    void clearFeatureCounts();

  signals:
    /**
     * Signals emitted when a modified key is held and the state is toggled.
     * 
     * \since QGIS 3.28
     */
    void toggleSelectedSymbols( const bool state );

  protected:
    QgsRuleBasedRenderer *mR = nullptr;
    QHash<QgsRuleBasedRenderer::Rule *, QgsRuleBasedRendererCount> mFeatureCountMap;
};


///////

#include "ui_qgsrulebasedrendererwidget.h"

/**
 * \ingroup gui
 * \class QgsRuleBasedRendererWidget
 */
class GUI_EXPORT QgsRuleBasedRendererWidget : public QgsRendererWidget, private Ui::QgsRuleBasedRendererWidget
{
    Q_OBJECT

  public:

    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    QgsRuleBasedRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsRuleBasedRendererWidget() override;

    QgsFeatureRenderer *renderer() override;
    void setDockMode( bool dockMode ) override;

  public slots:

    void addRule();
    void editRule();
    void editRule( const QModelIndex &index );
    void removeRule();
    void countFeatures();
    void clearFeatureCounts() { mModel->clearFeatureCounts(); }

    void refineRuleScales();
    void refineRuleCategories();
    void refineRuleRanges();

    void setRenderingOrder();

    void currentRuleChanged( const QModelIndex &current = QModelIndex(), const QModelIndex &previous = QModelIndex() );
    void selectedRulesChanged();

    void saveSectionWidth( int section, int oldSize, int newSize );
    void restoreSectionWidths();

  protected:
    void refineRule( int type );
    //! Opens the dialog for refining a rule using categories
    void refineRuleCategoriesGui();
    //! Opens the dialog for refining a rule using ranges
    void refineRuleRangesGui();
    void refineRuleScalesGui( const QModelIndexList &index );

    void setSymbolLevels( const QList< QgsLegendSymbolItem > &levels, bool enabled ) override;

    QgsRuleBasedRenderer::Rule *currentRule();

    QList<QgsSymbol *> selectedSymbols() override;
    QgsRuleBasedRenderer::RuleList selectedRules();
    void refreshSymbolView() override;
    void keyPressEvent( QKeyEvent *event ) override;

    std::unique_ptr< QgsRuleBasedRenderer > mRenderer;
    QgsRuleBasedRendererModel *mModel = nullptr;

    QMenu *mRefineMenu = nullptr;
    QAction *mDeleteAction = nullptr;

    QgsRuleBasedRenderer::RuleList mCopyBuffer;
    QMenu *mContextMenu = nullptr;

  protected slots:
    void copy() override;
    void paste() override;
    void pasteSymbolToSelection() override;

  private slots:
    void refineRuleCategoriesAccepted( QgsPanelWidget *panel );
    void refineRuleRangesAccepted( QgsPanelWidget *panel );
    void ruleWidgetPanelAccepted( QgsPanelWidget *panel );
    void liveUpdateRuleFromPanel();
    void showContextMenu( QPoint p );
    /**
     * Slot used to change the state of all selected items.
     * 
     * \since QGIS 3.28
     */
    void toggleSelectedSymbols( const bool state );
};

///////

#include <QDialog>

#include "ui_qgsrendererrulepropsdialogbase.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsRendererRulePropsWidget
 */
class GUI_EXPORT QgsRendererRulePropsWidget : public QgsPanelWidget, private Ui::QgsRendererRulePropsWidget
{
    Q_OBJECT

  public:

    /**
       * Widget to edit the details of a rule based renderer rule.
       * \param rule The rule to edit.
       * \param layer The layer used to pull layer related information.
       * \param style The active QGIS style.
       * \param parent The parent widget.
       * \param context the symbol widget context
       */
    QgsRendererRulePropsWidget( QgsRuleBasedRenderer::Rule *rule,
                                QgsVectorLayer *layer,
                                QgsStyle *style,
                                QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                const QgsSymbolWidgetContext &context = QgsSymbolWidgetContext() );

    /**
     * Returns the current set rule.
     * \returns The current rule.
     */
    QgsRuleBasedRenderer::Rule *rule() { return mRule; }

  public slots:

    /**
     * Test the filter that is set in the widget
     */
    void testFilter();

    /**
     * Open the expression builder widget to check if the
     */
    void buildExpression();

    /**
     * Apply any changes from the widget to the set rule.
     */
    void apply();

    /**
     * Set the widget in dock mode.
     * \param dockMode TRUE for dock mode.
     */
    void setDockMode( bool dockMode ) override;

  protected:
    QgsRuleBasedRenderer::Rule *mRule; // borrowed
    QgsVectorLayer *mLayer = nullptr;

    QgsSymbolSelectorWidget *mSymbolSelector = nullptr;
    QgsSymbol *mSymbol = nullptr; // a clone of original symbol

    QgsSymbolWidgetContext mContext;
};

/**
 * \ingroup gui
 * \class QgsRendererRulePropsDialog
 */
class GUI_EXPORT QgsRendererRulePropsDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRendererRulePropsDialog
     * \param rule associated rule based renderer rule
     * \param layer source vector layer
     * \param style style collection
     * \param parent parent widget
     * \param context symbol widget context
     */
    QgsRendererRulePropsDialog( QgsRuleBasedRenderer::Rule *rule, QgsVectorLayer *layer, QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr, const QgsSymbolWidgetContext &context = QgsSymbolWidgetContext() );

    QgsRuleBasedRenderer::Rule *rule() { return mPropsWidget->rule(); }

  public slots:
    void testFilter();
    void buildExpression();
    void accept() override;

  private slots:
    void showHelp();

  private:
    QgsRendererRulePropsWidget *mPropsWidget = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
};


#endif // QGSRULEBASEDRENDERERWIDGET_H
