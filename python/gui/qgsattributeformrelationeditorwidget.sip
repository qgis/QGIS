/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformrelationeditorwidget.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAttributeFormRelationEditorWidget : QgsAttributeFormWidget
{
%Docstring
*************************************************************************
qgsattributeformrelationeditorwidget.h
--------------------------------------
Date                 : Nov 2017
Copyright            : (C) 2017 Matthias Kuhn
Email                : matthias@opengis.ch
**************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgsattributeformrelationeditorwidget.h"
%End
  public:
    explicit QgsAttributeFormRelationEditorWidget( QgsRelationWidgetWrapper *wrapper, QgsAttributeForm *form );

    virtual void createSearchWidgetWrappers();
    virtual QString currentFilterExpression() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformrelationeditorwidget.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
