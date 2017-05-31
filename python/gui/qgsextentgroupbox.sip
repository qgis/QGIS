/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsextentgroupbox.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsExtentGroupBox : QgsCollapsibleGroupBox
{
%Docstring
 Collapsible group box for configuration of extent, typically for a save operation.

 Besides allowing the user to enter the extent manually, it comes with options to use
 original extent or extent defined by the current view in map canvas.

 When using the widget, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsextentgroupbox.h"
%End
  public:
    explicit QgsExtentGroupBox( QWidget *parent /TransferThis/ = 0 );

    enum ExtentState
    {
      OriginalExtent,
      CurrentExtent,
      UserExtent,
      ProjectLayerExtent,
    };

    void setOriginalExtent( const QgsRectangle &originalExtent, const QgsCoordinateReferenceSystem &originalCrs );
%Docstring
Setup original extent - should be called as part of initialization
%End

    QgsRectangle originalExtent() const;
%Docstring
 :rtype: QgsRectangle
%End
    const QgsCoordinateReferenceSystem &originalCrs() const;
%Docstring
 :rtype: QgsCoordinateReferenceSystem
%End

    void setCurrentExtent( const QgsRectangle &currentExtent, const QgsCoordinateReferenceSystem &currentCrs );
%Docstring
Setup current extent - should be called as part of initialization (or whenever current extent changes)
%End

    QgsRectangle currentExtent() const;
%Docstring
 :rtype: QgsRectangle
%End
    const QgsCoordinateReferenceSystem &currentCrs() const;
%Docstring
 :rtype: QgsCoordinateReferenceSystem
%End

    void setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs );
%Docstring
Should be called as part of initialization and whenever the the output CRS is changed
%End

    QgsRectangle outputExtent() const;
%Docstring
Get the resulting extent - in output CRS coordinates
 :rtype: QgsRectangle
%End

    QgsExtentGroupBox::ExtentState extentState() const;
%Docstring
 :rtype: QgsExtentGroupBox.ExtentState
%End

    void setTitleBase( const QString &title );
%Docstring
.. versionadded:: 2.12
%End
    QString titleBase() const;
%Docstring
.. versionadded:: 2.12
 :rtype: str
%End

  public slots:
    void setOutputExtentFromOriginal();
%Docstring
set output extent to be the same as original extent (may be transformed to output CRS)
%End

    void setOutputExtentFromCurrent();
%Docstring
set output extent to be the same as current extent (may be transformed to output CRS)
%End

    void setOutputExtentFromUser( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs );
%Docstring
set output extent to custom extent (may be transformed to output CRS)
%End

  signals:
    void extentChanged( const QgsRectangle &r );
%Docstring
emitted when extent is changed
%End

  protected slots:

    void on_mXMinLineEdit_textEdited( const QString & );
    void on_mXMaxLineEdit_textEdited( const QString & );
    void on_mYMinLineEdit_textEdited( const QString & );
    void on_mYMaxLineEdit_textEdited( const QString & );

    void groupBoxClicked();

  protected:
    void setOutputExtent( const QgsRectangle &r, const QgsCoordinateReferenceSystem &srcCrs, QgsExtentGroupBox::ExtentState state );
    void setOutputExtentFromLineEdit();
    void updateTitle();






};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsextentgroupbox.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
