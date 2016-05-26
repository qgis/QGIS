%MappedType std::string
{
%TypeHeaderCode
    #include <string>
%End

%ConvertToTypeCode
    if ( sipIsErr == NULL )
    {
        if ( !PyString_Check( sipPy )
            && !PyUnicode_Check( sipPy ) )
            return 0;
        else
            return 1;
    }

    PyObject* asciiString = sipPy;

    if ( PyUnicode_Check( sipPy ) )
    {
      asciiString = PyUnicode_AsASCIIString( sipPy );

      // Will be NULL if a Unicode string could not be encoded using ASCII
      if ( asciiString )
      {
        *sipCppPtr = new std::string( PyString_AsString( asciiString ) );
        delete ( asciiString );
      }
      else
      {
        *sipIsErr = 1;
      }
    }
    else
    {
      *sipCppPtr = new std::string( PyString_AsString( sipPy ) );
    }

    return sipGetState( sipTransferObj );
%End

%ConvertFromTypeCode
    if( sipCpp )
    {
      PyObject *s = PyString_FromString( sipCpp->c_str() );

      return s;
    }
    else
    {
      Py_INCREF( Py_None );
      return Py_None;
    }
%End
};
