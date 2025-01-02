/*
This file contains code for conversion between various (often nested) mapped types
which are not wrapped by PyQt:
- QVector< QVector<TYPE> >
- QVector< QVector< QVector<TYPE> > >
- QList< QList<TYPE> >
- QList<qint64>
- QSet<int>
- QSet<qint64>
- QSet<TYPE>
- QMap<qint64, QMap<int, TYPE> >
- QMap<QString, QVariant::Type>
- QMap<QString, int>
- QMap<TYPE1, TYPE2*>
- QMap<double, TYPE>
- QMultiMap<double, TYPE2>
- QMultiMap<int, TYPE2>
- QMap<qint64, TYPE>
- QList< QPair< QString, QList<QString> > >
- QVector<TYPE*>
- QMap<qint64, QgsFeature*>
- NULL QVariant which is missing in PyQt5 with sip.enableautoconversion
*/


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::GeometryType>
        /TypeHintIn="Iterable[Qgis.GeometryType]",
        TypeHintOut="List[Qgis.GeometryType]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_GeometryType);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::GeometryType> *ql = new QList<Qgis::GeometryType>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_GeometryType);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.GeometryType' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::GeometryType>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::WkbType>
        /TypeHintIn="Iterable[Qgis.WkbType]",
        TypeHintOut="List[Qgis.WkbType]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_WkbType);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::WkbType> *ql = new QList<Qgis::WkbType>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_WkbType);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.WkbType' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::WkbType>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::LayerType>
        /TypeHintIn="Iterable[Qgis.LayerType]",
        TypeHintOut="List[Qgis.LayerType]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_LayerType);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::LayerType> *ql = new QList<Qgis::LayerType>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_LayerType);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.LayerType' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::LayerType>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::DistanceUnit>
        /TypeHintIn="Iterable[Qgis.DistanceUnit]",
        TypeHintOut="List[Qgis.DistanceUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_DistanceUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::DistanceUnit> *ql = new QList<Qgis::DistanceUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_DistanceUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.DistanceUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::DistanceUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::AreaUnit>
        /TypeHintIn="Iterable[Qgis.AreaUnit]",
        TypeHintOut="List[Qgis.AreaUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_AreaUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::AreaUnit> *ql = new QList<Qgis::AreaUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_AreaUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.AreaUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::AreaUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::VolumeUnit>
        /TypeHintIn="Iterable[Qgis.VolumeUnit]",
        TypeHintOut="List[Qgis.VolumeUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_VolumeUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::VolumeUnit> *ql = new QList<Qgis::VolumeUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_VolumeUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.VolumeUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::VolumeUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::AngleUnit>
        /TypeHintIn="Iterable[Qgis.AngleUnit]",
        TypeHintOut="List[Qgis.AngleUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_AngleUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::AngleUnit> *ql = new QList<Qgis::AngleUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_AngleUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.AngleUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::AngleUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::TemporalUnit>
        /TypeHintIn="Iterable[Qgis.TemporalUnit]",
        TypeHintOut="List[Qgis.TemporalUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_TemporalUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::TemporalUnit> *ql = new QList<Qgis::TemporalUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_TemporalUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.TemporalUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::TemporalUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::RenderUnit>
        /TypeHintIn="Iterable[Qgis.RenderUnit]",
        TypeHintOut="List[Qgis.RenderUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_RenderUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::RenderUnit> *ql = new QList<Qgis::RenderUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_RenderUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.RenderUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::RenderUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::LayoutUnit>
        /TypeHintIn="Iterable[Qgis.LayoutUnit]",
        TypeHintOut="List[Qgis.LayoutUnit]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include "qgis.h"
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                sipType_Qgis_LayoutUnit);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::LayoutUnit> *ql = new QList<Qgis::LayoutUnit>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_LayoutUnit);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'Qgis.LayoutUnit' is expected",
                    i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::LayoutUnit>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QList<Qgis::SensorThingsEntity>
    /TypeHintIn="Iterable[Qgis.SensorThingsEntity]",
    TypeHintOut="List[Qgis.SensorThingsEntity]", TypeHintValue="[]"/
{
    %TypeHeaderCode
#include "qgis.h"
        %End

        %ConvertFromTypeCode
            PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                                            sipType_Qgis_SensorThingsEntity);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
    %End

        %ConvertToTypeCode
            PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<Qgis::SensorThingsEntity> *ql = new QList<Qgis::SensorThingsEntity>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_SensorThingsEntity);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                         "index %zd has type '%s' but 'Qgis.SensorThingsEntity' is expected",
                         i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::SensorThingsEntity>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
    %End
};


// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
%MappedType QVector<Qgis::LabelPredefinedPointPosition>
    /TypeHintIn="Iterable[Qgis.LabelPredefinedPointPosition]",
    TypeHintOut="List[Qgis.LabelPredefinedPointPosition]", TypeHintValue="[]"/
{
    %TypeHeaderCode
#include "qgis.h"
        %End

        %ConvertFromTypeCode
            PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *eobj = sipConvertFromEnum(static_cast<int>(sipCpp->at(i)),
                                            sipType_Qgis_LabelPredefinedPointPosition);

        if (!eobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, eobj);
    }

    return l;
    %End

        %ConvertToTypeCode
            PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QVector<Qgis::LabelPredefinedPointPosition> *ql = new QVector<Qgis::LabelPredefinedPointPosition>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int v = sipConvertToEnum(itm, sipType_Qgis_LabelPredefinedPointPosition);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                         "index %zd has type '%s' but 'Qgis.LabelPredefinedPointPosition' is expected",
                         i, sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(static_cast<Qgis::LabelPredefinedPointPosition>(v));

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
    %End
};

template <TYPE>
%MappedType std::unique_ptr< TYPE > /NoRelease,AllowNone,TypeHint="Optional[TYPE]",TypeHintValue="TYPE"/
{
%TypeHeaderCode
#include <memory>
%End

%ConvertFromTypeCode
    const sipTypeDef *sip_type = sipFindType("TYPE*");
    return sipConvertFromNewType(sipCpp->release(), sip_type, NULL);
%End

%ConvertToTypeCode
    // only one way for now...
    return 0;
%End
};

template <TYPE>
%MappedType QVector< QVector<TYPE> >
{
%TypeHeaderCode
#include <QVector>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipTypeDef *qvector_type = sipFindType("QVector<TYPE>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<TYPE> *t = new QVector<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, qvector_type, NULL)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  const sipTypeDef *qvector_type = sipFindType("QVector<TYPE>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QVector< QVector<TYPE> > *ql = new QVector< QVector<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<TYPE> *t = reinterpret_cast< QVector<TYPE> * >(sipConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, qvector_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseType(t, qvector_type, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};



template <TYPE>
%MappedType QVector< QVector< QVector<TYPE> > >
{
%TypeHeaderCode
#include <QVector>

%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipTypeDef *qvector_type = sipFindType("QVector<QVector<TYPE> >");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<QVector<TYPE> > *t = new QVector<QVector<TYPE> >(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, qvector_type, NULL)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }
  return l;
%End

%ConvertToTypeCode

  const sipTypeDef *qvector_type = sipFindType("QVector<QVector<TYPE> >");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QVector< QVector< QVector<TYPE> > > *ql = new QVector< QVector< QVector<TYPE> > >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<QVector<TYPE> > *t = reinterpret_cast< QVector< QVector<TYPE> > * >(sipConvertToType(PyList_GET_ITEM(sipPy, i), qvector_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, qvector_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseType(t, qvector_type, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};



template <TYPE>
%MappedType QList< QList<TYPE> >
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  const sipTypeDef *qlist_type = sipFindType("QList<TYPE>");

  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QList<TYPE> *t = new QList<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromNewType(t, qlist_type, NULL)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  const sipTypeDef *qlist_type = sipFindType("QList<TYPE>");

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), qlist_type, SIP_NOT_NONE))
        return 0;

    return 1;
  }


  QList< QList<TYPE> > *ql = new QList< QList<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QList<TYPE> *t = reinterpret_cast< QList<TYPE> * >(sipConvertToType(PyList_GET_ITEM(sipPy, i), qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, qlist_type, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseType(t, qlist_type, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End
};


%MappedType QSet<int>
{
%TypeHeaderCode
#include <QSet>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QSet<int>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyLong_FromLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QSet<int> *qset = new QSet<int>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    qset->insert(PyLong_AsLong(PyList_GET_ITEM(sipPy, i)));
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End
};

%MappedType QList<long>
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QList<long>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyLong_FromLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QList<long> *qlist = new QList<long>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    *qlist << PyLong_AsLong(PyList_GET_ITEM(sipPy, i));
  }

  *sipCppPtr = qlist;
  return sipGetState(sipTransferObj);
%End
};

