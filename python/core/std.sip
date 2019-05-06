/* std:: conversions */

%MappedType std::string
{
%TypeHeaderCode
#include <string>
%End

%ConvertFromTypeCode
    // convert an std::string to a Python (unicode) string
    PyObject* newstring;
    newstring = PyUnicode_DecodeUTF8(sipCpp->c_str(), sipCpp->length(), NULL);
    if(newstring == NULL) {
        PyErr_Clear();
        newstring = PyUnicode_FromString(sipCpp->c_str());
    }
    return newstring;
%End

%ConvertToTypeCode
    // Allow a Python string (or a unicode string) whenever a string is
    // expected.
    // If argument is a Unicode string, just decode it to UTF-8
        if (sipIsErr == NULL)
                return (PyUnicode_Check(sipPy));
        if (sipPy == Py_None) {
                *sipCppPtr = new std::string;
                return 1;
        }
        if (PyUnicode_Check(sipPy))	{
        PyObject* s = PyUnicode_AsEncodedString(sipPy, "UTF-8", "");
        *sipCppPtr = new std::string(PyUnicode_AS_DATA(s));
        Py_DECREF(s);
        return 1;
        }
    return 0;
%End
};

template<CLASS>
%MappedType std::unique_ptr<CLASS>
{
//%TypeHeaderCode
//%End

// Convert the value from C++.
%ConvertFromTypeCode
  const sipTypeDef *genvalueT = sipType_CLASS;
  // Note: the ptr is NOT moved! Ownership remains untouched
  PyObject *value = sipConvertFromType((void *)sipCpp->get(), genvalueT, sipTransferObj);
  if (value == NULL) {
    PyErr_Format(PyExc_TypeError, "cannot convert value");
    return 0;
  }
  return value;
%End

// Convert the value to C++.
%ConvertToTypeCode
  const sipTypeDef *genvalueT = sipType_CLASS;
  PyObject *value;
  value = sipPy;
  int valueState;
  CLASS *cxxvalue = reinterpret_cast<CLASS*>(sipForceConvertToType(value, genvalueT, sipTransferObj, SIP_NOT_NONE, &valueState, sipIsErr));

  if (*sipIsErr) {
      if (cxxvalue == NULL) {
          PyErr_Format(PyExc_TypeError, "value has type '%s' but '%s' is expected",
          Py_TYPE(value)->tp_name, "CLASS");
      }
      sipReleaseType((void *)cxxvalue, genvalueT, valueState);
      return 0;
  }
  sipReleaseType((void *)cxxvalue, genvalueT, valueState);
  *sipCppPtr.reset(cxxvalue);
  return sipGetState(sipTransferObj);
%End
};
