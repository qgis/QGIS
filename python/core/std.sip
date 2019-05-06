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

