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
#include "qgslayertreegroup.h"
#include "qgssymbol.h"
#include "qgslayertreemodellegendnode.h"
#include "qgis_gui.h"
#include <QStandardItemModel>
#include <QItemDelegate>

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

    virtual QList< QgsSymbolLegendNode* > generatePreviews( const QList<double>& breaks, QgsLayerTreeLayer* parent, const QgsVectorLayer* layer, const QgsSymbol* symbol, double minValue, double maxValue ) const;

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

    virtual QgsSizeScaleTransformer* createTransformer( double minValue, double maxValue ) const override;

    QList< QgsSymbolLegendNode* > generatePreviews( const QList<double>& breaks, QgsLayerTreeLayer* parent, const QgsVectorLayer* layer, const QgsSymbol* symbol, double minValue, double maxValue ) const override;
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

    void setSymbol( std::shared_ptr< QgsSymbol > symbol ) { mSymbol = symbol; updatePreview(); }

  private slots:
    void computeValuesFromLayer();
    void updatePreview();


  private:

    QgsPropertyDefinition mDefinition;
    const QgsVectorLayer* mLayer = nullptr;
    QgsExpressionContextGenerator* mExpressionContextGenerator = nullptr;
    QgsPropertyAbstractTransformerWidget* mTransformerWidget = nullptr;

    QgsLayerTreeLayer* mLayerTreeLayer = nullptr;
    QgsLayerTreeGroup mRoot;
    QStandardItemModel mPreviewList;

    std::shared_ptr< QgsSymbol > mSymbol;

    bool computeValuesFromExpression( const QString& expression, double& minValue, double& maxValue ) const;
    bool computeValuesFromField( const QString& fieldName, double& minValue, double& maxValue ) const;

};


/// @cond PRIVATE
class ItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit ItemDelegate( QStandardItemModel* model ) : mModel( model ) {}

    QSize sizeHint( const QStyleOptionViewItem& /*option*/, const QModelIndex & index ) const override
    {
      return mModel->item( index.row() )->icon().actualSize( QSize( 512, 512 ) );
    }

  private:
    QStandardItemModel* mModel = nullptr;

};

///@endcond

#endif // QGSPROPERTYASSISTANTWIDGET_H
