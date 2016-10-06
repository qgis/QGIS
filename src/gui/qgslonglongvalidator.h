/***************************************************************************
                         qgslonglongvalidator.h  -  description
                             -------------------
    begin                : August 2010
    copyright            : (C) 2010 by Jürgen E. Fischer
    email                : jef@norbit.de

  adapted version of QIntValidator for qint64
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLONGLONGVALIDATOR_H
#define QGSLONGLONGVALIDATOR_H

#include <limits>
#include <QValidator>
#include <QLocale>

/** \ingroup gui
 * \class QgsLongLongValidator
 */
class GUI_EXPORT QgsLongLongValidator : public QValidator
{
    Q_OBJECT

  public:
    explicit QgsLongLongValidator( QObject *parent )
        : QValidator( parent )
        , b( std::numeric_limits<qint64>::min() )
        , t( std::numeric_limits<qint64>::max() )
    {}

    QgsLongLongValidator( qint64 bottom, qint64 top, QObject *parent )
        : QValidator( parent )
        , b( bottom )
        , t( top )
    {}

    ~QgsLongLongValidator()
    {}

    QValidator::State validate( QString &input, int& ) const override
    {
      if ( input.isEmpty() )
        return Intermediate;

      if ( b >= 0 && input.startsWith( '-' ) )
        return Invalid;

      if ( t < 0 && input.startsWith( '+' ) )
        return Invalid;

      if ( input == "-" || input == "+" )
        return Intermediate;


      bool ok;
      qlonglong entered = input.toLongLong( &ok );
      if ( !ok )
        return Invalid;

      if ( entered >= b && entered <= t )
        return Acceptable;

      if ( entered >= 0 )
      {
        // the -entered < b condition is necessary to allow people to type
        // the minus last (e.g. for right-to-left languages)
        return ( entered > t && -entered < b ) ? Invalid : Intermediate;
      }
      else
      {
        return ( entered < b ) ? Invalid : Intermediate;
      }
    }

    void setBottom( qint64 bottom ) { b = bottom; }
    void setTop( qint64 top ) { t = top; }

    virtual void setRange( qint64 bottom, qint64 top )
    {
      b = bottom;
      t = top;
    }

    qint64 bottom() const { return b; }
    qint64 top() const { return t; }

  private:
    Q_DISABLE_COPY( QgsLongLongValidator )

    qint64 b;
    qint64 t;
};

#endif // QGSLONGLONGVALIDATOR_H
