/***************************************************************************
                         qgsattributetypedialog.h  -  description
                             -------------------
    begin                : June 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf.kostej@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTETYPEDIALOG_H
#define QGSATTRIBUTETYPEDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsattributetypeedit.h"

#include "qgseditorconfigwidget.h"
#include "qgsfeature.h"
#include "qgsvectordataprovider.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QWidget;
class QStandardItem;

class GUI_EXPORT QgsAttributeTypeDialog : public QWidget, private Ui::QgsAttributeTypeDialog, QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsAttributeTypeDialog( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );
    ~QgsAttributeTypeDialog() override;

    /**
     * Setting page which is to be selected
     * \param index index of page which was selected
     */
    void setPage( int index );

    const QString editorWidgetType();

    const QString editorWidgetText();

    /**
     * Sets the \a type in the widget combobox. Widget config is remembered,
     * allowing users to switch between types without losing configs, unless
     * \a forceWidgetRefresh is passed as true.
     *
     * \param type Editor widget type to be set
     * \param forceWidgetRefresh Always sets the config, ensuring a widget refresh
     */
    void setEditorWidgetType( const QString &type, bool forceWidgetRefresh = false );

    const QVariantMap editorWidgetConfig();

    void setEditorWidgetConfig( const QVariantMap &config );

    /**
     * Setter for checkbox to label on top
     * \param bool onTop
     */
    void setLabelOnTop( bool onTop );

    /**
     * Getter for checkbox for label on top of field
     */
    bool labelOnTop() const;

    /**
     * Setter for checkbox to reuse last entered values for the field.
     *
     * \see reuseLastValues()
     * \since QGIS 3.20
     */
    void setReuseLastValues( bool reuse );

    /**
     * Getter for checkbox to reuse last entered values for the field.
     *
     * \see setReuseLastValues()
     * \since QGIS 3.20
     */
    bool reuseLastValues() const;

    /**
     * Setter for label alias
     */
    void setAlias( const QString &alias );

    /**
     * Getter for label alias
     */
    QString alias() const;

    /**
     * Sets the data defined properties to \a properties.
     * \since QGIS 3.14
     */
    void setDataDefinedProperties( const QgsPropertyCollection &properties );

    /**
     * Returns the data defined properties.
     * \since QGIS 3.14
     */
    QgsPropertyCollection dataDefinedProperties() const;

    /**
     * Setter for label comment
     */
    void setComment( const QString &comment );

    /**
     * Setter for checkbox for editable state of field
     */
    void setFieldEditable( bool editable );

    /**
     * Getter for checkbox for editable state of field
     */
    bool fieldEditable() const;

    /**
     * Sets any provider side constraints which may affect this field's behavior.
     */
    void setProviderConstraints( QgsFieldConstraints::Constraints constraints );

    /**
     * Setter for checkbox for not null
     */
    void setNotNull( bool notNull );

    /**
     * Getter for checkbox for not null
     */
    bool notNull() const;

    /**
     * Returns whether the not null constraint is set by the provider
     */
    bool notNullFromProvider() const;

    /**
     * Returns whether the unique constraint is set by the provider
     */
    bool uniqueFromProvider() const;

    /**
     * Sets whether the not null constraint is enforced.
     */
    void setNotNullEnforced( bool enforced );

    /**
     * Returns whether the not null constraint should be enforced.
     */
    bool notNullEnforced() const;

    /**
     * Setter for unique constraint checkbox
     */
    void setUnique( bool unique );

    /**
     * Getter for unique constraint checkbox state
     */
    bool unique() const;

    /**
     * Sets whether the not null constraint is enforced.
     */
    void setUniqueEnforced( bool enforced );

    /**
     * Returns whether the not null constraint should be enforced.
     */
    bool uniqueEnforced() const;

    /**
     * Setter for constraint expression description
     * \param desc the expression description
     */
    void setConstraintExpressionDescription( const QString &desc );

    /**
     * Getter for constraint expression description
     * \returns the expression description
     */
    QString constraintExpressionDescription();

    /**
     * Getter for the constraint expression
     */
    QString constraintExpression() const;

    /**
     * Setter for the constraint expression
     */
    void setConstraintExpression( const QString &str );

    /**
     * Sets whether the expression constraint is enforced.
     */
    void setConstraintExpressionEnforced( bool enforced );

    /**
     * Returns whether the expression constraint should be enforced.
     */
    bool constraintExpressionEnforced() const;

    /**
     * Returns the expression used for the field's default value, or
     * an empty string if no default value expression is set.
     */
    QString defaultValueExpression() const;

    /**
     * Sets the expression used for the field's default value
     */
    void setDefaultValueExpression( const QString &expression );

    /**
     * Returns the field id
     */
    int fieldIdx() const;

    QgsExpressionContext createExpressionContext() const override;

    bool applyDefaultValueOnUpdate() const;
    void setApplyDefaultValueOnUpdate( bool applyDefaultValueOnUpdate );

    /**
     * Returns the field's split policy.
     *
     * \see setSplitPolicy()
     *
     * \since QGIS 3.32
     */
    Qgis::FieldDomainSplitPolicy splitPolicy() const;

    /**
     * Sets the field's split policy.
     *
     * \see splitPolicy()
     *
     * \since QGIS 3.32
     */
    void setSplitPolicy( Qgis::FieldDomainSplitPolicy policy );

    /**
     * Returns the field's duplicate policy.
     *
     * \see setDuplicatePolicy()
     *
     * \since QGIS 3.38
     */
    Qgis::FieldDuplicatePolicy duplicatePolicy() const;

    /**
     * Sets the field's duplicate policy.
     *
     * \see duplicatePolicy()
     *
     * \since QGIS 3.38
     */
    void setDuplicatePolicy( Qgis::FieldDuplicatePolicy policy );

  private slots:

    /**
     * Slot to handle change of index in combobox to select correct page
     * \param index index of value in combobox
     */
    void onCurrentWidgetChanged( int index );

    void defaultExpressionChanged();

    void updateSplitPolicyLabel();

    void updateDuplicatePolicyLabel();

  private:
    QgsVectorLayer *mLayer = nullptr;
    int mFieldIdx;

    QVariantMap mWidgetConfig;

    //! Cached configuration dialog (lazy loaded)
    QMap<QString, QgsEditorConfigWidget *> mEditorConfigWidgets;

    QStandardItem *currentItem() const;

    QgsFeature mPreviewFeature;

    QgsPropertyCollection mDataDefinedProperties;
};

#endif
