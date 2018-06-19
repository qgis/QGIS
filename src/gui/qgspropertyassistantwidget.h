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
#include "qgis_sip.h"
#include "ui_qgspropertyassistantwidgetbase.h"
#include "ui_qgspropertysizeassistantwidget.h"
#include "ui_qgspropertycolorassistantwidget.h"
#include "ui_qgspropertygenericnumericassistantwidget.h"
#include "qgsproperty.h"
#include "qgslayertreegroup.h"
#include "qgssymbol.h"
#include "qgslayertreemodellegendnode.h"
#include "qgis_gui.h"
#include <QStandardItemModel>
#include <QItemDelegate>

class QgsMapCanvas;

#ifndef SIP_RUN

///@cond PRIVATE

class GUI_EXPORT QgsPropertyAbstractTransformerWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsPropertyAbstractTransformerWidget( QWidget *parent = nullptr, const QgsPropertyDefinition &definition = QgsPropertyDefinition() )
      : QWidget( parent )
      , mDefinition( definition )
    {}

    virtual QgsPropertyTransformer *createTransformer( double minValue, double maxValue ) const = 0;

    virtual QList< QgsSymbolLegendNode * > generatePreviews( const QList<double> &breaks, QgsLayerTreeLayer *parent, const QgsSymbol *symbol, double minValue, double maxValue, QgsCurveTransform *curve ) const;

  signals:

    void widgetChanged();

  protected:

    QgsPropertyDefinition mDefinition;

};

class GUI_EXPORT QgsPropertyGenericNumericAssistantWidget : public QgsPropertyAbstractTransformerWidget, private Ui::PropertyGenericNumericAssistant
{
    Q_OBJECT

  public:

    QgsPropertyGenericNumericAssistantWidget( QWidget *parent = nullptr, const QgsPropertyDefinition &definition = QgsPropertyDefinition(), const QgsProperty &initialState = QgsProperty() );

    QgsGenericNumericTransformer *createTransformer( double minValue, double maxValue ) const override;

};

class GUI_EXPORT QgsPropertySizeAssistantWidget : public QgsPropertyAbstractTransformerWidget, private Ui::PropertySizeAssistant
{
    Q_OBJECT

  public:

    QgsPropertySizeAssistantWidget( QWidget *parent = nullptr, const QgsPropertyDefinition &definition = QgsPropertyDefinition(), const QgsProperty &initialState = QgsProperty() );

    QgsSizeScaleTransformer *createTransformer( double minValue, double maxValue ) const override;

    QList< QgsSymbolLegendNode * > generatePreviews( const QList<double> &breaks, QgsLayerTreeLayer *parent, const QgsSymbol *symbol, double minValue, double maxValue, QgsCurveTransform *curve ) const override;
};

class GUI_EXPORT QgsPropertyColorAssistantWidget : public QgsPropertyAbstractTransformerWidget, private Ui::PropertyColorAssistant
{
    Q_OBJECT

  public:

    QgsPropertyColorAssistantWidget( QWidget *parent = nullptr, const QgsPropertyDefinition &definition = QgsPropertyDefinition(), const QgsProperty &initialState = QgsProperty() );

    QgsColorRampTransformer *createTransformer( double minValue, double maxValue ) const override;

    QList< QgsSymbolLegendNode * > generatePreviews( const QList<double> &breaks, QgsLayerTreeLayer *parent, const QgsSymbol *symbol, double minValue, double maxValue, QgsCurveTransform *curve ) const override;
};

///@endcond PRIVATE

#endif


/**
 * \class QgsPropertyAssistantWidget
 * \ingroup gui
 * Shows a user-friendly assistant guiding users through the creation of QgsProperty overrides.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsPropertyAssistantWidget : public QgsPanelWidget, private Ui::PropertyAssistantBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsPropertyAssistantWidget. Aside from a \a parent widget, the constructor accepts a
     * corresponding property \a definition from which it customizes the displayed options (eg a color based
     * property definition will show an assistant to allow creation of color based properties).
     * The \a initialState dictates the initial state to show in the widget. A corresponding \a layer
     * can also be set to allow population of GUI widgets such as field selectors.
     */
    QgsPropertyAssistantWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                const QgsPropertyDefinition &definition = QgsPropertyDefinition(),
                                const QgsProperty &initialState = QgsProperty(),
                                const QgsVectorLayer *layer = nullptr );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the button when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    /**
     * Updates a \a property in place to corresponding to the current settings shown
     * in the widget.
     */
    void updateProperty( QgsProperty &property );

    /**
     * Sets a symbol which can be used for previews inside the widget. If not specified, default
     * created symbols will be used instead.
     * \note not available in Python bindings
     */
    void setSymbol( std::shared_ptr< QgsSymbol > symbol ) { mSymbol = symbol; updatePreview(); } SIP_SKIP

    void setDockMode( bool dockMode ) override;

  private slots:
    void computeValuesFromLayer();
    void updatePreview();


  private:

    QgsPropertyDefinition mDefinition;
    const QgsVectorLayer *mLayer = nullptr;
    QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
    QgsPropertyAbstractTransformerWidget *mTransformerWidget = nullptr;

    QgsLayerTreeLayer *mLayerTreeLayer = nullptr;
    QgsLayerTreeGroup mRoot;
    QStandardItemModel mPreviewList;

    std::shared_ptr< QgsSymbol > mSymbol;

    bool computeValuesFromExpression( const QString &expression, double &minValue, double &maxValue ) const;
    bool computeValuesFromField( const QString &fieldName, double &minValue, double &maxValue ) const;

};

#ifndef SIP_RUN

/// @cond PRIVATE
class QgsAssistantPreviewItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsAssistantPreviewItemDelegate( QStandardItemModel *model ) : mModel( model ) {}

    QSize sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override
    {
      QSize size = mModel->item( index.row() )->icon().actualSize( QSize( 512, 512 ) );
      size.rheight() += 6;
      return size;
    }

  private:
    QStandardItemModel *mModel = nullptr;

};

///@endcond

#endif

#endif // QGSPROPERTYASSISTANTWIDGET_H
