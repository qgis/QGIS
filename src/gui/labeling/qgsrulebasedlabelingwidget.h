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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "qgspanelwidget.h"

#include "ui_qgsrulebasedlabelingwidget.h"

#include "qgsrulebasedlabeling.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsVectorLayer;


/**
 * \ingroup gui
 * \brief Model for rule based rendering rules view.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsRuleBasedLabelingModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    //! constructor
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

    //! Returns the rule at the specified index
    QgsRuleBasedLabeling::Rule *ruleForIndex( const QModelIndex &index ) const;

    //! Inserts a new rule at the specified position
    void insertRule( const QModelIndex &parent, int before, QgsRuleBasedLabeling::Rule *newrule );
    //! Updates the rule at the specified position
    void updateRule( const QModelIndex &parent, int row );
    //! Update rule and all its descendants
    void updateRule( const QModelIndex &index );
    //! Removes the rule at the specified position
    void removeRule( const QModelIndex &index );

    //! Notify the model that new rules will be added
    void willAddRules( const QModelIndex &parent, int count ); // call beginInsertRows

    /**
     * Notify the model that one is done inserting new rules
     * \see willAddRules()
     */
    void finishedAddingRules(); // call endInsertRows

  protected:
    QgsRuleBasedLabeling::Rule *mRootRule = nullptr;
};


class QgsLabelingRulePropsWidget;


/**
 * \ingroup gui
 * \brief Widget for configuring rule based labeling
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsRuleBasedLabelingWidget : public QgsPanelWidget, private Ui::QgsRuleBasedLabelingWidget
{
    Q_OBJECT
  public:
    //! constructor
    QgsRuleBasedLabelingWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );
    ~QgsRuleBasedLabelingWidget() override;

    //! Gives access to the internal root of the rule tree
    const QgsRuleBasedLabeling::Rule *rootRule() const { return mRootRule; }

    void setDockMode( bool dockMode ) override;

  private slots:
    void addRule();
    void editRule();
    void editRule( const QModelIndex &index );
    void removeRule();
    void copy();
    void paste();
    void ruleWidgetPanelAccepted( QgsPanelWidget *panel );
    void liveUpdateRuleFromPanel();

  private:
    QgsRuleBasedLabeling::Rule *currentRule();

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

#include <QDialog>
#include <QDialogButtonBox>

#include "ui_qgslabelingrulepropswidget.h"
#include "qgsgui.h"

/**
 * \ingroup gui
 * \brief Widget for editing a labeling rule
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLabelingRulePropsWidget : public QgsPanelWidget, private Ui::QgsLabelingRulePropsWidget
{
    Q_OBJECT

  public:
    //! constructor
    QgsLabelingRulePropsWidget( QgsRuleBasedLabeling::Rule *rule, QgsVectorLayer *layer,
                                QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr );
    ~QgsLabelingRulePropsWidget() override;

    //! Returns the rule being edited
    QgsRuleBasedLabeling::Rule *rule() { return mRule; }

    /**
     * Set the widget in dock mode.
     * \param dockMode TRUE for dock mode.
     */
    void setDockMode( bool dockMode ) override;

  public slots:
    //! Apply any changes from the widget to the set rule.
    void apply();

    /**
     * Test the filter that is set in the widget
     */
    void testFilter();

    /**
     * Open the expression builder widget
     */
    void buildExpression();

  private:
    QgsRuleBasedLabeling::Rule *mRule; // borrowed
    QgsVectorLayer *mLayer = nullptr;

    QgsLabelingGui *mLabelingGui = nullptr;
    QgsPalLayerSettings *mSettings; // a clone of original settings

    QgsMapCanvas *mMapCanvas = nullptr;
};

/**
 * \ingroup gui
 * \class QgsLabelingRulePropsDialog
 * \brief Dialog for editing labeling rule
 *
 * \note This class is not a part of public API
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsLabelingRulePropsDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLabelingRulePropsDialog
     * \param rule associated rule based labeling rule
     * \param layer source vector layer
     * \param parent parent widget
     * \param mapCanvas map canvas
     */
    QgsLabelingRulePropsDialog( QgsRuleBasedLabeling::Rule *rule, QgsVectorLayer *layer,
                                QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr );

    /**
     * Returns the current set rule.
     * \returns The current rule.
     */
    QgsRuleBasedLabeling::Rule *rule() { return mPropsWidget->rule(); }

  public slots:

    /**
     * Test the filter that is set in the widget
     */
    void testFilter();

    /**
     * Open the expression builder widget
     */
    void buildExpression();

    /**
     * Apply any changes from the widget to the set rule.
     */
    void accept() override;

  private slots:
    void showHelp();

  private:
    QgsLabelingRulePropsWidget *mPropsWidget = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
};

#endif // QGSRULEBASEDLABELINGWIDGET_H
