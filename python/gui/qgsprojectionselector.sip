


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
      long getSelectedEpsg();

    public slots:
      void setSelectedSRSName(QString theSRSName);

      QString getSelectedName();

      void setSelectedSRSID(long theSRSID);
      
      void setSelectedEpsg(long epsg);

      QString getSelectedProj4String();

      //! Gets the current PostGIS-style projection identifier
      long getSelectedSRID();

      //! Gets the current QGIS projection identfier
      long getSelectedSRSID();

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

