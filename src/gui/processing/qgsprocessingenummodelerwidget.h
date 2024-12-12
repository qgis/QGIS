/***************************************************************************
                             qgsprocessingenummodelerwidget.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGENUMMODELERWIDGET_H
#define QGSPROCESSINGENUMMODELERWIDGET_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingenummodelerwidgetbase.h"
#include <QStandardItem>
#include <QStandardItemModel>

///@cond PRIVATE

/**
 * Processing enum widget for configuring enum parameter in modeler.
 * \ingroup gui
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingEnumModelerWidget : public QWidget, private Ui::QgsProcessingEnumModelerWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingEnumModelerWidget.
     */
    QgsProcessingEnumModelerWidget( QWidget *parent = nullptr );

    /**
    * Returns list of all available options.
    *
    * \see setOptions()
    */
    QStringList options() const;

    /**
    * Populate widget with available options.
    *
    * \see options()
    */
    void setOptions( const QStringList &options );

    /**
    * Returns indices of options used by default.
    *
    * \see setDefaultOptions()
    */
    QVariant defaultOptions() const;

    /**
    * Mark default options as checked.
    *
    * \see defaultOptions()
    */
    void setDefaultOptions( const QVariant &defaultValue );

    /**
     * Returns TRUE if the parameter allows multiple selected values.
     * \see setAllowMultiple()
     */
    bool allowMultiple() const;

    /**
     * Sets whether the parameter allows multiple selected values.
     * \see allowMultiple()
     */
    void setAllowMultiple( bool allowMultiple );

  private slots:

    void addItem();
    void removeItems( const bool removeAll );
    void onItemChanged( QStandardItem *item );

  private:
    QStandardItemModel *mModel = nullptr;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGENUMMODELERWIDGET_H
