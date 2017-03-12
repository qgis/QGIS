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
    virtual QVariantMap config() = 0;
    virtual void setConfig( const QVariantMap& config ) = 0;
    int field();
    QgsVectorLayer* layer();
    QgsExpressionContext createExpressionContext() const;

  signals:
    void changed();

  protected:
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsWidgetWrapper::Property key );
    void updateDataDefinedButtons();
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );
};

