/***************************************************************************
                         qgsattributeeditor.h  -  description
                             -------------------
    begin                : July 2009
    copyright            : (C) 2009 by Jürgen E. Fischer
    email                : jef@norbit.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITOR_H
#define QGSATTRIBUTEEDITOR_H

#include <QVariant>
#include <QMetaType>

#include "qgsfeature.h"

class QObject;
class QWidget;
class QgsVectorLayer;
class QComboBox;
class QListWidget;
class QgsAttributeEditorElement;

/* \brief create attribute widget for editing */
class GUI_EXPORT QgsAttributeEditor : public QObject
{
    Q_OBJECT

  public:
    QgsAttributeEditor( QObject *parent, QgsVectorLayer *vl = 0, int idx = -1 )
        : QObject( parent )
        , mLayer( vl )
        , mIdx( idx )
    {};
    /**
     * Creates or prepares a attributre editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     * @param proxyWidgets an array of widgets, which will act as a value proxy if the same field is inserted multiple times
     *
     */
    static QWidget *createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value, QMap<int, QWidget*> &proxyWidgets );
    /**
     * Creates or prepares a attributre editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     *
     */
    static QWidget *createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value );
    /**
     * Creates a widget form a QgsAttributeEditorElement definition. Will recursively generate containers and widgets.
     * @param widgetDef The definition for the widget
     * @param parent The parent object
     * @param vl The vector layer to use as data source
     * @param attrs Attributes for the current feature.
     * @param proxyWidgets An array of widgets, which will act as a value proxy if the same field is inserted multiple times
     * @param createGroupBox If the element is a container, should a GroupBox be created to hold the children?
     *
     */
    static QWidget *createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, QgsAttributes &attrs, QMap<int, QWidget*> &proxyWidgets, bool createGroupBox );
    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );

  private:
    static QComboBox *comboBox( QWidget *editor, QWidget *parent );
    static QListWidget *listWidget( QWidget *editor, QWidget *parent );

  public slots:
    void selectFileName();
    void selectDate();
    void loadUrl( const QString & );
    void loadPixmap( const QString & );
    void updateUrl();
    void openUrl();
    void updateColor();

  private:
    QgsVectorLayer *mLayer;
    int mIdx;
};

class QgsStringRelay : public QObject
{
    Q_OBJECT

  public:

    QgsStringRelay( QObject* parent = NULL )
        : QObject( parent ) {}

    void appendProxy( QWidget* proxy ) { mProxyList << proxy; }

  public slots:
    void changeText();
    void changeText( QString str );

  signals:
    void textChanged( QString );

  private:
    QList<QWidget*> mProxyList;
};

#endif
