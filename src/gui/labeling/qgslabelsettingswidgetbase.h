/***************************************************************************
    qgslabelsettingswidgetbase.h
    ----------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELSETTINGSWIDGETBASE_H
#define QGSLABELSETTINGSWIDGETBASE_H

#include "qgssymbolwidgetcontext.h"
#include "qgspallabeling.h"
#include "qgspropertycollection.h"
#include "qgspanelwidget.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QDialog>

class QDialogButtonBox;
class QgsPropertyOverrideButton;

/**
 * \ingroup gui
 * \class QgsLabelSettingsWidgetBase
 * \brief Base class for widgets which allow customization of label engine properties, such as label placement settings.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLabelSettingsWidgetBase : public QgsPanelWidget, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelSettingsWidgetBase.
     * \param parent parent widget
     * \param vl associated vector layer
     */
    QgsLabelSettingsWidgetBase( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsVectorLayer *vl = nullptr );

    /**
     * Sets the \a context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see context()
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Sets the geometry \a type of the features to customize the widget accordingly.
     */
    virtual void setGeometryType( Qgis::GeometryType type );

    /**
     * Returns the current data defined properties state as specified in the widget.
     *
     * \see updateDataDefinedProperties()
     * \see setDataDefinedProperties()
     */
    QgsPropertyCollection dataDefinedProperties() const;

    /**
     * Sets the current data defined properties to show in the widget.
     *
     * \see dataDefinedProperties()
     */
    void setDataDefinedProperties( const QgsPropertyCollection &dataDefinedProperties );

    /**
     * Updates a data defined \a properties collection, correctly setting the values
     * for any properties related to this widget.
     *
     * \see setDataDefinedProperties()
     * \see dataDefinedProperties()
     */
    virtual void updateDataDefinedProperties( QgsPropertyCollection &properties );

  signals:

    /**
     * Emitted when any of the settings described by the widget are changed.
     */
    void changed();

    /**
     * Emitted when an auxiliary field is created in the widget.
     */
    void auxiliaryFieldCreated();

  protected:
    QgsExpressionContext createExpressionContext() const override;

    /**
     * Registers a data defined override \a button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsPalLayerSettings::Property key );

    /**
     * Contains the data defined properties defined by the widget.
     */
    QgsPropertyCollection mDataDefinedProperties;

  private slots:

    void createAuxiliaryField();
    void updateDataDefinedProperty();

  private:
    QgsVectorLayer *mVectorLayer = nullptr;

    QgsSymbolWidgetContext mContext;
};

/**
 * \ingroup gui
 * \class QgsLabelSettingsWidgetDialog
 * \brief A blocking dialog containing a QgsLabelSettingsWidgetBase.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLabelSettingsWidgetDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLabelSettingsWidgetDialog.
     * \param widget label settings widget to embed in the dialog. Ownership is transferred to the dialog.
     * \param parent parent widget
     */
    QgsLabelSettingsWidgetDialog( QgsLabelSettingsWidgetBase *widget SIP_TRANSFER, QWidget *parent = nullptr );


    /**
     * Returns the dialog's button box.
     *
     * \since QGIS 3.14
     */
    QDialogButtonBox *buttonBox();

  private:
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSLABELSETTINGSWIDGETBASE_H
