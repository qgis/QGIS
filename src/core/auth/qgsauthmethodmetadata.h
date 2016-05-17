/***************************************************************************
    qgsauthmethodmetadata.h
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHMETHODMETADATA_H
#define QGSAUTHMETHODMETADATA_H

#include <QString>

/** \ingroup core
 * Holds data auth method key, description, and associated shared library file information.

   The metadata class is used in a lazy load implementation in
   QgsAuthMethodRegistry.  To save memory, auth methods are only actually
   loaded via QLibrary calls if they're to be used.  (Though they're all
   iteratively loaded once to get their metadata information, and then
   unloaded when the QgsAuthMethodRegistry is created.)  QgsProviderMetadata
   supplies enough information to be able to later load the associated shared
   library object.
 * \note Culled from QgsProviderMetadata
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsAuthMethodMetadata
{
  public:
    /**
     * Construct an authentication method metadata container
     * @param _key Textual key of the library plugin
     * @param _description Description of the library plugin
     * @param _library File name of library plugin
     */
    QgsAuthMethodMetadata( const QString & _key, const QString & _description, const QString & _library );

    /** This returns the unique key associated with the method

        This key string is used for the associative container in QgsAtuhMethodRegistry
     */
    QString key() const;

    /** This returns descriptive text for the method

        This is used to provide a descriptive list of available data methods.
     */
    QString description() const;

    /** This returns the library file name

        This is used to QLibrary calls to load the method.
     */
    QString library() const;

  private:

    /// unique key for method
    QString key_;

    /// associated terse description
    QString description_;

    /// file path
    QString library_;
};

#endif // QGSAUTHMETHODMETADATA_H
