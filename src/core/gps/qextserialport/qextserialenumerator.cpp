#include "qextserialenumerator.h"
#include <QDebug>
#include <QMetaType>

#ifdef Q_WS_WIN
#ifdef UNICODE
#define TCHAR
#else
#endif
#endif

QextSerialEnumerator::QextSerialEnumerator( )
{
    if( !QMetaType::isRegistered( QMetaType::type("QextPortInfo") ) )
        qRegisterMetaType<QextPortInfo>("QextPortInfo");
#if (defined Q_OS_WIN) && (defined QT_GUI_LIB)
    notificationWidget = 0;
#endif // Q_OS_WIN
}

QextSerialEnumerator::~QextSerialEnumerator( )
{
#ifdef Q_OS_MAC
    IONotificationPortDestroy( notificationPortRef );
#endif
#if (defined Q_OS_WIN) && (defined QT_GUI_LIB)
    if( notificationWidget )
        delete notificationWidget;
#endif
}

#ifdef Q_OS_WIN

    #include <objbase.h>
    #include <initguid.h>
    #include "qextserialport.h"
    #include <QRegExp>

    // see http://msdn.microsoft.com/en-us/library/ms791134.aspx for list of GUID classes
    #ifndef GUID_DEVCLASS_PORTS
        DEFINE_GUID(GUID_DEVCLASS_PORTS, 0x4D36E978, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 );
    #endif

    /* Gordon Schumacher's macros for TCHAR -> QString conversions and vice versa */
    #ifdef UNICODE
        #define QStringToTCHAR(x)     (wchar_t*) x.utf16()
        #define PQStringToTCHAR(x)    (wchar_t*) x->utf16()
        #define TCHARToQString(x)     QString::fromUtf16((ushort*)(x))
        #define TCHARToQStringN(x,y)  QString::fromUtf16((ushort*)(x),(y))
    #else
        #define QStringToTCHAR(x)     x.local8Bit().constData()
        #define PQStringToTCHAR(x)    x->local8Bit().constData()
        #define TCHARToQString(x)     QString::fromLocal8Bit((char*)(x))
        #define TCHARToQStringN(x,y)  QString::fromLocal8Bit((x),(y))
    #endif /*UNICODE*/


    //static
    QString QextSerialEnumerator::getRegKeyValue(HKEY key, LPCTSTR property)
    {
        DWORD size = 0;
        DWORD type;
        RegQueryValueEx(key, property, NULL, NULL, NULL, & size);
        BYTE* buff = new BYTE[size];
        QString result;
        if( RegQueryValueEx(key, property, NULL, &type, buff, & size) == ERROR_SUCCESS )
            result = TCHARToQString(buff);
        RegCloseKey(key);
        delete [] buff;
        return result;
    }

    //static
    QString QextSerialEnumerator::getDeviceProperty(HDEVINFO devInfo, PSP_DEVINFO_DATA devData, DWORD property)
    {
        DWORD buffSize = 0;
        SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, NULL, 0, & buffSize);
        BYTE* buff = new BYTE[buffSize];
        SetupDiGetDeviceRegistryProperty(devInfo, devData, property, NULL, buff, buffSize, NULL);
        QString result = TCHARToQString(buff);
        delete [] buff;
        return result;
    }

    //static
    void QextSerialEnumerator::setupAPIScan(QList<QextPortInfo> & infoList)
    {
#if 0
        enumerateDevicesWin(GUID_DEVCLASS_PORTS, &infoList);
#else
        // just iterate the com ports.
        // looks like GUID_DEVCLASS_PORTS doesn't find them all.
        for( unsigned int i = 0 ; i < 256; i++ )
        {
          QString port = "\\\\.\\COM" + QString::number(i);

          HANDLE hPort = ::CreateFile(port.toAscii(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
          if( hPort == INVALID_HANDLE_VALUE )
            continue;

          QextPortInfo info;
          info.friendName = QString("COM%1:").arg( i );
          info.portName = port;
          infoList.append(info);

          CloseHandle( hPort );
	}
#endif
    }

    void QextSerialEnumerator::enumerateDevicesWin( const GUID & guid, QList<QextPortInfo>* infoList )
    {
        HDEVINFO devInfo;
        if( (devInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT)) != INVALID_HANDLE_VALUE)
        {
            SP_DEVINFO_DATA devInfoData;
            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            for(int i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devInfoData); i++)
            {
                QextPortInfo info;
                info.productID = info.vendorID = 0;
                getDeviceDetailsWin( &info, devInfo, &devInfoData );
                infoList->append(info);
            }
            SetupDiDestroyDeviceInfoList(devInfo);
        }
    }

