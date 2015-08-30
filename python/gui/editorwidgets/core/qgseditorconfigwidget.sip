/***************************************************************************
    qgseditorconfigwidget.sip
     --------------------------------------
    Date                 : 24.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsEditorConfigWidget : QWidget
{
%TypeHeaderCode
#include <qgseditorconfigwidget.h>
%End

  public:
    explicit QgsEditorConfigWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent /TransferThis/ );

    /**
     * Returns the field for which this configuration widget applies
     *
     * @return The field index
     */
    int field();

    /**
     * Returns the layer for which this configuration widget applies
     *
     * @return The layer
     */
    QgsVectorLayer* layer();

    virtual QgsEditorWidgetConfig config() = 0;
    virtual void setConfig( const QgsEditorWidgetConfig& config ) = 0;
};

