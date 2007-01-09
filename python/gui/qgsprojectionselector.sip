

template <TYPE>
%MappedType QSet<TYPE>
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
  int i;
  for (QSet<TYPE>::iterator it = sipCpp->begin(); it != sipCpp->end(); ++it, ++i)
  {
    TYPE *t = new TYPE(*it);
    PyObject *tobj;

    if ((tobj = sipConvertFromNewInstance(t, sipClass_TYPE, sipTransferObj)) == NULL)
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
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, SIP_NOT_NONE))
        return 0;

    return 1;
  }

  QSet<TYPE> *qset = new QSet<TYPE>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    TYPE* t = reinterpret_cast<TYPE *>(sipConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseInstance(t, sipClass_TYPE, state);
      delete qset;
      return 0;
    }
    qset->insert(*t);
    sipReleaseInstance(t, sipClass_TYPE, state);
  }

  *sipCppPtr = qset;
  return sipGetState(sipTransferObj);
%End

};


/**
  @author Tim Sutton
  */
class QgsProjectionSelector: QWidget //, private Ui::QgsProjectionSelectorBase
{
%TypeHeaderCode
#include <qgsprojectionselector.h>
%End

    public:
      QgsProjectionSelector(QWidget* parent,
                            const char * name = "",
                            Qt::WFlags fl = 0);

      ~QgsProjectionSelector();

      //typedef QSet<QString> QStringSet;

      /**
       * \brief Populate the proj tree view with user defined projection names...
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \todo Should this be public?
       */
      void applyUserProjList(QSet<QString> * crsFilter = 0);

      /**
       * \brief Populate the proj tree view with system projection names...
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \todo Should this be public?
       */
      void applyProjList(QSet<QString> * crsFilter = 0);

      void updateProjAndEllipsoidAcronyms(int theSrsid, QString theProj4String);

      /*!
       * \brief Make the string safe for use in SQL statements.
       *  This involves escaping single quotes, double quotes, backslashes,
       *  and optionally, percentage symbols.  Percentage symbols are used
       *  as wildcards sometimes and so when using the string as part of the
       *  LIKE phrase of a select statement, should be escaped.
       * \arg const QString in The input string to make safe.
       * \return The string made safe for SQL statements.
       */
      const QString stringSQLSafe(const QString theSQL);

      //! Gets the current EPSG-style projection identifier
      long getCurrentEpsg();

    public slots:
      void setSelectedSRSName(QString theSRSName);

      QString getSelectedName();

      void setSelectedSRSID(long theSRSID);

      QString getCurrentProj4String();

      //! Gets the current PostGIS-style projection identifier
      long getCurrentSRID();

      //! Gets the current QGIS projection identfier
      long getCurrentSRSID();

      /**
       * \brief filters this widget by the given CRSs
       *
       * Sets this widget to filter the available projections to those listed
       * by the given Coordinate Reference Systems.
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \note This function only deals with EPSG labels only at this time.
       *
       * \warning This function's behaviour is undefined if it is called after the widget is shown.
       */
      void setOgcWmsCrsFilter(QSet<QString> crsFilter);

      void on_pbnFind_clicked();

    protected:
      /** Used to ensure the projection list view is actually populated */
      void showEvent ( QShowEvent * theEvent );

      /** Used to manage column sizes */
      void resizeEvent ( QResizeEvent * theEvent );

    signals:
      void sridSelected(QString theSRID);
      //! Refresh any listening canvases
      void refresh();
      //! Let listeners know if find has focus so they can adjust the default button
      void searchBoxHasFocus(bool);
};