#ifdef QT_GUI_LIB
    bool QextSerialRegistrationWidget::winEvent( MSG* message, long* result )
    {
        if ( message->message == WM_DEVICECHANGE ) {
            qese->onDeviceChangeWin( message->wParam, message->lParam );
            *result = 1;
            return true;
        }
        return false;
    }
#endif

    void QextSerialEnumerator::setUpNotificationWin( )
    {
        #ifdef QT_GUI_LIB
        if(notificationWidget)
            return;
        notificationWidget = new QextSerialRegistrationWidget(this);

        DEV_BROADCAST_DEVICEINTERFACE dbh;
        ZeroMemory(&dbh, sizeof(dbh));
        dbh.dbcc_size = sizeof(dbh);
        dbh.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        CopyMemory(&dbh.dbcc_classguid, &GUID_DEVCLASS_PORTS, sizeof(GUID));
        if( RegisterDeviceNotification( notificationWidget->winId( ), &dbh, DEVICE_NOTIFY_WINDOW_HANDLE ) == NULL)
            qWarning() << "RegisterDeviceNotification failed:" << GetLastError();
        #else
        qWarning("QextSerialEnumerator: GUI not enabled - can't register for device notifications.");
        #endif // QT_GUI_LIB
    }

    LRESULT QextSerialEnumerator::onDeviceChangeWin( WPARAM wParam, LPARAM lParam )
    {
        if ( DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam )
        {
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
            if( pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
                 // delimiters are different across APIs...change to backslash.  ugh.
                QString deviceID = TCHARToQString(pDevInf->dbcc_name).toUpper().replace("#", "\\");

                matchAndDispatchChangedDevice(deviceID, GUID_DEVCLASS_PORTS, wParam);
            }
        }
        return 0;
    }

    bool QextSerialEnumerator::matchAndDispatchChangedDevice(const QString & deviceID, const GUID & guid, WPARAM wParam)
    {
        bool rv = false;
        DWORD dwFlag = (DBT_DEVICEARRIVAL == wParam) ? DIGCF_PRESENT : DIGCF_ALLCLASSES;
        HDEVINFO devInfo;
        if( (devInfo = SetupDiGetClassDevs(&guid,NULL,NULL,dwFlag)) != INVALID_HANDLE_VALUE )
        {
            SP_DEVINFO_DATA spDevInfoData;
            spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            for(int i=0; SetupDiEnumDeviceInfo(devInfo, i, &spDevInfoData); i++)
            {
                DWORD nSize=0;
                TCHAR buf[MAX_PATH];
                if ( SetupDiGetDeviceInstanceId(devInfo, &spDevInfoData, buf, MAX_PATH, &nSize) &&
                        deviceID.contains(TCHARToQString(buf))) // we found a match
                {
                    rv = true;
                    QextPortInfo info;
                    info.productID = info.vendorID = 0;
                    getDeviceDetailsWin( &info, devInfo, &spDevInfoData, wParam );
                    if( wParam == DBT_DEVICEARRIVAL )
                        emit deviceDiscovered(info);
                    else if( wParam == DBT_DEVICEREMOVECOMPLETE )
                        emit deviceRemoved(info);
                    break;
                }
            }
            SetupDiDestroyDeviceInfoList(devInfo);
        }
        return rv;
    }

    bool QextSerialEnumerator::getDeviceDetailsWin( QextPortInfo* portInfo, HDEVINFO devInfo, PSP_DEVINFO_DATA devData, WPARAM wParam )
    {
        portInfo->friendName = getDeviceProperty(devInfo, devData, SPDRP_FRIENDLYNAME);
        if( wParam == DBT_DEVICEARRIVAL)
            portInfo->physName = getDeviceProperty(devInfo, devData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
        portInfo->enumName = getDeviceProperty(devInfo, devData, SPDRP_ENUMERATOR_NAME);
        QString hardwareIDs = getDeviceProperty(devInfo, devData, SPDRP_HARDWAREID);
        HKEY devKey = SetupDiOpenDevRegKey(devInfo, devData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        portInfo->portName = QextSerialPort::fullPortNameWin( getRegKeyValue(devKey, TEXT("PortName")) );
        QRegExp idRx("VID_(\\w+)&PID_(\\w+)");
        if( hardwareIDs.toUpper().contains(idRx) )
        {
            bool dummy;
            portInfo->vendorID = idRx.cap(1).toInt(&dummy, 16);
            portInfo->productID = idRx.cap(2).toInt(&dummy, 16);
            //qDebug() << "got vid:" << vid << "pid:" << pid;
        }
        return true;
    }

#endif /*Q_OS_WIN*/

#ifdef Q_OS_UNIX

#ifdef Q_OS_MAC
#include <IOKit/serial/IOSerialKeys.h>
#include <CoreFoundation/CFNumber.h>
#include <sys/param.h>

// static
void QextSerialEnumerator::scanPortsOSX(QList<QextPortInfo> & infoList)
{
    io_iterator_t serialPortIterator = 0;
    kern_return_t kernResult = KERN_FAILURE;
    CFMutableDictionaryRef matchingDictionary;

    // first try to get any serialbsd devices, then try any USBCDC devices
    if( !(matchingDictionary = IOServiceMatching(kIOSerialBSDServiceValue) ) ) {
        qWarning("IOServiceMatching returned a NULL dictionary.");
        return;
    }
    CFDictionaryAddValue(matchingDictionary, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));

    // then create the iterator with all the matching devices
    if( IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictionary, &serialPortIterator) != KERN_SUCCESS ) {
        qCritical() << "IOServiceGetMatchingServices failed, returned" << kernResult;
        return;
    }
    iterateServicesOSX(serialPortIterator, infoList);
    IOObjectRelease(serialPortIterator);
    serialPortIterator = 0;

    if( !(matchingDictionary = IOServiceNameMatching("AppleUSBCDC")) ) {
        qWarning("IOServiceNameMatching returned a NULL dictionary.");
        return;
    }

    if( IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictionary, &serialPortIterator) != KERN_SUCCESS ) {
        qCritical() << "IOServiceGetMatchingServices failed, returned" << kernResult;
        return;
    }
    iterateServicesOSX(serialPortIterator, infoList);
    IOObjectRelease(serialPortIterator);
}

