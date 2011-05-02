/*!
 * \file qextserialenumerator.h
 * \author Michal Policht
 * \see QextSerialEnumerator
 */

#ifndef _QEXTSERIALENUMERATOR_H_
#define _QEXTSERIALENUMERATOR_H_

#include <QString>
#include <QList>
#include <QObject>

#ifdef Q_OS_WIN
    #ifdef __MINGW32__
        #define _WIN32_WINNT 0x0500
        #define _WIN32_WINDOWS 0x0500
        #define WINVER 0x0500 
    #endif
    #include <windows.h>
    #include <setupapi.h>
    #include <dbt.h>
#endif /*Q_OS_WIN*/

#ifdef Q_OS_MAC
    #include <IOKit/usb/IOUSBLib.h>
#endif

/*!
 * Structure containing port information.
 */
struct QextPortInfo {
    QString portName;   ///< Port name.
    QString physName;   ///< Physical name.
    QString friendName; ///< Friendly name.
    QString enumName;   ///< Enumerator name.
    int vendorID;       ///< Vendor ID.
    int productID;      ///< Product ID
};

#undef QT_GUI_LIB

#ifdef Q_OS_WIN
#ifdef QT_GUI_LIB
#include <QWidget>
class QextSerialEnumerator;

class QextSerialRegistrationWidget : public QWidget
{
    Q_OBJECT
    public:
        QextSerialRegistrationWidget( QextSerialEnumerator* qese ) {
            this->qese = qese;
        }
        ~QextSerialRegistrationWidget( ) { }

    protected:
        QextSerialEnumerator* qese;
        bool winEvent( MSG* message, long* result );
};
#endif // QT_GUI_LIB
#endif // Q_OS_WIN

/*!
  Provides list of ports available in the system.

  \section Usage
  To poll the system for a list of connected devices, simply use getPorts().  Each
  QextPortInfo structure will populated with information about the corresponding device.

  \b Example
  \code
  QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
  foreach( QextPortInfo port, ports ) {
      // inspect port...
  }
  \endcode

  To enable event-driven notification of device connection events, first call
  setUpNotifications() and then connect to the deviceDiscovered() and deviceRemoved()
  signals.  Event-driven behavior is currently available only on Windows and OS X.

  \b Example
  \code
  QextSerialEnumerator* enumerator = new QextSerialEnumerator();
  connect(enumerator, SIGNAL(deviceDiscovered(const QextPortInfo &)),
             myClass, SLOT(onDeviceDiscovered(const QextPortInfo &)));
  connect(enumerator, SIGNAL(deviceRemoved(const QextPortInfo &)),
             myClass, SLOT(onDeviceRemoved(const QextPortInfo &)));
  \endcode

  \section Credits
  Windows implementation is based on Zach Gorman's work from
  <a href="http://www.codeproject.com">The Code Project</a> (http://www.codeproject.com/system/setupdi.asp).

  OS X implementation, see
  http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/AH_Finding_Devices/chapter_4_section_2.html

  \author Michal Policht, Liam Staskawicz
*/
class QextSerialEnumerator : public QObject
{
Q_OBJECT
    public:
        QextSerialEnumerator( );
        ~QextSerialEnumerator( );

        #ifdef Q_OS_WIN
            LRESULT onDeviceChangeWin( WPARAM wParam, LPARAM lParam );
            private:
            /*!
             * Get value of specified property from the registry.
             * 	\param key handle to an open key.
             * 	\param property property name.
             * 	\return property value.
             */
            static QString getRegKeyValue(HKEY key, LPCTSTR property);

            /*!
             * Get specific property from registry.
             * \param devInfo pointer to the device information set that contains the interface
             *    and its underlying device. Returned by SetupDiGetClassDevs() function.
             * \param devData pointer to an SP_DEVINFO_DATA structure that defines the device instance.
             *    this is returned by SetupDiGetDeviceInterfaceDetail() function.
             * \param property registry property. One of defined SPDRP_* constants.
             * \return property string.
             */
            static QString getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property);

            /*!
             * Search for serial ports using setupapi.
             *  \param infoList list with result.
             */
            static void setupAPIScan(QList<QextPortInfo> & infoList);
            void setUpNotificationWin( );
            static bool getDeviceDetailsWin( QextPortInfo* portInfo, HDEVINFO devInfo,
                                  PSP_DEVINFO_DATA devData, WPARAM wParam = DBT_DEVICEARRIVAL );
            static void enumerateDevicesWin( const GUID & guidDev, QList<QextPortInfo>* infoList );
            bool matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam);
            #ifdef QT_GUI_LIB
            QextSerialRegistrationWidget* notificationWidget;
            #endif
        #endif /*Q_OS_WIN*/

        #ifdef Q_OS_UNIX
            #ifdef Q_OS_MAC
            private:
              /*!
               * Search for serial ports using IOKit.
               *    \param infoList list with result.
               */
              static void scanPortsOSX(QList<QextPortInfo> & infoList);
              static void iterateServicesOSX(io_object_t service, QList<QextPortInfo> & infoList);
              static bool getServiceDetailsOSX( io_object_t service, QextPortInfo* portInfo );

              void setUpNotificationOSX( );
              void onDeviceDiscoveredOSX( io_object_t service );
              void onDeviceTerminatedOSX( io_object_t service );
              friend void deviceDiscoveredCallbackOSX( void *ctxt, io_iterator_t serialPortIterator );
              friend void deviceTerminatedCallbackOSX( void *ctxt, io_iterator_t serialPortIterator );

              IONotificationPortRef notificationPortRef;

            #else // Q_OS_MAC
              /*!
               * Search for serial ports on unix.
               *    \param infoList list with result.
               */
              // static void scanPortsNix(QList<QextPortInfo> & infoList);
            #endif // Q_OS_MAC
        #endif /* Q_OS_UNIX */

    public:
        /*!
          Get list of ports.
          \return list of ports currently available in the system.
        */
        static QList<QextPortInfo> getPorts();
        /*!
          Enable event-driven notifications of board discovery/removal.
        */
        void setUpNotifications( );

    signals:
        /*!
          A new device has been connected to the system.

          setUpNotifications() must be called first to enable event-driven device notifications.
          Currently only implemented on Windows and OS X.
          \param info The device that has been discovered.
        */
        void deviceDiscovered( const QextPortInfo & info );
        /*!
          A device has been disconnected from the system.

          setUpNotifications() must be called first to enable event-driven device notifications.
          Currently only implemented on Windows and OS X.
          \param info The device that was disconnected.
        */
        void deviceRemoved( const QextPortInfo & info );
};

#endif /*_QEXTSERIALENUMERATOR_H_*/
