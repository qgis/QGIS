/***************************************************************************
    qgsattributewidgetedit.h
    ---------------------
    begin                : February 2020
    copyright            : (C) 2020 Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEWIDGETEDIT_H
#define QGSATTRIBUTEWIDGETEDIT_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "qgsattributesformmodel.h"
#include "ui_qgsattributewidgeteditgroupbox.h"
#include "ui_qgsattributewidgetrelationeditwidget.h"
#include "qgis_gui.h"

class QgsAbstractRelationEditorConfigWidget;

/**
 * Widget to edit the configuration (tab, group box, row, any field or relation, QML, …) of a form item
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAttributeWidgetEdit : public QgsCollapsibleGroupBox, private Ui_QgsAttributeWidgetEditGroupBox
{
    Q_OBJECT

  public:
    explicit QgsAttributeWidgetEdit( const QgsAttributesFormData::AttributeFormItemData &itemData, QWidget *parent = nullptr );

    /**
     * Updates the contents of the \a itemData object based on the widget status.
     *
     * \since QGIS 3.44
     */
    void updateItemData( QgsAttributesFormData::AttributeFormItemData &itemData ) const;

    /**
     * Returns an updated relation editor configuration based on the widget status.
     *
     * \since QGIS 3.44
     */
    QgsAttributesFormData::RelationEditorConfiguration updatedRelationConfiguration() const;

    // Methods to update widget status
    void setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle );
    void setShowLabel( bool showLabel );
    void setHorizontalStretch( const int horizontalStretch );
    void setVerticalStretch( const int verticalStretch );

    /**
     * Sets up the relation specific widget based on a \a configuration object and on a \a relationId.
     *
     * \since QGIS 3.44
     */
    void setRelationSpecificWidget( const QgsAttributesFormData::RelationEditorConfiguration &configuration, const QString &relationId );

  private:
    void showRelationButtons( bool show );

    QWidget *mSpecificEditWidget = nullptr;
};


/**
 * Widget to edit a relation specific config for a form item
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAttributeWidgetRelationEditWidget : public QWidget, private Ui_QgsAttributeWidgetRelationEditWidget
{
    Q_OBJECT

  public:
    explicit QgsAttributeWidgetRelationEditWidget( QWidget *parent = nullptr );

    void setRelationEditorConfiguration( const QgsAttributesFormData::RelationEditorConfiguration &config, const QString &relationId );

    QgsAttributesFormData::RelationEditorConfiguration relationEditorConfiguration() const;

    static QString title() { return tr( "Relation" ); }

  private slots:
    void relationCardinalityComboCurrentIndexChanged( int index );

  private:
    void setCardinalityCombo( const QString &cardinalityComboItem, const QVariant &auserData = QVariant() );
    void setNmRelationId( const QVariant &auserData = QVariant() );

    QMetaObject::Connection mWidgetTypeComboBoxConnection;
    QgsAbstractRelationEditorConfigWidget *mConfigWidget = nullptr;
};

#endif // QGSATTRIBUTEWIDGETEDIT_H