void QextSerialEnumerator::iterateServicesOSX(io_object_t service, QList<QextPortInfo> & infoList)
{
    // Iterate through all modems found.
    io_object_t usbService;
    while( ( usbService = IOIteratorNext(service) ) )
    {
        QextPortInfo info;
        info.vendorID = 0;
        info.productID = 0;
        getServiceDetailsOSX( usbService, &info );
        infoList.append(info);
    }
}

bool QextSerialEnumerator::getServiceDetailsOSX( io_object_t service, QextPortInfo* portInfo )
{
    bool retval = true;
    CFTypeRef bsdPathAsCFString = NULL;
    CFTypeRef productNameAsCFString = NULL;
    CFTypeRef vendorIdAsCFNumber = NULL;
    CFTypeRef productIdAsCFNumber = NULL;
    // check the name of the modem's callout device
    bsdPathAsCFString = IORegistryEntryCreateCFProperty(service, CFSTR(kIOCalloutDeviceKey),
                                                        kCFAllocatorDefault, 0);

    // wander up the hierarchy until we find the level that can give us the
    // vendor/product IDs and the product name, if available
    io_registry_entry_t parent;
    kern_return_t kernResult = IORegistryEntryGetParentEntry(service, kIOServicePlane, &parent);
    while( kernResult == KERN_SUCCESS && !vendorIdAsCFNumber && !productIdAsCFNumber )
    {
        if(!productNameAsCFString)
            productNameAsCFString = IORegistryEntrySearchCFProperty(parent,
                                                                    kIOServicePlane,
                                                                    CFSTR("Product Name"),
                                                                    kCFAllocatorDefault, 0);
        vendorIdAsCFNumber = IORegistryEntrySearchCFProperty(parent,
                                                             kIOServicePlane,
                                                             CFSTR(kUSBVendorID),
                                                             kCFAllocatorDefault, 0);
        productIdAsCFNumber = IORegistryEntrySearchCFProperty(parent,
                                                              kIOServicePlane,
                                                              CFSTR(kUSBProductID),
                                                              kCFAllocatorDefault, 0);
        io_registry_entry_t oldparent = parent;
        kernResult = IORegistryEntryGetParentEntry(parent, kIOServicePlane, &parent);
        IOObjectRelease(oldparent);
    }

    io_string_t ioPathName;
    IORegistryEntryGetPath( service, kIOServicePlane, ioPathName );
    portInfo->physName = ioPathName;

    if( bsdPathAsCFString )
    {
        char path[MAXPATHLEN];
        if( CFStringGetCString((CFStringRef)bsdPathAsCFString, path,
                               PATH_MAX, kCFStringEncodingUTF8) )
            portInfo->portName = path;
        CFRelease(bsdPathAsCFString);
    }

    if(productNameAsCFString)
    {
        char productName[MAXPATHLEN];
        if( CFStringGetCString((CFStringRef)productNameAsCFString, productName,
                               PATH_MAX, kCFStringEncodingUTF8) )
            portInfo->friendName = productName;
        CFRelease(productNameAsCFString);
    }

    if(vendorIdAsCFNumber)
    {
        SInt32 vID;
        if(CFNumberGetValue((CFNumberRef)vendorIdAsCFNumber, kCFNumberSInt32Type, &vID))
            portInfo->vendorID = vID;
        CFRelease(vendorIdAsCFNumber);
    }

    if(productIdAsCFNumber)
    {
        SInt32 pID;
        if(CFNumberGetValue((CFNumberRef)productIdAsCFNumber, kCFNumberSInt32Type, &pID))
            portInfo->productID = pID;
        CFRelease(productIdAsCFNumber);
    }
    IOObjectRelease(service);
    return retval;
}

