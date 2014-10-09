/**
 * \class QgsServerInterface
 * \brief Class defining interfaces
 * made available to server plugins.
 *
 * Only functionality exposed by QgisServerInterface can be used in server plugins.
 */


typedef QMultiMap<int, QgsServerFilter*> QgsServerFiltersMap;

class QgsServerInterface
{
%TypeHeaderCode
  #include "qgsserverinterface.h"
%End

  public:
    //virtual void setRequestHandler( QgsRequestHandler* requestHandler) = 0;
    virtual QgsCapabilitiesCache* capabiblitiesCache() = 0 /KeepReference/;
    virtual QgsRequestHandler* requestHandler( ) = 0 /KeepReference/;
    // Tansfer ownership to avoid garbage collector to call dtor
    virtual void registerFilter( QgsServerFilter* filter /Transfer/, int priority = 0 ) = 0;
// Commented because of problem with typedef QgsServerFiltersMap
//    virtual QgsServerFiltersMap filters( ) = 0;

private:
    /** Constructor */
    QgsServerInterface( );

};

