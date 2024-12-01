/***************************************************************************
   qgshanadriver.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAEXCEPTION_H
#define QGSHANAEXCEPTION_H

#include "qexception.h"
#include "qgslogger.h"
#include "qgshanautils.h"

class QgsHanaException final : public QException
{
  public:
    explicit QgsHanaException( const QString &what ) noexcept
      : mMessage( QgsHanaUtils::formatErrorMessage( what.toStdString().c_str() ).toStdString() )
    {
      QgsDebugError( what );
    }

    explicit QgsHanaException( const char *what ) noexcept
      : mMessage( QgsHanaUtils::formatErrorMessage( what ).toStdString() )
    {
      QgsDebugError( what );
    }

    void raise() const override { throw *this; }

    QgsHanaException *clone() const override
    {
      return new QgsHanaException( *this );
    }

    char const *what() const noexcept override
    {
      return mMessage.c_str();
    }

  private:
    std::string mMessage;
};

#endif // QGSHANAEXCEPTION_H
