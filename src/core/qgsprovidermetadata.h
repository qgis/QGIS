/***************************************************************************
                    qgsprovidermetadata.h  -  Metadata class for
                    describing a data provider.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERMETADATA_H
#define QGSPROVIDERMETADATA_H


#include <QString>

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
class CORE_EXPORT QgsProviderMetadata
{
  public:

    QgsProviderMetadata( const QString & _key, const QString & _description, const QString & _library );

    /** This returns the unique key associated with the provider

        This key string is used for the associative container in QgsProviderRegistry
    */
    const QString & key() const;

    /** This returns descriptive text for the provider

        This is used to provide a descriptive list of available data providers.
    */
    const QString & description() const;

    /** This returns the library file name

        This is used to QLibrary calls to load the data provider.
    */
    const QString & library() const;

  private:

    /// unique key for data provider
    QString key_;

    /// associated terse description
    QString description_;

    /// file path
    QString library_;

}; // class QgsProviderMetadata

#endif //QGSPROVIDERMETADATA_H

