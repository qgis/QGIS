/***************************************************************************
  qgsmeshtransformcoordinatesdialog.h - QgsMeshTransformCoordinatesDialog

 ---------------------
 begin                : 26.8.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHTRANSFORMCOORDINATESDIALOG_H
#define QGSMESHTRANSFORMCOORDINATESDIALOG_H

#include "ui_qgsmeshtransformcoordinatesdialogbase.h"

#include "qgsexpressioncontextgenerator.h"
#include "qgsmeshadvancedediting.h"
#include "qgisapp.h"

class QgsMeshLayer;

/**
 * \brief A Dialog widget that is used to make some geometrical transformations of mesh vertices by expression
 *
 * \since QGIS 3.22
 */
class APP_EXPORT QgsMeshTransformCoordinatesDialog: public QDialog, public QgsExpressionContextGenerator, private Ui::QgsMeshTransformCoordinatesDialogBase
{
    Q_OBJECT
  public:

    //! Constructor
    QgsMeshTransformCoordinatesDialog( QWidget *parent );

    virtual QgsExpressionContext createExpressionContext() const override;

    //! Returns the vertex with index \a vertexIndex after calculation
    QgsMeshVertex transformedVertex( int vertexIndex );

    //! Returns whether the result of transformation is a valid mesh
    bool isResultValid() const;

    //! Returns cxhether the calculation has been done
    bool isCalculated() const;

  signals:
    //! Emitted when the calculation of the transform is done
    void calculationUpdated();

    //! Emitted just before the transform is applied
    void aboutToBeApplied();

    //! Emitted just after the transform is applied
    void applied();

  public slots:
    //! Set the vertices indexes to transform \vertexIndexes for the mesh \a layer
    void setInput( QgsMeshLayer *layer, const QList<int> &vertexIndexes );

  private slots:
    void calculate();
    void updateButton();
    void apply();
    void showHelp() const;

  private:
    QgsMeshTransformVerticesByExpression mTransformVertices;
    QgsMeshLayer *mInputLayer;
    QList<int> mInputVertices;
    bool mIsCalculated = false;
    bool mIsResultValid = false;
    QList<QgsExpressionLineEdit *> mExpressionLineEdits;
    QList<QCheckBox *> mCheckBoxes;

};

#endif // QGSMESHTRANSFORMCOORDINATESDIALOG_H
