/***************************************************************************
    qgseditorwidgetwrapper.sip
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsEditorWidgetWrapper : QObject
{
%TypeHeaderCode
#include <qgseditorwidgetwrapper.h>
%End

  public:
    explicit QgsEditorWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent /TransferThis/ = 0 );
    QWidget* widget();
    virtual void setConfig( QMap<QString, QVariant> config );
    virtual QVariant value() = 0;
    QVariant config( QString key );

    QgsVectorLayer* layer();
    int field();

  protected:
    virtual QWidget* createWidget( QWidget* parent ) = 0 /Factory/;

    virtual void initWidget( QWidget* editor );

  signals:
    void valueChanged( const QVariant& value );

  public slots:
    virtual void setValue( const QVariant& value ) = 0;
    virtual void setEnabled( bool enabled );

};
