/* Copyright 2014, Uwe L. Korn <uwelk@xhochy.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#ifndef QJSONWRAPPER_JSON_H
#define QJSONWRAPPER_JSON_H

#include <QVariant>

namespace QJsonWrapper
{

  /**
   * Convert a QObject instance to a QVariantMap by adding its properties
   * as key-value pairs.
   *
   * @param object Object that shall be "serialised"
   * @return All properties of the object stored as QVariantMap
   */
  QVariantMap qobject2qvariant( const QObject *object );

  /**
   * Write out all key-value pairs into the respective properties of the
   * given object.
   *
   * @param variant The key-value pairs that shall be stored in the object.
   * @param object The destiation object where we store the key-value pairs of the map as properties.
   */
  void qvariant2qobject( const QVariantMap &variant, QObject *object );

  /**
   * Parse the JSON string and return the result as a QVariant.
   *
   * @param jsonData The string containing the data as JSON.
   * @param ok Set to TRUE if the conversion was successful, otherwise FALSE.
   * @param errorString Any error string produced during parsing
   * @return After a successful conversion the parsed data either as QVariantMap or QVariantList.
   */
  QVariant parseJson( const QByteArray &jsonData, bool *ok = nullptr, QByteArray *errorString = nullptr );

  /**
   * Convert a QVariant to a JSON representation.
   *
   * This function will accept Strings, Number, QVariantList and QVariantMaps
   * as input types. Although Qt5's JSON implementation itself does not
   * support the serialisation of QVariantHash, we will convert a QVariantHash
   * to a QVariantMap but it is suggest to convert all QVariantHash to
   * QVariantMap in your code than passing them here.
   *
   * @param variant The data to be serialised.
   * @param ok Set to TRUE if the conversion was successful, otherwise FALSE.
   * @param errorString Any error string produced during conversion
   * @param indented Whether to indent resultant JSON code
   * @return After a successful serialisation the data of the QVariant represented as JSON.
   */
  QByteArray toJson( const QVariant &variant, bool *ok = nullptr, QByteArray *errorString = nullptr, bool indented = false );
}

#endif // QJSONWRAPPER_JSON_H