%MappedType QVector<long long>
    /TypeHintIn="Iterable[int]",
    TypeHintOut="List[int]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include <QVector>
    %End

    %ConvertFromTypeCode
        // Create the list.
        PyObject *l;

if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

// Set the list elements.
QVector<long long>::iterator it = sipCpp->begin();
for (int i = 0; it != sipCpp->end(); ++it, ++i)
{
    PyObject *tobj;

    if ((tobj = PyLong_FromLongLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
}

return l;
%End

    %ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    return PyList_Check(sipPy);

QVector<long long> *qlist = new QVector<long long>;
qlist->reserve( PyList_GET_SIZE(sipPy) );
for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
{
    *qlist << PyLong_AsLongLong(PyList_GET_ITEM(sipPy, i));
}

*sipCppPtr = qlist;
return sipGetState(sipTransferObj);
%End
};


%MappedType QSet<qint64>
{
%TypeHeaderCode
#include <QSet>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QSet<qint64>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = PyLong_FromLongLong(*it)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QSet<qint64> *qset = new QSet<qint64>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    qset->insert(PyLong_AsLongLong(PyList_GET_ITEM(sipPy, i)));
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End
};


template<TYPE>
%MappedType QMap<qint64, QMap<int, TYPE> >
{
%TypeHeaderCode
#include <QtGlobal>
#include <QMap>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *d;

  if ((d = PyDict_New()) == NULL)
    return NULL;

  const sipTypeDef *qmap2 = sipFindType("QMap<int, TYPE>");

  // Set the list elements.
  for (QMap<qint64, QMap<int, TYPE> >::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it)
  {
    QMap<int, TYPE> *t = new QMap<int, TYPE>(*it);

    PyObject *kobj = PyLong_FromLongLong(it.key());
    PyObject *tobj = sipConvertFromNewType(t, qmap2, sipTransferObj);

    if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
    {
      Py_DECREF(d);

      if (kobj)
      {
        Py_DECREF(kobj);
      }

      if (tobj)
      {
        Py_DECREF(tobj);
      }
      else
      {
        delete t;
      }

      return NULL;
    }

    Py_DECREF(kobj);
    Py_DECREF(tobj);
  }

  return d;
%End

%ConvertToTypeCode
  PyObject *kobj, *tobj, *kobj2, *tobj2;
  Py_ssize_t i = 0;

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyDict_Check(sipPy))
      return 0;

    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
      if (!PyDict_Check(tobj))
        return 0;

      Py_ssize_t j = 0;
      while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
      {
        if (!sipCanConvertToType(tobj2, sipType_TYPE, SIP_NOT_NONE))
          return 0;
      }
    }
    return 1;
  }

  QMap<qint64, QMap<int, TYPE> > *qm = new QMap<qint64, QMap<int, TYPE> >;

  while (PyDict_Next(sipPy, &i, &kobj, &tobj))
  {
    qint64 k = PyLong_AsLongLong(kobj);

    // using sipConvertToType to convert directly to QMap<int, TYPE> doesn't work
    // and ends with a segfault

    QMap<int, TYPE> qm2;

    Py_ssize_t j = 0;
    while (PyDict_Next(tobj, &j, &kobj2, &tobj2))
    {
      int k2 = PyLong_AsLong(kobj2);
      int state;

      TYPE *t2 = reinterpret_cast<TYPE*>(sipConvertToType(tobj2, sipType_TYPE, sipTransferObj,SIP_NOT_NONE,&state,sipIsErr));

      if (*sipIsErr)
      {
        sipReleaseType(t2, sipType_TYPE, state);
        delete qm;
        return 0;
      }

      qm2.insert(k2, *t2);
      sipReleaseType(t2, sipType_TYPE, state);
    }
    qm->insert(k, qm2);
  }

  *sipCppPtr = qm;
  return sipGetState(sipTransferObj);
%End
};


//
// copied from PyQt4 QMap<int, TYPE> and adapted to qint64
//

