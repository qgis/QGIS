/***************************************************************************
    qgsnumericformatwidget.h
    ------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNUMERICFORMATWIDGET_H
#define QGSNUMERICFORMATWIDGET_H

#include "qgis_sip.h"
#include "qgsnumericformat.h"
#include "qgspanelwidget.h"
#include <QStandardItemModel>

/**
 * \ingroup gui
 * \class QgsNumericFormatWidget
 * Base class for widgets which allow control over the properties of QgsNumericFormat subclasses
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsNumericFormatWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNumericFormatWidget.
     */
    QgsNumericFormatWidget( QWidget *parent SIP_TRANSFERTHIS )
      : QgsPanelWidget( parent )
    {}

    /**
     * Sets the \a format to show in the widget. Ownership is not transferred.
     * \see format()
     */
    virtual void setFormat( QgsNumericFormat *format ) = 0;

    /**
     * Returns the format defined by the current settings in the widget.
     *
     * Ownership of the returned object is transferred to the caller
     *
     * \see setFormat()
     */
    virtual QgsNumericFormat *format() = 0 SIP_TRANSFERBACK;

  signals:

    /**
     * Emitted whenever the configuration of the numeric format is changed.
     */
    void changed();

};


#endif // QGSNUMERICFORMATWIDGET_H