// IOKit callbacks registered via setupNotifications()
void deviceDiscoveredCallbackOSX( void *ctxt, io_iterator_t serialPortIterator );
void deviceTerminatedCallbackOSX( void *ctxt, io_iterator_t serialPortIterator );

void deviceDiscoveredCallbackOSX( void *ctxt, io_iterator_t serialPortIterator )
{
    QextSerialEnumerator* qese = (QextSerialEnumerator*)ctxt;
    io_object_t serialService;
    while ((serialService = IOIteratorNext(serialPortIterator)))
        qese->onDeviceDiscoveredOSX(serialService);
}

void deviceTerminatedCallbackOSX( void *ctxt, io_iterator_t serialPortIterator )
{
    QextSerialEnumerator* qese = (QextSerialEnumerator*)ctxt;
    io_object_t serialService;
    while ((serialService = IOIteratorNext(serialPortIterator)))
        qese->onDeviceTerminatedOSX(serialService);
}

/*
  A device has been discovered via IOKit.
  Create a QextPortInfo if possible, and emit the signal indicating that we've found it.
*/
void QextSerialEnumerator::onDeviceDiscoveredOSX( io_object_t service )
{
    QextPortInfo info;
    info.vendorID = 0;
    info.productID = 0;
    if( getServiceDetailsOSX( service, &info ) )
        emit deviceDiscovered( info );
}

/*
  Notification via IOKit that a device has been removed.
  Create a QextPortInfo if possible, and emit the signal indicating that it's gone.
*/
void QextSerialEnumerator::onDeviceTerminatedOSX( io_object_t service )
{
    QextPortInfo info;
    info.vendorID = 0;
    info.productID = 0;
    if( getServiceDetailsOSX( service, &info ) )
        emit deviceRemoved( info );
}