// QMap<qint64, TYPE> is implemented as a Python dictionary.
template<TYPE>
%MappedType QMap<qint64, TYPE>
{
%TypeHeaderCode
#include <qmap.h>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<qint64, TYPE>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        TYPE *t = new TYPE(i.value());

        PyObject *kobj = PyLong_FromLongLong(i.key());
        //PyObject *kobj = SIPLong_FromLong(i.key());
        PyObject *tobj = sipConvertFromNewType(t, sipType_TYPE, sipTransferObj);

        if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
        {
            Py_DECREF(d);

            if (kobj)
            {
                Py_DECREF(kobj);
            }

            if (tobj)
            {
                Py_DECREF(tobj);
            }
            else
            {
                delete t;
            }

            return NULL;
        }

        Py_DECREF(kobj);
        Py_DECREF(tobj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *kobj, *tobj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &kobj, &tobj))
            if (!sipCanConvertToType(tobj, sipType_TYPE, SIP_NOT_NONE))
                return 0;

        return 1;
    }

    QMap<qint64, TYPE> *qm = new QMap<qint64, TYPE>;

    while (PyDict_Next(sipPy, &i, &kobj, &tobj))
    {
        int state;
        //, k = SIPLong_AsLong(kobj);
        qint64 k = PyLong_AsLongLong(kobj);
        TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(tobj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t, sipType_TYPE, state);

            delete qm;
            return 0;
        }

        qm->insert(k, *t);

        sipReleaseType(t, sipType_TYPE, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

template<TYPE>
%MappedType QList<QMap<int, TYPE>>
{
%TypeHeaderCode
#include <qmap.h>
#include <qlist.h>
%End

%ConvertFromTypeCode

    // Create the list
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return NULL;

    // Set the list elements.
    QList<QMap<int, TYPE>>::const_iterator it = sipCpp->constBegin();
    int i = 0;
    while (it != sipCpp->constEnd())
    {
        // Create the dictionary.
        PyObject *d = PyDict_New();
        if (!d)
        {
            Py_DECREF(l);
            return NULL;
        }

        QMap<int, TYPE>::const_iterator mapIt = it->constBegin();
        while (mapIt != it->constEnd())
        {
            TYPE *t = new TYPE(mapIt.value());

            PyObject *kobj = PyLong_FromLong(mapIt.key());
            PyObject *tobj = sipConvertFromNewType(t, sipType_TYPE, sipTransferObj);

            if (kobj == NULL || tobj == NULL || PyDict_SetItem(d, kobj, tobj) < 0)
            {
                Py_DECREF(d);

                if (kobj)
                {
                    Py_DECREF(kobj);
                }

                if (tobj)
                {
                    Py_DECREF(tobj);
                }
                else
                {
                    delete t;
                }

                Py_DECREF(l);
                return NULL;
            }

            Py_DECREF(kobj);
            Py_DECREF(tobj);

            ++mapIt;
        }

        PyList_SET_ITEM(l, i, d);
        ++i;
        ++it;
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *kobj, *tobj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyList_Check(sipPy))
          return 0;

        for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
        {
            if (!PyDict_Check(PyList_GET_ITEM(sipPy, i)))
                return 0;

            Py_ssize_t j = 0;
            while (PyDict_Next(PyList_GET_ITEM(sipPy, i), &j, &kobj, &tobj))
                if (!sipCanConvertToType(tobj, sipType_TYPE, SIP_NOT_NONE))
                    return 0;
        }

        return 1;
    }

    QList< QMap<int, TYPE> >* ql = new QList< QMap<int, TYPE> >;
    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
        Py_ssize_t j = 0;
        QMap<int, TYPE> qm;
        while (PyDict_Next(PyList_GET_ITEM(sipPy, i), &j, &kobj, &tobj))
        {
            int state;
            int k = PyLong_AsLong(kobj);
            TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(tobj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

            if (*sipIsErr)
            {
                sipReleaseType(t, sipType_TYPE, state);

                delete ql;
                return 0;
            }

            qm.insert(k, *t);
            sipReleaseType(t, sipType_TYPE, state);
        }
        ql->append( qm );
    }

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


%MappedType QMap<QString, int>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QString, int>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        QString *t1 = new QString(i.key());

        PyObject *t1obj = sipConvertFromNewType(t1, sipType_QString, sipTransferObj);
        PyObject *t2obj = PyLong_FromLong( (long) i.value() );

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

	    if (t1obj) {
	      Py_DECREF(t1obj);
	    } else {
	      delete t1;
	    }

	    if (t2obj) {
	      Py_DECREF(t2obj);
	    }

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_QString, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<QString, int> *qm = new QMap<QString, int>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(t1obj, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
        int t2 = PyLong_AsLong(t1obj);

        if (*sipIsErr)
        {
            sipReleaseType(t1, sipType_QString, state);
            delete qm;
            return 0;
        }

        qm->insert(*t1, t2);

        sipReleaseType(t1, sipType_QString, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};



template<TYPE1, TYPE2>
%MappedType QMap<TYPE1, TYPE2*>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<TYPE1, TYPE2*>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        TYPE1 *t1 = new TYPE1(i.key());
        TYPE2 *t2 = i.value();

        PyObject *t1obj = sipConvertFromNewType(t1, sipType_TYPE1, sipTransferObj);
        PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);
            else
                delete t1;

            if (t2obj)
                Py_DECREF(t2obj);
            else
                delete t2;

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_TYPE1, SIP_NOT_NONE))
                return 0;

            if (!sipCanConvertToType(t2obj, sipType_TYPE2, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<TYPE1, TYPE2*> *qm = new QMap<TYPE1, TYPE2*>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state1, state2;

        TYPE1 *t1 = reinterpret_cast<TYPE1 *>(sipConvertToType(t1obj, sipType_TYPE1, sipTransferObj, SIP_NOT_NONE, &state1, sipIsErr));
        TYPE2 *t2 = reinterpret_cast<TYPE2 *>(sipConvertToType(t2obj, sipType_TYPE2, sipTransferObj, SIP_NOT_NONE, &state2, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t1, sipType_TYPE1, state1);
            sipReleaseType(t2, sipType_TYPE2, state2);

            delete qm;
            return 0;
        }

        qm->insert(*t1, t2);

        sipReleaseType(t1, sipType_TYPE1, state1);
        sipReleaseType(t2, sipType_TYPE2, state2);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


%MappedType QMap<QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength>
/TypeHint="Dict[QgsFieldConstraints.Constraint, QgsFieldConstraints.ConstraintStrength]", TypeHintValue="{}"/
{
%TypeHeaderCode
#include <QMap>
#include "qgsfieldconstraints.h"
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength>::const_iterator i = sipCpp->constBegin();

    while (i != sipCpp->constEnd())
    {
        PyObject *kobj = sipConvertFromEnum(static_cast<int>(i.key()),
                                            sipType_QgsFieldConstraints_Constraint);
        PyObject *vobj = sipConvertFromEnum(static_cast<int>(i.value()),
                                            sipType_QgsFieldConstraints_ConstraintStrength);

        if (kobj == NULL || vobj == NULL || PyDict_SetItem(d, kobj, vobj) < 0)
        {
            Py_DECREF(d);

            if (kobj)
                Py_DECREF(kobj);

            if (vobj)
                Py_DECREF(vobj);

            return NULL;
        }

        Py_DECREF(kobj);
        Py_DECREF(vobj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *key, *value;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        return PyDict_Check(sipPy);
    }

    // Create a new QMap to hold the values
    QMap<QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength> *map = new QMap<QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength>;

    // Loop over the Python dictionary items
    while (PyDict_Next(sipPy, &i, &key, &value))
    {
        // Convert the key and value Python objects to C++ types
        int keyVal = sipConvertToEnum(key, sipType_QgsFieldConstraints_Constraint);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                         "a key has type '%s' but 'QgsFieldConstraints.Constraint' is expected",
                         sipPyTypeName(Py_TYPE(key)));

            delete map;
            *sipIsErr = 1;

            return 0;
        }

        int valueVal = sipConvertToEnum(value, sipType_QgsFieldConstraints_ConstraintStrength);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                         "a value has type '%s' but 'QgsFieldConstraints.ConstraintStrength' is expected",
                         sipPyTypeName(Py_TYPE(value)));

            delete map;
            *sipIsErr = 1;

            return 0;
        }

        // Add the key and value to the map
        map->insert(static_cast<QgsFieldConstraints::Constraint>(keyVal), static_cast<QgsFieldConstraints::ConstraintStrength>(valueVal));
    }

    *sipCppPtr = map;

    return sipGetState(sipTransferObj);
%End
};


template<double, TYPE>
%MappedType QMap<double, TYPE>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<double, TYPE>::iterator i;

    for (i = sipCpp->begin(); i != sipCpp->end(); ++i)
    {
        PyObject *t1obj = PyFloat_FromDouble(i.key());
        TYPE *t2 = &i.value();
        PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE, sipTransferObj);

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);

            if (t2obj)
                Py_DECREF(t2obj);

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);
   }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!PyFloat_Check(t1obj))
                return 0;

            if (!sipCanConvertToType(t2obj, sipType_TYPE, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<double, TYPE> *qm = new QMap<double, TYPE>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        double k = PyFloat_AsDouble(t1obj);
        TYPE *t2 = reinterpret_cast<TYPE *>(sipConvertToType(t2obj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t2, sipType_TYPE, state);
            delete qm;
            return 0;
        }

        qm->insert(k, *t2);

        sipReleaseType(t2, sipType_TYPE, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


%MappedType QHash<double, double>
{
%TypeHeaderCode
#include <QHash>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QHash<double, double>::iterator i;

    for (i = sipCpp->begin(); i != sipCpp->end(); ++i)
    {
        PyObject *t1obj = PyFloat_FromDouble(i.key());
        PyObject *t2obj = PyFloat_FromDouble(i.value());

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);

            if (t2obj)
                Py_DECREF(t2obj);

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);
   }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!PyFloat_Check(t1obj))
                return 0;

           if (!PyFloat_Check(t2obj))
                return 0;
        }

        return 1;
    }

    QHash<double, double> *qm = new QHash<double, double>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        double k = PyFloat_AsDouble(t1obj);
        double v = PyFloat_AsDouble(t2obj);

        qm->insert(k, v);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QMap<double, double>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<double, double>::iterator i;

    for (i = sipCpp->begin(); i != sipCpp->end(); ++i)
    {
        PyObject *t1obj = PyFloat_FromDouble(i.key());
        PyObject *t2obj = PyFloat_FromDouble(i.value());

        if (t1obj == NULL || t2obj == NULL || PyDict_SetItem(d, t1obj, t2obj) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);

            if (t2obj)
                Py_DECREF(t2obj);

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2obj);
   }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!PyFloat_Check(t1obj))
                return 0;

           if (!PyFloat_Check(t2obj))
                return 0;
        }

        return 1;
    }

    QMap<double, double> *qm = new QMap<double, double>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        double k = PyFloat_AsDouble(t1obj);
        double v = PyFloat_AsDouble(t2obj);

        qm->insert(k, v);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

