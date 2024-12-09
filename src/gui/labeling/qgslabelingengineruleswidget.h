/***************************************************************************
    qgslabelingengineruleswidget.h
    ------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELINGENGINERULESWIDGET_H
#define QGSLABELINGENGINERULESWIDGET_H

#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgsguiutils.h"
#include "ui_qgslabelingengineruleswidgetbase.h"

#include <QAbstractItemModel>
#include <QDialog>

class QgsAbstractLabelingEngineRule;
class QDialogButtonBox;

#ifndef SIP_RUN
/**
 * \ingroup gui
 * \class QgsLabelingEngineRulesModel
 * \brief A model for configuration of a list of labeling engine rules.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.40
 */
class QgsLabelingEngineRulesModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelingEngineRulesModel.
     */
    explicit QgsLabelingEngineRulesModel( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsLabelingEngineRulesModel() override;

    // QAbstractItemModel interface
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Sets the rules to include in the model.
     *
     * Ownership is not transferred, an internal copy will be made.
     */
    void setRules( const QList<QgsAbstractLabelingEngineRule *> &rules );

    /**
     * Adds a \a rule to the model.
     */
    void addRule( std::unique_ptr<QgsAbstractLabelingEngineRule> &rule );

    /**
     * Returns the rule at the specified model \a index.
     */
    QgsAbstractLabelingEngineRule *ruleAtIndex( const QModelIndex &index ) const;

    /**
     * Swaps the rule at the specified \a index for a new \a rule.
     */
    void changeRule( const QModelIndex &index, std::unique_ptr<QgsAbstractLabelingEngineRule> &rule );

    /**
     * Returns the rules shown in the widget.
     *
     * The caller takes ownership of all returned rules.
     */
    QList<QgsAbstractLabelingEngineRule *> rules() const;

  private:
    std::vector<std::unique_ptr<QgsAbstractLabelingEngineRule>> mRules;
};

#endif


/**
 * \ingroup gui
 * \class QgsLabelingEngineRulesWidget
 * \brief A widget which allows configuration of a list of labeling engine rules.
 *
 * This widget allows users to add, remove, and edit the properties of a
 * list of QgsAbstractLabelingEngineRule objects.
 *
 * \see QgsLabelingEngineRuleWidget for a widget for configuring a single rule
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsLabelingEngineRulesWidget : public QgsPanelWidget, private Ui_QgsLabelingEngineRulesWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelingEngineRulesWidget.
     */
    QgsLabelingEngineRulesWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a rules to show in the widget.
     *
     * Ownership is not transferred, an internal copy will be made.
     */
    void setRules( const QList<QgsAbstractLabelingEngineRule *> &rules );

    /**
     * Returns the rules shown in the widget.
     *
     * The caller takes ownership of all returned rules.
     */
    QList<QgsAbstractLabelingEngineRule *> rules() const SIP_TRANSFERBACK;

  signals:

    /**
     * Emitted when the rules configured in the widget are changed.
     */
    void changed();

  private slots:

    void createTypesMenu();
    void createRule( const QString &id );
    void editSelectedRule();
    void editRule( const QModelIndex &index );
    void removeRules();

  private:
    QgsLabelingEngineRulesModel *mModel = nullptr;
    QMenu *mAddRuleMenu = nullptr;
};


/**
 * \class QgsLabelingEngineRulesDialog
 * \ingroup gui
 * \brief A dialog which allows configuration of a list of labeling engine rules.
 *
 * This dialog allows users to add, remove, and edit the properties of a
 * list of QgsAbstractLabelingEngineRule objects.
 *
 * \see QgsLabelingEngineRulesWidget()
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsLabelingEngineRulesDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelingEngineRulesDialog.
     * \param parent parent widget
     * \param flags window flags for dialog
     */
    QgsLabelingEngineRulesDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Sets the \a rules to show in the dialog.
     *
     * Ownership is not transferred, an internal copy will be made.
     */
    void setRules( const QList<QgsAbstractLabelingEngineRule *> &rules );

    /**
     * Returns the rules shown in the dialog.
     *
     * The caller takes ownership of all returned rules.
     */
    QList<QgsAbstractLabelingEngineRule *> rules() const SIP_TRANSFERBACK;

  private:
    QgsLabelingEngineRulesWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};


#endif // QGSLABELINGENGINERULESWIDGET_H
