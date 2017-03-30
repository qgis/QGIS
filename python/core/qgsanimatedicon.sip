/******************************************************************
 * This file has been generated automatically by sipify.pl        *
 * Do not edit manually ! Edit header file and generate it again. *
 *****************************************************************/



class QgsAnimatedIcon : QObject
{
%Docstring
 \ingroup core
 Animated icon is keeping an animation running if there are listeners connected to frameChanged
%End

%TypeHeaderCode
#include "qgsanimatedicon.h"
%End
  public:

    QgsAnimatedIcon( const QString &iconPath = QString(), QObject *parent = 0 );
%Docstring
 Create a new animated icon. Optionally, the \a iconPath can already be specified.
%End

    QString iconPath() const;
%Docstring
 Path to a movie, e.g. animated GIF
%End

    void setIconPath( const QString &iconPath );
%Docstring
 Path to a movie, e.g. animated GIF
%End

    QIcon icon() const;
%Docstring
 Get the icons representation in the current frame.
 This will need to be called repeatedly, whenever a frameChanged()
 signal is emitted.
%End


    bool connectFrameChanged( const QObject *receiver, const char *method );
%Docstring
 Connect a slot that will be notified repeatedly whenever a frame changes and which should
 request the current icon and trigger UI updates.

 Connect to the frame changed signal with this method and not directly. This method
 makes sure the annimation is started.

 @note Added in QGIS 3.0
%End

    bool disconnectFrameChanged( const QObject *receiver, const char *method );
%Docstring
 Convenience function to disconnect the same style that the frame change connection was established.

 @note Added in QGIS 3.0
%End


    int width() const;
%Docstring
 The native width of the icon.

 @note Added in QGIS 3.0
%End

    int height() const;
%Docstring
 The native height of the icon.

 @note Added in QGIS 3.0
%End

  signals:

    void frameChanged();
%Docstring
 Emitted when the icon changed. You should use connectFrameChanged instead of connecting
 to this signal directly.
 Connecting to this signal directly will cause the animation not to be started.

 @see connectFrameChanged
%End

};

