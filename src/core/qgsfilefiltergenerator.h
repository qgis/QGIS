/***************************************************************************
                         qgsfilefiltergenerator.h
                         ------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILEFILTERGENERATOR_H
#define QGSFILEFILTERGENERATOR_H

#include "qgis_core.h"

#include <QCoreApplication>
#include <QString>

/**
 * \ingroup core
 * \brief Abstract interface for classes which generate a file filter string.
 *
 * This interface can be inherited by classes which can generate a file filter
 * string, for use in file open or file save dialogs.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsFileFilterGenerator
{
    Q_DECLARE_TR_FUNCTIONS( QgsFileFilterGenerator )

  public:
    virtual ~QgsFileFilterGenerator() = default;

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return a file filter.
     */
    virtual QString createFileFilter() const = 0;

    /**
     * Standard file filters.
     *
     * \since QGIS 4.6
     */
    enum class StandardFileFormat
    {
      QML,               //!< QML Files
      HTML,              //!< HTML Files
      TXT,               //!< Text files
      SLD,               //!< SLD files
      QgisStyleDbExport, //!< QGIS exported style database
      AllFiles,          //!< All files (*.*)
    };

    /**
     * Generates a file format for the specified \a format.
     *
     * \since QGIS 4.6
     */
    static QString fileFilter( QgsFileFilterGenerator::StandardFileFormat format );

    /**
     * Generates a complete file filter string for all specified \a formats.
     *
     * If \a includeAllFiles is TRUE then an "All Files (*.*)" option will be added to the end of the string.
     *
     * \since QGIS 4.6
     */
    static QString filterString( const QList< QgsFileFilterGenerator::StandardFileFormat > &formats, bool includeAllFiles = false );
};

#endif // QGSPROCESSINGUTILS_H
