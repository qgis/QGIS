/***************************************************************************
                             qgsfieldchooserwidget.cpp
                             -------------------------
    begin                : September 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDCHOOSERWIDGET_H
#define QGSFIELDCHOOSERWIDGET_H

#include "qgslayerchooserwidget.h"
#include "qgsvectorlayer.h"

#include <QObject>

/**
  * Manage a widget to allow selecting a field
  * for a given vector layer.
  * Different types of widgets are managed using
  * inherited classes: QgsFieldChooserCombo
  *
  * It can be used either using predefined layer or
  * by associating a QgsLayerChooserWidget.
  *
  * A QgsFieldChooserWidget::FieldFilter can be used
  * to filter the fields.
  */
class GUI_EXPORT QgsFieldChooserWidget : public QObject
{
    Q_OBJECT

  public:
    /**
     * @brief The visibility of the field in the widget depending on the result of the QgsFieldChooserWidget::FieldFilter
     */
    enum DisplayStatus
    {
      enabled,
      disabled,
      hidden
    };

    /**
     * Filter the fields to list in the widget.
     * @see QgsFieldChooserWidget
     */
    class FieldFilter
    {
      public:
        /**
         * @brief define if the field should be listed or not.
         * @param idx the field index
         * @return a QgsFieldChooserWidget::DisplayStatus (enabled, disabled or hidden)
         */
        virtual DisplayStatus acceptField( int idx ) { Q_UNUSED( idx ); return enabled; }
    };

    /**
     * @brief constructs a field chooser widget depending on a layer chooser widget
     * @param layerChooser the layer chooser widget
     * @param parent the parent object
     */
    QgsFieldChooserWidget( QgsLayerChooserWidget* layerChooser, QObject *parent = 0 );

    /**
     * @brief constructs a field chooser widget without a layer chooser widget. setLayer can be called manually.
     * @param parent the parent object
     */
    QgsFieldChooserWidget( QObject *parent = 0 );

    /**
     * @brief initWidget is a pure virtual method to initialize the widget
     * @return true in case of success
     */
    virtual bool initWidget( QWidget* ) = 0;

    /**
     * @brief set the filter to be used to determine layers visibility
     * @param filter
     */
    void setFilter( FieldFilter* filter );

    /**
     * @brief getFieldIndex is a pure virtual method
     * @return return the currently choosen field index
     */
    virtual int getFieldIndex() = 0;

    /**
     * @brief getFieldName is a pure virtual method
     * @return return the currently choosen field name
     */
    virtual QString getFieldName() = 0;

    /** set the currently select field */
    virtual void setField( QString fieldName ) = 0;

  signals:
    /**
     * @brief fieldChanged is emitted whenever the selected field changes
     */
    void fieldChanged( int );

  public slots:
    /** setLayer should be manually called only if the chooser depends on a vector layer and not on a QgsLayerChooserWidget */
    void setLayer( QgsMapLayer*layer );

  protected:
    virtual void clearWidget() = 0;
    virtual void addField( QString fieldAlias, QString fieldName, DisplayStatus display ) = 0;
    virtual void unselect() = 0;
    QgsLayerChooserWidget* mLayerChooser;
    FieldFilter* mFilter;
    QgsVectorLayer* mLayer;

  protected slots:
    void layerChanged();
    void layerDeleted();
};

#endif // QGSFIELDCHOOSERWIDGET_H
