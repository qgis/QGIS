/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
* Copyright 2008-2013 Pelican Mapping
* http://osgearth.org
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

    /**
     * A template for defining "optional" class members. An optional member has a default value
     * and a flag indicating whether the member is "set".
     * This is used extensively in osgEarth's ConfigOptions subsystem.
     */
template<TYPE>
%MappedType optional<TYPE> /DocType="optional-TYPE"/
{
%TypeHeaderCode
#include <osgEarth/optional>
%End

%ConvertFromTypeCode
  PyObject *o;

  o = sipConvertFromType( &(*sipCpp), sipType_TYPE, sipTransferObj );

  return o;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if( sipIsErr == NULL )
  {
    return sipCanConvertToType(sipPy, sipType_TYPE, SIP_NOT_NONE);
  }

  int state;
  optional<TYPE>* t = new optional<TYPE>( TYPE(), *static_cast<TYPE*>( sipConvertToType( sipPy, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr ) ) );

  *sipCppPtr = t;
%End

//        optional() : _set(false), _value(T()), _defaultValue(T()) { }
//        optional(T defaultValue) : _set(false), _value(defaultValue), _defaultValue(defaultValue) { }
//        optional(T defaultValue, T value) : _set(true), _value(value), _defaultValue(defaultValue) { }
//        optional(const optional<T>& rhs) { (*this)=rhs; }
//        virtual ~optional() { }
//        optional<T>& operator =(const optional<T>& rhs) { _set=rhs._set; _value=rhs._value; _defaultValue=rhs._defaultValue; return *this; }
//        const T& operator =(const T& value) { _set=true; _value=value; return _value; }
//        bool operator ==(const optional<T>& rhs) const { return _set && rhs._set && _value==rhs._value; }
//        bool operator !=(const optional<T>& rhs) const { return !( (*this)==rhs); }
//        bool operator ==(const T& value) const { return _value==value; }
//        bool operator !=(const T& value) const { return _value!=value; }
//        bool operator > (const T& value) const { return _value>value; }
//        bool operator >=(const T& value) const { return _value>=value; }
//        bool operator < (const T& value) const { return _value<value; }
//        bool operator <=(const T& value) const { return _value<=value; }
//        bool isSetTo(const T& value) const { return _set && _value==value; } // differs from == in that the value must be explicity set
//        bool isSet() const { return _set; }
//        void unset() { _set = false; _value=_defaultValue; }
//
//        const T& get() const { return _value; }
//        const T& value() const { return _value; }
//        const T& defaultValue() const { return _defaultValue; }
//        T temp_copy() const { return _value; }
//
//        const T& getOrUse(const T& fallback) const { return _set ? _value : fallback; }
//
//        // gets a mutable reference, automatically setting
//        T& mutable_value() { _set = true; return _value; }
//
//        void init(T defValue) { _value=defValue; _defaultValue=defValue; unset(); }
//
//        operator const T*() const { return &_value; }
//
//        T* operator ->() { _set=true; return &_value; }
//        const T* operator ->() const { return &_value; }
//
//    private:
//        bool _set;
//        T _value;
//        T _defaultValue;
//        typedef T* optional::*unspecified_bool_type;
//
//    public:
//        operator unspecified_bool_type() const { return 0; }
    };
