/***************************************************************************
    qgsfieldconditionalformatwidget.h
    ---------------------
    begin                : August 2015
    copyright            : (C) 2015 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDCONDITIONALFORMATWIDGET_H
#define QGSFIELDCONDITIONALFORMATWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include "qgspanelwidget.h"
#include "qgspanelwidgetstack.h"

#include "ui_qgsfieldconditionalformatwidget.h"
#include "ui_qgseditconditionalformatrulewidget.h"
#include "qgsconditionalstyle.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsFieldConditionalFormatWidget
 * A widget for customizing conditional formatting options.
 * \since QGIS 2.12
 */
class GUI_EXPORT QgsFieldConditionalFormatWidget : public QgsPanelWidget, private Ui::QgsFieldConditionalWidget
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFieldConditionalFormatWidget.
     */
    explicit QgsFieldConditionalFormatWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Switches the widget to the rules page.
     *
     * \deprecated no longer used, will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void viewRules() SIP_DEPRECATED;

    /**
     * Sets the vector \a layer associated with the widget.
    */
    void setLayer( QgsVectorLayer *layer );

    // TODO QGIS 4.0 - make private

    /**
     * Switches the widget to the edit style mode for the specified style,
     * where \a index is the index of the conditional style to edit
     * and \a style is the initial definition of the style.
     */
    void editStyle( int index, const QgsConditionalStyle &style );

    /**
     * \deprecated no longer used, use QgsEditConditionalFormatRuleWidget::loadStyle instead.
     */
    Q_DECL_DEPRECATED void loadStyle( const QgsConditionalStyle &style ) SIP_DEPRECATED;

    /**
     * Resets the formatting options to their default state.
     *
     * \deprecated no longer used, will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void reset() SIP_DEPRECATED;

    /**
     * Sets the preset \a styles that can be used for quick pick.
     */
    void setPresets( const QList<QgsConditionalStyle> &styles );

    /**
     * Returns a list of the default presets. Normally used when the widget is
     * created, however calling setPresets() will override the default styles.
     */
    static QList<QgsConditionalStyle> defaultPresets();

  signals:

    /**
     * Emitted when the conditional styling rules are updated.
     *
     * The \a fieldName argument indicates the name of the field whose rules
     * have been modified, or an empty \a fieldName indicates that a row-based
     * rule was updated.
     */
    void rulesUpdated( const QString &fieldName );

  private:
    QgsVectorLayer *mLayer = nullptr;
    int mEditIndex = -1;
    bool mEditing = false;
    bool mPanelHandled = false;
    QStandardItemModel *mModel = nullptr;

    QList<QgsConditionalStyle> mPresets;

    QList<QgsConditionalStyle> getStyles();

  private slots:

    void ruleClicked( const QModelIndex &index );
    void reloadStyles();
    void addNewRule();
    void fieldChanged( const QString &fieldName );
    void deleteCurrentRule();

};

/**
 * \ingroup gui
 * \class QgsEditConditionalFormatRuleWidget
 * A widget for customizing an individual conditional formatting rule.
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsEditConditionalFormatRuleWidget : public QgsPanelWidget, private Ui::QgsEditConditionalRuleWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFieldConditionalFormatWidget, with the specified \a parent widget.
     */
    explicit QgsEditConditionalFormatRuleWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the vector \a layer associated with the widget.
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Sets the preset \a styles that can be used for quick pick.
     */
    void setPresets( const QList<QgsConditionalStyle> &styles );

    /**
     * Sets the widget to match the settings from the specified \a style.
     *
     * \see currentStyle()
     */
    void loadStyle( const QgsConditionalStyle &style );

    /**
     * Returns the current style defined by the widget.
     *
     * \see loadStyle()
     */
    QgsConditionalStyle currentStyle() const;

    /**
     * Sets the current expression \a rule to show in the widget.
     */
    void setRule( const QString &rule );

  signals:

    /**
     * Emitted when a user has opted to save the current rule.
     */
    void ruleSaved();

    /**
     * Emitted when a user has opted to deleted the current rule.
     */
    void ruleDeleted();

    /**
     * Emitted when a user has opted to cancel the rule modification.
     */
    void canceled();

  private:
    QgsVectorLayer *mLayer = nullptr;
    int mEditIndex = 0;
    bool mEditing = false;
    QStandardItemModel *mModel = nullptr;
    QStandardItemModel *mPresetsModel = nullptr;
    QList<QgsConditionalStyle> mPresets;

    void setFormattingFromStyle( const QgsConditionalStyle &style );

  private slots:
    void setExpression();
    void presetSet( int index );
    bool isCustomSet();

};

#endif // QGSFIELDCONDITIONALFORMATWIDGET_H
