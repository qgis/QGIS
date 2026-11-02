/***************************************************************************
    qgsesriarcadeconverter.h
    --------------------
    begin                : October 2026
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSESRIARCADECONVERTER_H
#define QGSESRIARCADECONVERTER_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>

/**
 * \ingroup core
 * \brief Converts an Esri Arcade script to a QgsExpression.
 *
 * \since QGIS 4.4
 */

class CORE_EXPORT QgsEsriArcadeConverter
{
  public:
    /**
     * Results of converting the Arcade script.
     */
    enum class Result
    {
      Success,                      //!< Script was successfully converted
      ParserError,                  //!< Script could not be parsed
      InvalidScriptMultiStatements, //!< Scripts containing multiple statements cannot be converted
      InvalidScriptIncompatible,    //!< Script is incompatible with conversion to a QGIS expression
      UnknownError,                 //!< An unknown error occurred while converting the script
    };

    /**
     * Constructor for QgsEsriArcadeConverter for converting the specified \a script.
     */
    QgsEsriArcadeConverter( const QString &script );

    /**
     * Returns the result of converting the script.
     *
     * If the conversion was not successful, a descriptive error can be retrieved by errorMessage().
     *
     * \see errorMessage()
     */
    Result result() const { return mResult; }

    /**
     * Returns the descriptive error message, if an error was obtained while converting the script.
     *
     * \see result()
     */
    QString errorMessage() const { return mErrorString; }

    /**
     * Returns the QGIS expression equivalent of the Arcade script, if conversion was successful.
     */
    QString expression() const { return mExpression; }

  private:
    QString mScript;

    QString mErrorString;
    Result mResult = Result::Success;

    QString mExpression;
};

#endif // QGSESRIARCADECONVERTER_H
