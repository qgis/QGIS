/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssearchquerybuilder.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSearchQueryBuilder : QDialog
{
%Docstring
 Query Builder for search strings
%End

%TypeHeaderCode
#include "qgssearchquerybuilder.h"
%End
  public:
    QgsSearchQueryBuilder( QgsVectorLayer *layer,
                           QWidget *parent /TransferThis/ = 0,
                           Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor - takes pointer to vector layer as a parameter
%End

    QString searchString();
%Docstring
returns newly created search string
 :rtype: str
%End

    void setSearchString( const QString &searchString );
%Docstring
change search string shown in text field
%End

  public slots:

    void saveQuery();
    void loadQuery();

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssearchquerybuilder.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
