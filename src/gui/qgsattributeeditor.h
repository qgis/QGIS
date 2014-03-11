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
#include <QGridLayout>

#include "qgsfeature.h"
#include "qgsrelationmanager.h"

class QObject;
class QWidget;
class QComboBox;
class QListWidget;

class QgsAttributeEditorContext;
class QgsAttributeEditorElement;
class QgsDualView;
class QgsRelationManager;
class QgsVectorLayer;

/* \brief create attribute widget for editing */
class GUI_EXPORT QgsAttributeEditor : public QObject
{
    Q_OBJECT

  public:
    QgsAttributeEditor( QObject* parent, QgsVectorLayer* vl = 0, int idx = -1 )
        : QObject( parent )
        , mLayer( vl )
        , mIdx( idx )
    {}
    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     * @param proxyWidgets an array of widgets, which will act as a value proxy if the same field is inserted multiple times
     *
     * @deprecated
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant &value, QMap<int, QWidget*>& proxyWidgets );

    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     *
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value );
    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     * @param context the context used for the created attribute editor
     *
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value, QgsAttributeEditorContext& context );

    /**
     * Creates a widget form a QgsAttributeEditorElement definition. Will recursively generate containers and widgets.
     * @param widgetDef The definition for the widget
     * @param parent The parent object
     * @param vl The vector layer to use as data source
     * @param feat The feature to create the widget for
     * @param context the context used for the created attribute editor
     * @param [out] labelText An optional label text will be written into the referenced QString. It will be set to
     *        a QString::null value if no label should be shown
     * @param [out] labelOnTop Will be set to true if the label should be placed on top of the field.
     *        If set to false, the label should be shown left or right of the field
     *
     */
    static QWidget *createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, const QgsFeature &feat, QgsAttributeEditorContext& context, QString& labelText, bool& labelOnTop );

    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );

  private:
    static QComboBox *comboBox( QWidget *editor, QWidget *parent );
    static QListWidget *listWidget( QWidget *editor, QWidget *parent );
    static QgsDualView* dualView( QWidget* editor, QWidget* parent );

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

class GUI_EXPORT QgsStringRelay : public QObject
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

Q_DECLARE_METATYPE( QgsStringRelay* )

#endif
