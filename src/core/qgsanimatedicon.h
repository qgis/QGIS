/***************************************************************************
  qgsanimatedicon.h - QgsAnimatedIcon

 ---------------------
 begin                : 13.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSANIMATEDICON_H
#define QGSANIMATEDICON_H

#include <QObject>
#include <QMovie>
#include <QIcon>
#include <QMetaMethod>

#include "qgis_core.h"

/**
 * \ingroup core
 * Animated icon is keeping an animation running if there are listeners connected to frameChanged
*/
class CORE_EXPORT QgsAnimatedIcon : public QObject
{
    Q_OBJECT
  public:

    /**
     * Create a new animated icon. Optionally, the \a iconPath can already be specified.
     */
    QgsAnimatedIcon( const QString &iconPath = QString(), QObject *parent = nullptr );

    /**
     * Path to a movie, e.g. animated GIF
     */
    QString iconPath() const;

    /**
     * Path to a movie, e.g. animated GIF
     */
    void setIconPath( const QString &iconPath );

    /**
     * Gets the icons representation in the current frame.
     * This will need to be called repeatedly, whenever a frameChanged()
     * signal is emitted.
     */
    QIcon icon() const;

#ifndef SIP_RUN

    /**
     * Connect a slot that will be notified repeatedly whenever a frame changes and which should
     * request the current icon and trigger UI updates.
     *
     * Connect to the frame changed signal with this method and not directly. This method
     * makes sure the annimation is started.
     *
     * \note Available in Python bindings as
     *       bool connectFrameChanged( const QObject *receiver, const char *method );.
     * \since QGIS 3.0
     */
    template <typename Func1>
    bool connectFrameChanged( const typename QtPrivate::FunctionPointer<Func1>::Object *receiver, Func1 slot )
    {
      if ( connect( this, &QgsAnimatedIcon::frameChanged, receiver, slot ) )
      {
        mMovie->setPaused( false );
        return true;
      }
      else
        return false;
    }

    /**
     * Convenience function to disconnect the same style that the frame change connection was established.
     *
     * \note Available in Python bindings as
     *       bool disconnectFrameChanged( const QObject *receiver, const char *method );.
     * \since QGIS 3.0
     */
    template <typename Func1>
    bool disconnectFrameChanged( const typename QtPrivate::FunctionPointer<Func1>::Object *receiver, Func1 slot )
    {
      return disconnect( this, &QgsAnimatedIcon::frameChanged, receiver, slot );
    }

#endif

    /**
     * Connect a slot that will be notified repeatedly whenever a frame changes and which should
     * request the current icon and trigger UI updates.
     *
     * Connect to the frame changed signal with this method and not directly. This method
     * makes sure the annimation is started.
     *
     * \since QGIS 3.0
     */
    bool connectFrameChanged( const QObject *receiver, const char *method );

    /**
     * Convenience function to disconnect the same style that the frame change connection was established.
     *
     * \since QGIS 3.0
     */
    bool disconnectFrameChanged( const QObject *receiver, const char *method );


    /**
     * The native width of the icon.
     *
     * \since QGIS 3.0
     */
    int width() const;

    /**
     * The native height of the icon.
     *
     * \since QGIS 3.0
     */
    int height() const;

  signals:

    /**
     * Emitted when the icon changed. You should use connectFrameChanged instead of connecting
     * to this signal directly.
     * Connecting to this signal directly will cause the animation not to be started.
     *
     * \see connectFrameChanged
     */
    void frameChanged();

  private slots:
    void onFrameChanged();

  private:
    QMovie *mMovie = nullptr;
    QIcon mIcon;
};

#endif // QGSANIMATEDICON_H
