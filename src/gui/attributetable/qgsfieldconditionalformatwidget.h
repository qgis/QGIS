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
#include "qgis.h"
#include <QStandardItemModel>
#include <QStandardItem>

#include "ui_qgsfieldconditionalformatwidget.h"
#include "qgsconditionalstyle.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsFieldConditionalFormatWidget
 * A widget for customizing conditional formatting options.
 * \since QGIS 2.12
 */
class GUI_EXPORT QgsFieldConditionalFormatWidget : public QWidget, private Ui::QgsFieldConditionalWidget
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFieldConditionalFormatWidget.
     * \param parent parent widget
     */
    explicit QgsFieldConditionalFormatWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Switches the widget to the rules page.
     */
    void viewRules();

    /**
     * Sets the vector layer associated with the widget.
     * \param layer vector layer
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Switches the widget to the edit style mode for the specified style.
     * \param index index of conditional style to edit
     * \param style initial conditional styling options
     */
    void editStyle( int index, const QgsConditionalStyle &style );

    /**
     * \param style initial conditional styling options
     */
    void loadStyle( const QgsConditionalStyle &style );

    /**
     * Resets the formatting options to their default state.
     */
    void reset();

    /**
     * \brief Set the presets that can be used for quick pick
     * \param styles A list of styles used as presets
     */
    void setPresets( const QList<QgsConditionalStyle> &styles );

    /**
     * \brief The default presets for the widget.  Normally set when the widget is
     * created however called setPresets will override the default styles.
     * \returns List of default presets.
     */
    QList<QgsConditionalStyle> defaultPresets() const;

  signals:

    /**
     * Emitted when the conditional styling rules are updated.
     * \param fieldName name of field whose rules have been modified.
     */
    void rulesUpdated( const QString &fieldName );

  public slots:

  private:
    QgsVectorLayer *mLayer = nullptr;
    int mEditIndex = 0;
    bool mEditing = false;
    QStandardItemModel *mModel = nullptr;
    QStandardItemModel *mPresetsModel = nullptr;
    QList<QgsConditionalStyle> mPresets;

    QList<QgsConditionalStyle> getStyles();

    void setFormattingFromStyle( const QgsConditionalStyle &style );

  private slots:
    void setExpression();
    void presetSet( int index );
    bool isCustomSet();
    void ruleClicked( const QModelIndex &index );
    void reloadStyles();
    void cancelRule();
    void deleteRule();
    void saveRule();
    void addNewRule();
    void fieldChanged( const QString &fieldName );

};

#endif // QGSFIELDCONDITIONALFORMATWIDGET_H