template<TYPE2>
%MappedType QMap<QString, QList<TYPE2> >
{
%TypeHeaderCode
#include <QMap>
#include <QList>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMap<QString, QList< TYPE2 > >::const_iterator i;

    for (i = sipCpp->constBegin(); i != sipCpp->constEnd(); ++i)
    {
        QString *t1 = new QString(i.key());
        PyObject *t1obj = sipConvertFromNewType(t1, sipType_QString, sipTransferObj);

        // build list for dictionary value
        QList< TYPE2 > sourceList = i.value();
        PyObject *t2list = PyList_New( sourceList.size() );
        if ( t2list )
        {
            for ( int j = 0; j < sourceList.size(); j++ )
            {
                TYPE2 *t = new TYPE2(sourceList.at(j));
                PyObject *lobj = sipConvertFromNewType(t, sipType_TYPE2, sipTransferObj);
                PyList_SetItem( t2list, j, lobj );
            }
        }

        if (t1obj == NULL || t2list == NULL || PyDict_SetItem(d, t1obj, t2list) < 0)
        {
            Py_DECREF(d);

            if (t1obj)
                Py_DECREF(t1obj);

            if (t2list)
                Py_DECREF(t2list);

            return NULL;
        }

        Py_DECREF(t1obj);
        Py_DECREF(t2list);
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;


    const sipTypeDef *qlist_type = sipFindType("QList<TYPE2>");


    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
            if (!sipCanConvertToType(t1obj, sipType_QString, SIP_NOT_NONE))
                return 0;
        }

        return 1;
    }

    QMap<QString, QList< TYPE2 > > *qm = new QMap<QString, QList< TYPE2 > >;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;

        QString *t1 = reinterpret_cast<QString *>(sipConvertToType(t1obj, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    QList<TYPE2> *t2 = reinterpret_cast< QList<TYPE2> * >(sipConvertToType(t2obj, qlist_type, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
        if (*sipIsErr)
        {
            sipReleaseType(t2, sipType_TYPE2, state);
            delete qm;
            return 0;
        }

        if ( t2 )
          qm->insert(*t1, *t2);
        else
          qm->insert(*t1, QList<TYPE2>() );

        sipReleaseType(t1, sipType_QString, state);

        sipReleaseType(t2, sipType_TYPE2, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

template<double, TYPE2>
%MappedType QMultiMap<double, TYPE2>
{
%TypeHeaderCode
#include <QMultiMap>
%End

%ConvertFromTypeCode
    // Create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
        return NULL;

    // Set the dictionary elements.
    QMultiMap<double, TYPE2>::iterator i = sipCpp->begin();

    while (i != sipCpp->end())
    {

      const double t1 = i.key();
      TYPE2 *t2 = &i.value();
      PyObject *t1obj = PyFloat_FromDouble(t1);
      PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);
      if (PyDict_GetItem(d, t1obj) == NULL) {
       PyObject *lst = PyList_New(0);
       PyDict_SetItem(d, t1obj, lst);
       if (lst)
       {
         Py_DECREF(lst);
       }
      }

      if (t1obj == NULL || t2obj == NULL ||
         PyList_Append(PyDict_GetItem(d, t1obj), t2obj) < 0)
        {
         Py_DECREF(d);
         if (t1obj)
         {
           Py_DECREF(t1obj);
         }

         if (t2obj)
         {
           Py_DECREF(t2obj);
         }

         return NULL;
        }
        Py_DECREF(t1obj);
        Py_DECREF(t2obj);

        ++i;
    }

    return d;
%End

%ConvertToTypeCode
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
         for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i) {
           if (!sipCanConvertToType(PyList_GET_ITEM(t2obj, i),
                                        sipType_TYPE2, SIP_NOT_NONE))
             return 0;
         }
        }

        return 1;
    }

    QMultiMap<double, TYPE2> *qm = new QMultiMap<double, TYPE2>;
    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state2;
       double k = PyFloat_AsDouble(t1obj);
       for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i) {
         TYPE2 *t2 =
           reinterpret_cast<TYPE2 *>(sipConvertToType(PyList_GET_ITEM(t2obj, i),
                                                          sipType_TYPE2,
                                                          sipTransferObj,
                                                          SIP_NOT_NONE,
                                                          &state2,
                                                          sipIsErr));

         if (*sipIsErr)
           {
             sipReleaseType(t2, sipType_TYPE2, state2);

             delete qm;
             return 0;
           }

         qm->insert(k, *t2);

         sipReleaseType(t2, sipType_TYPE2, state2);
       }
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


template<int, TYPE2*>
%MappedType QMultiMap<int, TYPE2*>
{
%TypeHeaderCode
#include <QMultiMap>
%End

%ConvertFromTypeCode
    // Convert to Python: create the dictionary.
    PyObject *d = PyDict_New();

    if (!d)
    {
        return NULL;
    }

    // Set the dictionary elements.
    QMultiMap<int, TYPE2*>::iterator i = sipCpp->begin();

    while (i != sipCpp->end())
    {

      const int t1 = i.key();
      TYPE2 *t2 = i.value();
      PyObject *t1obj = PyLong_FromSize_t(t1);
      PyObject *t2obj = sipConvertFromType(t2, sipType_TYPE2, sipTransferObj);
      if (PyDict_GetItem(d, t1obj) == NULL)
      {
        PyObject *lst = PyList_New(0);
        PyDict_SetItem(d, t1obj, lst);
        if (lst)
        {
           Py_DECREF(lst);
        }
      }

      if (t1obj == NULL || t2obj == NULL ||
         PyList_Append(PyDict_GetItem(d, t1obj), t2obj) < 0)
      {
         Py_DECREF(d);
         if (t1obj)
         {
           Py_DECREF(t1obj);
         }

         if (t2obj)
         {
           Py_DECREF(t2obj);
         }

         return NULL;
       }
       Py_DECREF(t1obj);
       Py_DECREF(t2obj);

       ++i;
    }

    return d;
%End

%ConvertToTypeCode
    // Convert from Python:
    PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
        if (!PyDict_Check(sipPy))
            return 0;

        while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
        {
         for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i) {
           if (!sipCanConvertToType(PyList_GET_ITEM(t2obj, i),
                                        sipType_TYPE2, SIP_NOT_NONE))
             return 0;
         }
        }

        return 1;
    }

    QMultiMap<int, TYPE2*> *qm = new QMultiMap<int, TYPE2*>;
    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
       int state2;
       int k = (int) PyLong_AsLong(t1obj);
       for (int i = 0; i < PyList_GET_SIZE(t2obj); ++i)
       {
           TYPE2 *t2 =
           reinterpret_cast<TYPE2 *>(sipConvertToType(PyList_GET_ITEM(t2obj, i),
                                                          sipType_TYPE2,
                                                          sipTransferObj,
                                                          SIP_NOT_NONE,
                                                          &state2,
                                                          sipIsErr));

         if (*sipIsErr)
         {
             sipReleaseType(t2, sipType_TYPE2, state2);

             delete qm;
             return 0;
         }

         qm->insert(k, t2);

         sipReleaseType(t2, sipType_TYPE2, state2);
       }
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};


