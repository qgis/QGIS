/***************************************************************************
    qgseditorwidgetregistry.h
     --------------------------------------
    Date                 : 24.4.2013
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

#ifndef QGSEDITORWIDGETREGISTRY_H
#define QGSEDITORWIDGETREGISTRY_H

#include <QObject>
#include <QMap>

#include "qgseditorwidgetfactory.h"

class QgsMapLayer;
class QDomNode;

/**
 * This class manages all known edit widget factories
 */
class GUI_EXPORT QgsEditorWidgetRegistry : public QObject
{
    Q_OBJECT

  public:
    /**
     * This class is a singleton and has therefore to be accessed with this method instead
     * of a constructor.
     *
     * @return
     */
    static QgsEditorWidgetRegistry* instance();
    ~QgsEditorWidgetRegistry();

    /**
     * Create an attribute editor widget wrapper of a given type for a given field.
     * The editor may be NULL if you want the widget wrapper to create a default widget.
     *
     * @param widgetId  The id of the widget type to create an attribute editor for
     * @param vl        The vector layer for which this widget will be created
     * @param fieldIdx  The field index on the specified layer for which this widget will be created
     * @param config    A configuration which should be used for the widget creation
     * @param editor    An editor widget which will be used instead of an autocreated widget
     * @param parent    The parent which will be used for the created wrapper and the created widget
     *
     * @return A new widget wrapper
     */
    QgsEditorWidgetWrapper* create( const QString& widgetId, QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, QWidget* editor, QWidget* parent );

    /**
     * Creates a configuration widget
     *
     * @param widgetId  The id of the widget type to create a configuration widget for
     * @param vl        The vector layer for which this widget will be created
     * @param fieldIdx  The field index on the specified layer for which this widget will be created
     * @param parent    The parent widget for the created widget
     *
     * @return A new configuration widget
     */
    QgsEditorConfigWidget* createConfigWidget( const QString& widgetId, QgsVectorLayer* vl, int fieldIdx, QWidget* parent );

    /**
     * Get the human readable name for a widget type
     *
     * @param widgetId The widget type to get the name for
     *
     * @return A human readable name
     */
    QString name( const QString& widgetId );

    /**
     * Get access to all registered factories
     *
     * @return All ids and factories
     */
    const QMap<QString, QgsEditorWidgetFactory*> factories();

    /**
     * The other part which does the boring work for you
     */
    template <class W, class C>
    void registerWidget( const QString& widgetType, const QString& name )
    {
      mWidgetFactories.insert( widgetType, new QgsEditWidgetFactoryHelper<W, C>( name ) );
    }

    /**
     * Register a new widget factory with the given id
     *
     * @param widgetId      The id which will be used later to refer to this widget type
     * @param widgetFactory The factory which will create this widget type
     *
     * @return true, if successful, false, if the widgetId is already in use or widgetFactory is NULL
     */
    bool registerWidget( const QString& widgetId, QgsEditorWidgetFactory* widgetFactory );

  protected:
    QgsEditorWidgetRegistry();

  private slots:
    void readMapLayer( QgsMapLayer* mapLayer , const QDomElement& layerElem );
    void writeMapLayer( QgsMapLayer* mapLayer , QDomElement& layerElem, QDomDocument& doc );

  private:
    QMap<QString, QgsEditorWidgetFactory*> mWidgetFactories;
};


#endif // QGSEDITORWIDGETREGISTRY_H
