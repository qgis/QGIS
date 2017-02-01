/***************************************************************************
    qgspropertyassistantwidget.h
    ----------------------------
    begin                : February, 2017
    copyright            : (C) 2017 Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROPERTYASSISTANTWIDGET_H
#define QGSPROPERTYASSISTANTWIDGET_H

#include "qgspanelwidget.h"
#include "ui_qgspropertyassistantwidgetbase.h"
#include "ui_qgspropertysizeassistantwidget.h"
#include "qgsproperty.h"
#include "qgis_gui.h"

class QgsMapCanvas;

///@cond PRIVATE

class GUI_EXPORT QgsPropertyAbstractTransformerWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsPropertyAbstractTransformerWidget( QWidget* parent = nullptr, const QgsPropertyDefinition& definition = QgsPropertyDefinition() )
        : QWidget( parent )
        , mDefinition( definition )
    {}

    virtual QgsPropertyTransformer* createTransformer( double minValue, double maxValue ) const = 0;

  signals:

    void widgetChanged();

  protected:

    QgsPropertyDefinition mDefinition;

};

class GUI_EXPORT QgsPropertySizeAssistantWidget : public QgsPropertyAbstractTransformerWidget, private Ui::PropertySizeAssistant
{
    Q_OBJECT

  public:

    QgsPropertySizeAssistantWidget( QWidget* parent = nullptr, const QgsPropertyDefinition& definition = QgsPropertyDefinition(), const QgsProperty& initialState = QgsProperty() );

    virtual QgsPropertyTransformer* createTransformer( double minValue, double maxValue ) const override;


};

///@endcond PRIVATE

class GUI_EXPORT QgsPropertyAssistantWidget : public QgsPanelWidget, private Ui::PropertyAssistantBase
{
    Q_OBJECT

  public:

    QgsPropertyAssistantWidget( QWidget* parent = nullptr, const QgsPropertyDefinition& definition = QgsPropertyDefinition(),
                                const QgsProperty& initialState = QgsProperty(),
                                const QgsVectorLayer* layer = nullptr );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the button when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator* generator );

    void updateProperty( QgsProperty& property );


  private slots:
    void computeValuesFromLayer();

  private:

    QgsPropertyDefinition mDefinition;
    const QgsVectorLayer* mLayer = nullptr;
    QgsExpressionContextGenerator* mExpressionContextGenerator = nullptr;
    QgsPropertyAbstractTransformerWidget* mTransformerWidget = nullptr;

    bool computeValuesFromExpression( const QString& expression, double& minValue, double& maxValue ) const;
    bool computeValuesFromField( const QString& fieldName, double& minValue, double& maxValue ) const;

};


#endif // QGSPROPERTYASSISTANTWIDGET_H