template <TYPE>
%MappedType QMap< QPair< QString, QString>, TYPE >
{
%TypeHeaderCode
#include <QPair>
#include <QMap>
%End

%ConvertFromTypeCode
//convert map to a python dictionary
  PyObject *d;

  if ( ( d = PyDict_New() ) == NULL )
    return NULL;

  for ( auto it = sipCpp->constBegin(); it != sipCpp->constEnd(); ++ it )
  {
    PyObject *keyobj;
    if ( ( keyobj = PyTuple_New( 2 ) ) == NULL )
    {
      Py_DECREF( d );
      return NULL;
    }

    TYPE *t = new TYPE(it.value());
    PyObject *tobj = sipConvertFromNewType(t, sipType_TYPE, sipTransferObj);
    if ( tobj == NULL )
    {
        Py_DECREF(d);
        delete t;
        return NULL;
    }

    // build key
    PyObject *k1obj = sipConvertFromNewType( new QString( it.key().first ), sipType_QString, sipTransferObj );
    PyTuple_SetItem( keyobj, 0, k1obj );
    PyObject *k2obj = sipConvertFromNewType( new QString( it.key().second ), sipType_QString, sipTransferObj );
    PyTuple_SetItem( keyobj, 1, k2obj );

    if(keyobj == NULL || tobj == NULL || PyDict_SetItem(d, keyobj, tobj) < 0)
    {
        Py_DECREF(d);

        if (keyobj)
        {
            Py_DECREF(keyobj);
        }

        if (tobj)
        {
            Py_DECREF(tobj);
        }
        return NULL;
      }

    Py_DECREF(keyobj);
    Py_DECREF(tobj);
  }

  return d;
%End

%ConvertToTypeCode
  Py_ssize_t i = 0;
  PyObject *kobj, *tobj;

  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
      if (!PyDict_Check(sipPy))
          return 0;

      while (PyDict_Next(sipPy, &i, &kobj, &tobj))
          if (!sipCanConvertToType(tobj, sipType_TYPE, SIP_NOT_NONE))
              return 0;

      return 1;
  }

  PyObject *t1obj, *t2obj;
  QMap< QPair< QString, QString>, TYPE > *qm = new QMap< QPair< QString, QString>, TYPE >;

  int state;
  while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
  {
      PyObject *sipKeyFirst  = PyTuple_GetItem( t1obj, 0 );
      PyObject *sipKeySecond = PyTuple_GetItem( t1obj, 1 );
      QString *k1 = reinterpret_cast<QString *>(sipConvertToType(sipKeyFirst, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      if (*sipIsErr)
      {
          sipReleaseType(k1, sipType_QString, state);
          delete qm;
          return 0;
      }

      QString *k2 = reinterpret_cast<QString *>(sipConvertToType(sipKeySecond, sipType_QString, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      if (*sipIsErr)
      {
          sipReleaseType(k1, sipType_QString, state);
          sipReleaseType(k2, sipType_QString, state);
          delete qm;
          return 0;
      }

      TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(t2obj, sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
      if (*sipIsErr)
      {
          sipReleaseType(t, sipType_TYPE, state);

          delete qm;
          return 0;
      }

      qm->insert( qMakePair( *k1,*k2 ), *t );
      sipReleaseType(k1, sipType_QString, state);
      sipReleaseType(k2, sipType_QString, state);
      sipReleaseType(t, sipType_TYPE, state);
  }

  *sipCppPtr = qm;

  return sipGetState( sipTransferObj );
%End
};

%If (VECTOR_MAPPED_TYPE)
template <TYPE>
%MappedType QVector< TYPE* >
{
%TypeHeaderCode
#include <QVector>
%End

%ConvertFromTypeCode
    // Create the list
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return NULL;

    // Set the dictionary elements.
    for( int i = 0; i < sipCpp->size(); i++ )
    {
        TYPE *t = sipCpp->at(i);
        PyObject *tobj = sipConvertFromType(t, sipType_TYPE, sipTransferObj);

        if (tobj == NULL || PyList_SetItem(l, i, tobj) < 0)
        {
            Py_DECREF(tobj);
            Py_DECREF(l);
            return NULL;
        }
    }

    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
      if (!PyList_Check(sipPy))
        return 0;

      for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      {
        if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, SIP_NOT_NONE))
          return 0;
      }

      return 1;
    }

    QVector<TYPE*> *v = new QVector<TYPE*>( PyList_GET_SIZE(sipPy) );

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
      int state;

      TYPE *t = reinterpret_cast<TYPE *>(sipConvertToType(PyList_GET_ITEM(sipPy, i), sipType_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

      if (*sipIsErr)
      {
        sipReleaseType(t, sipType_TYPE, state);
        delete v;
        return 0;
      }

      v->replace( i, t );

      sipReleaseType(t, sipType_TYPE, state);
    }

    *sipCppPtr = v;

    return sipGetState(sipTransferObj);
%End
};
%End


%MappedType QPointer< QgsMapLayer >
{
%TypeHeaderCode
#include <QPointer>
%End

%ConvertFromTypeCode
    QgsMapLayer *t = sipCpp->data();
    PyObject *tobj = sipConvertFromType(t, sipType_QgsMapLayer, sipTransferObj);
    if (tobj == NULL)
    {
        Py_DECREF(tobj);
        return NULL;
    }
    return tobj;
%End

%ConvertToTypeCode
    int state;

    QgsMapLayer *t = reinterpret_cast<QgsMapLayer *>(sipConvertToType(sipPy, sipType_QgsMapLayer, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseType(t, sipType_QgsMapLayer, state);
      return 0;
    }
    QPointer<QgsMapLayer> *v = new QPointer<QgsMapLayer>( t );
    sipReleaseType(t, sipType_QgsMapLayer, state);

    *sipCppPtr = v;

    return sipGetState(sipTransferObj);
%End
};


%MappedType QMap<qint64, QgsFeature*>
{
%TypeHeaderCode
#include <QMap>
%End

%ConvertFromTypeCode

//convert map to a python dictionary
    PyObject *d;

    if ((d = PyDict_New()) == NULL)
      return NULL;

    for (QMap<qint64, QgsFeature*>::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it)
    {
        QgsFeature *oobj = new QgsFeature(*it.value());

        PyObject *keyobj = PyLong_FromLong(it.key());
        PyObject *pyOobj = sipConvertFromType(oobj, sipType_QgsFeature, sipTransferObj);
        if(pyOobj == NULL || keyobj == NULL || PyDict_SetItem(d, keyobj, pyOobj) < 0)
        {
            Py_DECREF(d);

            if (pyOobj)
            {
                Py_DECREF(pyOobj);
            }
            else
            {
                delete oobj;
            }

            if (keyobj)
            {
                Py_DECREF(keyobj);
            }
            return NULL;
          }
        Py_DECREF(pyOobj);
        Py_DECREF(keyobj);
        }
    return d;

%End

%ConvertToTypeCode
 PyObject *t1obj, *t2obj;
    Py_ssize_t i = 0;

    QMap<qint64, QgsFeature*> *qm = new QMap<qint64, QgsFeature*>;

    while (PyDict_Next(sipPy, &i, &t1obj, &t2obj))
    {
        int state;
        qint64 t1 = PyLong_AsLongLong(t1obj);
        QgsFeature *t2 = reinterpret_cast<QgsFeature*>(sipConvertToType(t2obj, sipType_QgsFeature, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            sipReleaseType(t2, sipType_QgsFeature, state);
            delete qm;
            return 0;
        }

        qm->insert(t1, t2);

        sipReleaseType(t2, sipType_QgsFeature, state);
    }

    *sipCppPtr = qm;

    return sipGetState(sipTransferObj);
%End
};

// QList<QgsField> is implemented as a Python list of QgsField.
%MappedType QList<QgsField>
{
%TypeHeaderCode
#include <qgsfield.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        QgsField *t = new QgsField(sipCpp->at(i));
        PyObject *tobj = sipConvertFromNewType(t, sipType_QgsField, sipTransferObj);

        if (!tobj)
        {
            delete t;
            Py_DECREF(l);

            return 0;
        }

        PyList_SET_ITEM(l, i, tobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        Py_XDECREF(iter);

        return (iter
#if PY_MAJOR_VERSION < 3
                && !PyString_Check(sipPy)
#endif
                && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<QgsField> *ql = new QList<QgsField>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int state;
        QgsField *t = reinterpret_cast<QgsField *>( sipForceConvertToType(itm, sipType_QgsField, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            PyErr_Format(PyExc_TypeError,
                    "index %ld has type '%s' but 'QgsField' is expected",
                    (long) i, Py_TYPE(itm)->tp_name);

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);

            return 0;
        }

        ql->append(*t);

        sipReleaseType(t, sipType_QgsField, state);
        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%ModuleCode
bool null_from_qvariant_converter( const QVariant *varp, PyObject **objp )
{
  static bool sWatchDog = false;

  if ( sWatchDog )
    return false;

  // If we deal with a NULL QVariant (and it's not a QByteArray which properly
  // maps NULL values)
  // If there are more cases like QByteArray, we should consider using a allowlist
  // instead of a blocklist.

  // Note that we force silence the usual "storing a null ... in a qvariant" warnings here,
  // as we don't have any control over sip storing empty objects in a valid qvariant.
  if ( QgsVariantUtils::isNull( *varp, true )
       && varp->type() != QVariant::ByteArray
       && varp->type() != QMetaType::VoidStar
       && varp->type() != QMetaType::Nullptr
       && varp->type() != QMetaType::QObjectStar )
  {
    if ( varp->type() == QVariant::UserType
         && varp->userType() == QMetaType::type("QgsLayoutItem*") )
    {
      return false;
    }

    sWatchDog = true;
    Py_INCREF(Py_None);
    *objp = Py_None;
    sWatchDog = false;
    return true;
  }
  else
  {
    return false;
  }
}
%End

%ModuleHeaderCode
#define SIP_PYQT_FROM_QVARIANT_BY_TYPE "pyqt6_from_qvariant_by_type"
#include "qgsvariantutils.h"
%End

// In Qt6 QVector is just an alias to QList but doesn't appear in pyqt
// Mapping code is directly copied from QList mapping code in qpycore_qlist.sip and
// std::pair mapping code in qpycore_std_pair

template<_TYPE_>
%MappedType QVector<_TYPE_>
        /TypeHintIn="Iterable[_TYPE_]", TypeHintOut="List[_TYPE_]",
        TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qlist.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        _TYPE_ *t = new _TYPE_(sipCpp->at(i));
        PyObject *tobj = sipConvertFromNewType(t, sipType__TYPE_,
                sipTransferObj);

        if (!tobj)
        {
            delete t;
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, tobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<_TYPE_> *ql = new QList<_TYPE_>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int state;
        _TYPE_ *t = reinterpret_cast<_TYPE_ *>(
                sipForceConvertToType(itm, sipType__TYPE_, sipTransferObj,
                        SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but '_TYPE_' is expected", i,
                    sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);

            return 0;
        }

        ql->append(*t);

        sipReleaseType(t, sipType__TYPE_, state);
        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};


template<_TYPE_>
%MappedType QVector<_TYPE_ *>
        /TypeHintIn="Iterable[_TYPE_]", TypeHintOut="List[_TYPE_]",
        TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qlist.h>
%End

%ConvertFromTypeCode
    int gc_enabled = sipEnableGC(0);
    PyObject *l = PyList_New(sipCpp->size());

    if (l)
    {
        for (int i = 0; i < sipCpp->size(); ++i)
        {
            _TYPE_ *t = sipCpp->at(i);

            // The explicit (void *) cast allows _TYPE_ to be const.
            PyObject *tobj = sipConvertFromType((void *)t, sipType__TYPE_,
                    sipTransferObj);

            if (!tobj)
            {
                Py_DECREF(l);
                l = 0;

                break;
            }

            PyList_SetItem(l, i, tobj);
        }
    }

    sipEnableGC(gc_enabled);

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<_TYPE_ *> *ql = new QList<_TYPE_ *>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        _TYPE_ *t = reinterpret_cast<_TYPE_ *>(
                sipForceConvertToType(itm, sipType__TYPE_, sipTransferObj, 0,
                        0, sipIsErr));

        if (*sipIsErr)
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but '_TYPE_' is expected", i,
                    sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);

            return 0;
        }

        ql->append(t);

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QVector<qreal>
        /TypeHintIn="Iterable[float]", TypeHintOut="List[float]",
        TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qlist.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *pobj = PyFloat_FromDouble(sipCpp->value(i));

        if (!pobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, pobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<qreal> *ql = new QList<qreal>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        PyErr_Clear();
        double val = PyFloat_AsDouble(itm);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'float' is expected", i,
                    sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(val);

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QVector<float>
        /TypeHintIn="Iterable[float]", TypeHintOut="List[float]",
        TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qlist.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *pobj = PyFloat_FromDouble(sipCpp->value(i));

        if (!pobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, pobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<float> *ql = new QList<float>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        PyErr_Clear();
        double val = PyFloat_AsDouble(itm);

        if (PyErr_Occurred())
        {
            PyErr_Format(PyExc_TypeError,
                    "index %zd has type '%s' but 'float' is expected", i,
                    sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(val);

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QVector<int>
        /TypeHintIn="Iterable[int]", TypeHintOut="List[int]",
        TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qlist.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    for (int i = 0; i < sipCpp->size(); ++i)
    {
        PyObject *pobj = PyLong_FromLong(sipCpp->value(i));

        if (!pobj)
        {
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, pobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<int> *ql = new QList<int>;

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int val = sipLong_AsInt(itm);

        if (PyErr_Occurred())
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError))
                PyErr_Format(PyExc_TypeError,
                        "index %zd has type '%s' but 'int' is expected", i,
                        sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);
            *sipIsErr = 1;

            return 0;
        }

        ql->append(val);

        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

template<_TYPE1_, _TYPE2_>
%MappedType QPair<_TYPE1_, _TYPE2_> /TypeHint="Tuple[_TYPE1_, _TYPE2_]"/
{
%TypeHeaderCode
#include <utility>
%End

%ConvertFromTypeCode
    _TYPE1_ *first = new _TYPE1_(sipCpp->first);
    _TYPE2_ *second = new _TYPE2_(sipCpp->second);
    PyObject *t = sipBuildResult(NULL, "(NN)", first, sipType__TYPE1_,
            sipTransferObj, second, sipType__TYPE2_, sipTransferObj);

    if (!t)
    {
        delete first;
        delete second;

        return 0;
    }

    return t;
%End

%ConvertToTypeCode
    if (!sipIsErr)
        return (PySequence_Check(sipPy) && !PyUnicode_Check(sipPy));

    Py_ssize_t len = PySequence_Size(sipPy);

    if (len != 2)
    {
        // A negative length should only be an internal error so let the
        // original exception stand.
        if (len >= 0)
            PyErr_Format(PyExc_TypeError,
                    "sequence has %zd elements but 2 elements are expected",
                    len);

        *sipIsErr = 1;

        return 0;
    }

    PyObject *firstobj = PySequence_GetItem(sipPy, 0);

    if (!firstobj)
    {
        *sipIsErr = 1;

        return 0;
    }

    int firststate;
    _TYPE1_ *first = reinterpret_cast<_TYPE1_ *>(
            sipForceConvertToType(firstobj, sipType__TYPE1_, sipTransferObj,
                    SIP_NOT_NONE, &firststate, sipIsErr));

    if (*sipIsErr)
    {
        PyErr_Format(PyExc_TypeError,
                "the first element has type '%s' but '_TYPE1_' is expected",
                sipPyTypeName(Py_TYPE(firstobj)));

        return 0;
    }

    PyObject *secondobj = PySequence_GetItem(sipPy, 1);

    if (!secondobj)
    {
        sipReleaseType(first, sipType__TYPE1_, firststate);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    int secondstate;
    _TYPE2_ *second = reinterpret_cast<_TYPE2_ *>(
            sipForceConvertToType(secondobj, sipType__TYPE2_, sipTransferObj,
                    SIP_NOT_NONE, &secondstate, sipIsErr));

    if (*sipIsErr)
    {
        PyErr_Format(PyExc_TypeError,
                "the second element has type '%s' but '_TYPE2_' is expected",
                sipPyTypeName(Py_TYPE(secondobj)));

        Py_DECREF(secondobj);
        sipReleaseType(first, sipType__TYPE1_, firststate);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    *sipCppPtr = new QPair<_TYPE1_, _TYPE2_>(*first, *second);

    sipReleaseType(second, sipType__TYPE2_, secondstate);
    Py_DECREF(secondobj);
    sipReleaseType(first, sipType__TYPE1_, firststate);
    Py_DECREF(firstobj);

    return sipGetState(sipTransferObj);
%End
};


template<_TYPE_>
%MappedType QPair<_TYPE_, int> /TypeHint="Tuple[_TYPE_, int]"/
{
%TypeHeaderCode
#include <utility>
%End

%ConvertFromTypeCode
    _TYPE_ *first = new _TYPE_(sipCpp->first);
    PyObject *t = sipBuildResult(NULL, "(Ni)", first, sipType__TYPE_,
            sipTransferObj, sipCpp->second);

    if (!t)
    {
        delete first;

        return 0;
    }

    return t;
%End

%ConvertToTypeCode
    if (!sipIsErr)
        return (PySequence_Check(sipPy) && !PyUnicode_Check(sipPy));

    Py_ssize_t len = PySequence_Size(sipPy);

    if (len != 2)
    {
        // A negative length should only be an internal error so let the
        // original exception stand.
        if (len >= 0)
            PyErr_Format(PyExc_TypeError,
                    "sequence has %zd elements but 2 elements are expected",
                    len);

        *sipIsErr = 1;

        return 0;
    }

    PyObject *firstobj = PySequence_GetItem(sipPy, 0);

    if (!firstobj)
    {
        *sipIsErr = 1;

        return 0;
    }

    int firststate;
    _TYPE_ *first = reinterpret_cast<_TYPE_ *>(
            sipForceConvertToType(firstobj, sipType__TYPE_, sipTransferObj,
                    SIP_NOT_NONE, &firststate, sipIsErr));

    if (*sipIsErr)
    {
        PyErr_Format(PyExc_TypeError,
                "the first element has type '%s' but '_TYPE_' is expected",
                sipPyTypeName(Py_TYPE(firstobj)));

        return 0;
    }

    PyObject *secondobj = PySequence_GetItem(sipPy, 1);

    if (!secondobj)
    {
        sipReleaseType(first, sipType__TYPE_, firststate);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    int second = sipLong_AsInt(secondobj);

    if (PyErr_Occurred())
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
            PyErr_Format(PyExc_TypeError,
                    "the second element has type '%s' but 'int' is expected",
                    sipPyTypeName(Py_TYPE(secondobj)));

        Py_DECREF(secondobj);
        sipReleaseType(first, sipType__TYPE_, firststate);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    *sipCppPtr = new QPair<_TYPE_, int>(*first, second);

    Py_DECREF(secondobj);
    sipReleaseType(first, sipType__TYPE_, firststate);
    Py_DECREF(firstobj);

    return sipGetState(sipTransferObj);
%End
};


%MappedType QPair<int, int> /TypeHint="Tuple[int, int]"/
{
%TypeHeaderCode
#include <utility>
%End

%ConvertFromTypeCode
    return Py_BuildValue("(ii)", sipCpp->first, sipCpp->second);
%End

%ConvertToTypeCode
    if (!sipIsErr)
        return (PySequence_Check(sipPy) && !PyUnicode_Check(sipPy));

    Py_ssize_t len = PySequence_Size(sipPy);

    if (len != 2)
    {
        // A negative length should only be an internal error so let the
        // original exception stand.
        if (len >= 0)
            PyErr_Format(PyExc_TypeError,
                    "sequence has %zd elements but 2 elements are expected",
                    len);

        *sipIsErr = 1;

        return 0;
    }

    PyObject *firstobj = PySequence_GetItem(sipPy, 0);

    if (!firstobj)
    {
        *sipIsErr = 1;

        return 0;
    }

    int first = sipLong_AsInt(firstobj);

    if (PyErr_Occurred())
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
            PyErr_Format(PyExc_TypeError,
                    "the first element has type '%s' but 'int' is expected",
                    sipPyTypeName(Py_TYPE(firstobj)));

        *sipIsErr = 1;

        return 0;
    }

    PyObject *secondobj = PySequence_GetItem(sipPy, 1);

    if (!secondobj)
    {
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    int second = sipLong_AsInt(secondobj);

    if (PyErr_Occurred())
    {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
            PyErr_Format(PyExc_TypeError,
                    "the second element has type '%s' but 'int' is expected",
                    sipPyTypeName(Py_TYPE(secondobj)));

        Py_DECREF(secondobj);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    *sipCppPtr = new QPair<int, int>(first, second);

    Py_DECREF(secondobj);
    Py_DECREF(firstobj);

    return sipGetState(sipTransferObj);
%End
};


%MappedType QPair<float, float> /TypeHint="Tuple[float, float]"/
{
%TypeHeaderCode
#include <utility>
%End

%ConvertFromTypeCode
    return Py_BuildValue("(ff)", sipCpp->first, sipCpp->second);
%End

%ConvertToTypeCode
    if (!sipIsErr)
        return (PySequence_Check(sipPy) && !PyUnicode_Check(sipPy));

    Py_ssize_t len = PySequence_Size(sipPy);

    if (len != 2)
    {
        // A negative length should only be an internal error so let the
        // original exception stand.
        if (len >= 0)
            PyErr_Format(PyExc_TypeError,
                    "sequence has %zd elements but 2 elements are expected",
                    len);

        *sipIsErr = 1;

        return 0;
    }

    PyObject *firstobj = PySequence_GetItem(sipPy, 0);

    if (!firstobj)
    {
        *sipIsErr = 1;

        return 0;
    }

    PyErr_Clear();
    double first = PyFloat_AsDouble(firstobj);

    if (PyErr_Occurred())
    {
        PyErr_Format(PyExc_TypeError,
                "the first element has type '%s' but 'float' is expected",
                sipPyTypeName(Py_TYPE(firstobj)));

        *sipIsErr = 1;

        return 0;
    }

    PyObject *secondobj = PySequence_GetItem(sipPy, 1);

    if (!secondobj)
    {
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    PyErr_Clear();
    double second = PyFloat_AsDouble(secondobj);

    if (PyErr_Occurred())
    {
        PyErr_Format(PyExc_TypeError,
                "the second element has type '%s' but 'float' is expected",
                sipPyTypeName(Py_TYPE(secondobj)));

        Py_DECREF(secondobj);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    *sipCppPtr = new QPair<float, float>(first, second);;

    Py_DECREF(secondobj);
    Py_DECREF(firstobj);

    return sipGetState(sipTransferObj);
%End
};

%MappedType QPair<qreal, qreal> /TypeHint="Tuple[float, float]"/
{
%TypeHeaderCode
#include <utility>
%End

%ConvertFromTypeCode
    return Py_BuildValue("(ff)", sipCpp->first, sipCpp->second);
%End

%ConvertToTypeCode
    if (!sipIsErr)
        return (PySequence_Check(sipPy) && !PyUnicode_Check(sipPy));

    Py_ssize_t len = PySequence_Size(sipPy);

    if (len != 2)
    {
        // A negative length should only be an internal error so let the
        // original exception stand.
        if (len >= 0)
            PyErr_Format(PyExc_TypeError,
                    "sequence has %zd elements but 2 elements are expected",
                    len);

        *sipIsErr = 1;

        return 0;
    }

    PyObject *firstobj = PySequence_GetItem(sipPy, 0);

    if (!firstobj)
    {
        *sipIsErr = 1;

        return 0;
    }

    PyErr_Clear();
    double first = PyFloat_AsDouble(firstobj);

    if (PyErr_Occurred())
    {
        PyErr_Format(PyExc_TypeError,
                "the first element has type '%s' but 'float' is expected",
                sipPyTypeName(Py_TYPE(firstobj)));

        *sipIsErr = 1;

        return 0;
    }

    PyObject *secondobj = PySequence_GetItem(sipPy, 1);

    if (!secondobj)
    {
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    PyErr_Clear();
    double second = PyFloat_AsDouble(secondobj);

    if (PyErr_Occurred())
    {
        PyErr_Format(PyExc_TypeError,
                "the second element has type '%s' but 'float' is expected",
                sipPyTypeName(Py_TYPE(secondobj)));

        Py_DECREF(secondobj);
        Py_DECREF(firstobj);
        *sipIsErr = 1;

        return 0;
    }

    *sipCppPtr = new QPair<qreal, qreal>(first, second);;

    Py_DECREF(secondobj);
    Py_DECREF(firstobj);

    return sipGetState(sipTransferObj);
%End
};

%MappedType QVector<QMap<QString, QVariant>>
    /TypeHintIn="Iterable[Dict[str, Any]]",
    TypeHintOut="Iterable[Dict[str, Any]]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qvector.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    const sipTypeDef *qvariantmap_type = sipFindType( "QMap<QString,QVariant>" );
    for (int i = 0; i < sipCpp->size(); ++i)
    {
        QVariantMap *t = new QVariantMap(sipCpp->at(i));
        PyObject *tobj = sipConvertFromNewType(t, qvariantmap_type,
                                               sipTransferObj);

        if (!tobj)
        {
            delete t;
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, tobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QVector<QVariantMap> *ql = new QVector<QVariantMap>;
    const sipTypeDef *qvariantmap_type = sipFindType( "QMap<QString,QVariant>" );

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int state;
        QVariantMap *t = reinterpret_cast<QVariantMap *>(
            sipForceConvertToType(itm, qvariantmap_type, sipTransferObj,
                                  SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            PyErr_Format(PyExc_TypeError,
                         "index %zd has type '%s' but 'Dict[str, Any]' is expected", i,
                         sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);

            return 0;
        }

        ql->append(*t);

        sipReleaseType(t, qvariantmap_type, state);
        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

%MappedType QList<QMap<QString, QVariant>>
        /TypeHintIn="Iterable[Dict[str, Any]]",
        TypeHintOut="Iterable[Dict[str, Any]]", TypeHintValue="[]"/
{
%TypeHeaderCode
#include <qlist.h>
%End

%ConvertFromTypeCode
    PyObject *l = PyList_New(sipCpp->size());

    if (!l)
        return 0;

    const sipTypeDef *qvariantmap_type = sipFindType( "QMap<QString,QVariant>" );
    for (int i = 0; i < sipCpp->size(); ++i)
    {
        QVariantMap *t = new QVariantMap(sipCpp->at(i));
        PyObject *tobj = sipConvertFromNewType(t, qvariantmap_type,
                                               sipTransferObj);

        if (!tobj)
        {
            delete t;
            Py_DECREF(l);

            return 0;
        }

        PyList_SetItem(l, i, tobj);
    }

    return l;
%End

%ConvertToTypeCode
    PyObject *iter = PyObject_GetIter(sipPy);

    if (!sipIsErr)
    {
        PyErr_Clear();
        Py_XDECREF(iter);

        return (iter && !PyBytes_Check(sipPy) && !PyUnicode_Check(sipPy));
    }

    if (!iter)
    {
        *sipIsErr = 1;

        return 0;
    }

    QList<QVariantMap> *ql = new QList<QVariantMap>;
    const sipTypeDef *qvariantmap_type = sipFindType( "QMap<QString,QVariant>" );

    for (Py_ssize_t i = 0; ; ++i)
    {
        PyErr_Clear();
        PyObject *itm = PyIter_Next(iter);

        if (!itm)
        {
            if (PyErr_Occurred())
            {
                delete ql;
                Py_DECREF(iter);
                *sipIsErr = 1;

                return 0;
            }

            break;
        }

        int state;
        QVariantMap *t = reinterpret_cast<QVariantMap *>(
            sipForceConvertToType(itm, qvariantmap_type, sipTransferObj,
                                  SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
            PyErr_Format(PyExc_TypeError,
                         "index %zd has type '%s' but 'Dict[str, Any]' is expected", i,
                         sipPyTypeName(Py_TYPE(itm)));

            Py_DECREF(itm);
            delete ql;
            Py_DECREF(iter);

            return 0;
        }

        ql->append(*t);

        sipReleaseType(t, qvariantmap_type, state);
        Py_DECREF(itm);
    }

    Py_DECREF(iter);

    *sipCppPtr = ql;

    return sipGetState(sipTransferObj);
%End
};

// QVariant::Type is obsolete, migrate to QMetaType
%MappedType QVariant::Type
{
%TypeHeaderCode
#include "qgsvariantutils.h"
%End
%ConvertFromTypeCode
    const QMetaType::Type metaType = QgsVariantUtils::variantTypeToMetaType( *sipCpp );
    const sipTypeDef *qmetatype_type = sipFindType( "QMetaType::Type" );
    PyObject *eobj = sipConvertFromEnum(metaType, qmetatype_type);
    if (!eobj)
    {
        return 0;
    }
    return eobj;
%End

%ConvertToTypeCode
    const sipTypeDef *qmetatype_type = sipFindType( "QMetaType::Type" );
    const QMetaType::Type metaType = static_cast<QMetaType::Type>( sipConvertToEnum( sipPy, qmetatype_type ) );
    if (sipIsErr == NULL)
    {
        return PyErr_Occurred() ? 0 : 1;
    }
    else if (PyErr_Occurred())
    {
        *sipIsErr = 1;
        return 0;
    }
    const QVariant::Type variantType = QgsVariantUtils::metaTypeToVariantType( metaType );
    *sipCppPtr = new QVariant::Type( variantType );
    return 1;
%End
};

%PostInitialisationCode  //#spellok

// Import the Chimera helper registration functions.
typedef bool ( *FromQVariantConverterFn )( const QVariant *, PyObject ** );
void (*register_from_qvariant_converter)(FromQVariantConverterFn);
register_from_qvariant_converter = (void (*)(FromQVariantConverterFn))sipImportSymbol("pyqt6_register_from_qvariant_convertor");  //#spellok
register_from_qvariant_converter(null_from_qvariant_converter);
%End
