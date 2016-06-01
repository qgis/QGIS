/***************************************************************************
  qgslayertreeembeddedwidgetregistry.h
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H
#define QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H

#include <QMap>
#include <QWidget>

#if 0
/** Base class for embedded widgets */
class QgsLayerTreeEmbeddedWidget : public QWidget
{
  public:

};
#endif

class QgsMapLayer;

/** Provider interface to be implemented */
class GUI_EXPORT QgsLayerTreeEmbeddedWidgetProvider
{
  public:
    virtual ~QgsLayerTreeEmbeddedWidgetProvider() {}

    //! unique name of the provider (among other providers)
    virtual QString id() const = 0;

    //! factory to create widgets
    virtual QWidget* createWidget( QgsMapLayer* layer, QMap<QString, QString> properties ) = 0;

    //! whether it makes sense to use this widget for a particular layer
    virtual bool supportsLayer( QgsMapLayer* layer ) = 0;

};

/** Singleton registry */
class GUI_EXPORT QgsLayerTreeEmbeddedWidgetRegistry
{
  public:

    /** Means of accessing canonical single instance  */
    static QgsLayerTreeEmbeddedWidgetRegistry* instance();

    ~QgsLayerTreeEmbeddedWidgetRegistry();

    QStringList providers() const;

    /** Get provider object from the provider's ID */
    QgsLayerTreeEmbeddedWidgetProvider* provider( const QString& providerId ) const;

    /** Register a provider, takes ownership of the object.
     * Returns true on success, false if the provider is already registered. */
    bool addProvider( QgsLayerTreeEmbeddedWidgetProvider* provider );

    /** Unregister a provider, the provider object is deleted.
     * Returns true on success, false if the provider was not registered. */
    bool removeProvider( const QString& providerId );

  protected:
    QgsLayerTreeEmbeddedWidgetRegistry();


    //! storage of all the providers
    QMap<QString, QgsLayerTreeEmbeddedWidgetProvider*> mProviders;
};


////


// TMP
class QSlider;
class QgsMapLayer;
#include <QWidget>
class TransparencyWidget : public QWidget
{
    Q_OBJECT
  public:
    TransparencyWidget( QgsMapLayer* layer );

    virtual QSize sizeHint() const override;

  public slots:
    void sliderValueChanged( int value );
    void layerTrChanged();

  private:
    QgsMapLayer* mLayer;
    QSlider* mSlider;
};



#endif // QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H
