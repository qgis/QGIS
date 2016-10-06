
/** \ingroup core
 * Holds data provider key, description, and associated shared library file information.

   The metadata class is used in a lazy load implementation in
   QgsProviderRegistry.  To save memory, data providers are only actually
   loaded via QLibrary calls if they're to be used.  (Though they're all
   iteratively loaded once to get their metadata information, and then
   unloaded when the QgsProviderRegistry is created.)  QgsProviderMetadata
   supplies enough information to be able to later load the associated shared
   library object.

 */
class QgsProviderMetadata
{
%TypeHeaderCode
#include <qgsprovidermetadata.h>
%End

  public:

    QgsProviderMetadata( const QString & _key, const QString & _description, const QString & _library );

    /** This returns the unique key associated with the provider

        This key string is used for the associative container in QgsProviderRegistry
     */
    QString key() const;

    /** This returns descriptive text for the provider

        This is used to provide a descriptive list of available data providers.
     */
    QString description() const;

    /** This returns the library file name

        This is used to QLibrary calls to load the data provider.
     */
    QString library() const;


}; // class QgsProviderMetadata