/*
  Create matching dictionaries for the devices we want to get notifications for,
  and add them to the current run loop.  Invoke the callbacks that will be responding
  to these notifications once to arm them, and discover any devices that
  are currently connected at the time notifications are setup.
*/
void QextSerialEnumerator::setUpNotificationOSX( )
{
    kern_return_t kernResult;
    mach_port_t masterPort;
    CFRunLoopSourceRef notificationRunLoopSource;
    CFMutableDictionaryRef classesToMatch;
    CFMutableDictionaryRef cdcClassesToMatch;
    io_iterator_t portIterator;

    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (KERN_SUCCESS != kernResult) {
        qDebug() << "IOMasterPort returned:" << kernResult;
        return;
    }

    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch == NULL)
        qDebug("IOServiceMatching returned a NULL dictionary.");
    else
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));

    if( !(cdcClassesToMatch = IOServiceNameMatching("AppleUSBCDC") ) ) {
        qWarning("couldn't create cdc matching dict");
        return;
    }

    // Retain an additional reference since each call to IOServiceAddMatchingNotification consumes one.
    classesToMatch = (CFMutableDictionaryRef) CFRetain(classesToMatch);
    cdcClassesToMatch = (CFMutableDictionaryRef) CFRetain(cdcClassesToMatch);

    notificationPortRef = IONotificationPortCreate(masterPort);
    if(notificationPortRef == NULL) {
        qDebug("IONotificationPortCreate return a NULL IONotificationPortRef.");
        return;
    }

    notificationRunLoopSource = IONotificationPortGetRunLoopSource(notificationPortRef);
    if (notificationRunLoopSource == NULL) {
        qDebug("IONotificationPortGetRunLoopSource returned NULL CFRunLoopSourceRef.");
        return;
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), notificationRunLoopSource, kCFRunLoopDefaultMode);

    kernResult = IOServiceAddMatchingNotification(notificationPortRef, kIOMatchedNotification, classesToMatch,
                                                  deviceDiscoveredCallbackOSX, this, &portIterator);
    if (kernResult != KERN_SUCCESS) {
        qDebug() << "IOServiceAddMatchingNotification return:" << kernResult;
        return;
    }

    // arm the callback, and grab any devices that are already connected
    deviceDiscoveredCallbackOSX( this, portIterator );

    kernResult = IOServiceAddMatchingNotification(notificationPortRef, kIOMatchedNotification, cdcClassesToMatch,
                                                  deviceDiscoveredCallbackOSX, this, &portIterator);
    if (kernResult != KERN_SUCCESS) {
        qDebug() << "IOServiceAddMatchingNotification return:" << kernResult;
        return;
    }

    // arm the callback, and grab any devices that are already connected
    deviceDiscoveredCallbackOSX( this, portIterator );

    kernResult = IOServiceAddMatchingNotification(notificationPortRef, kIOTerminatedNotification, classesToMatch,
                                                  deviceTerminatedCallbackOSX, this, &portIterator);
    if (kernResult != KERN_SUCCESS) {
        qDebug() << "IOServiceAddMatchingNotification return:" << kernResult;
        return;
    }

    // arm the callback, and clear any devices that are terminated
    deviceTerminatedCallbackOSX( this, portIterator );

    kernResult = IOServiceAddMatchingNotification(notificationPortRef, kIOTerminatedNotification, cdcClassesToMatch,
                                                  deviceTerminatedCallbackOSX, this, &portIterator);
    if (kernResult != KERN_SUCCESS) {
        qDebug() << "IOServiceAddMatchingNotification return:" << kernResult;
        return;
    }

    // arm the callback, and clear any devices that are terminated
    deviceTerminatedCallbackOSX( this, portIterator );
}
#endif // Q_OS_MAC

#endif // Q_OS_UNIX

//static
QList<QextPortInfo> QextSerialEnumerator::getPorts()
{
    QList<QextPortInfo> ports;

    #ifdef Q_OS_WIN
        OSVERSIONINFO vi;
        vi.dwOSVersionInfoSize = sizeof(vi);
        if (!::GetVersionEx(&vi)) {
            qCritical("Could not get OS version.");
            return ports;
        }
        // Handle windows 9x and NT4 specially
        if (vi.dwMajorVersion < 5) {
            qCritical("Enumeration for this version of Windows is not implemented yet");
        /*if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
                EnumPortsWNt4(ports);
            else
                EnumPortsW9x(ports);*/
        } else  //w2k or later
            setupAPIScan(ports);
    #endif /*Q_OS_WIN*/
    #ifdef Q_OS_UNIX
        #ifdef Q_OS_MAC
            scanPortsOSX(ports);
        #else
            qCritical("Enumeration for POSIX systems is not implemented yet.");
        #endif /* Q_OS_MAC */
    #endif /*Q_OS_UNIX*/

    return ports;
}

void QextSerialEnumerator::setUpNotifications( )
{
#ifdef Q_OS_WIN
    setUpNotificationWin( );
#endif

#ifdef Q_OS_UNIX
#ifdef Q_OS_MAC
    setUpNotificationOSX( );
#else
    qCritical("Notifications for *Nix/FreeBSD are not implemented yet");
#endif // Q_OS_MAC
#endif // Q_OS_UNIX
}
