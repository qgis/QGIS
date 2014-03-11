/**
 * Collapsible group box for configuration of extent, typically for a save operation.
 *
 * Besides allowing the user to enter the extent manually, it comes with options to use
 * original extent or extent defined by the current view in map canvas.
 *
 * When using the widget, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.
 *
 * @note added in 2.4
 */
class QgsExtentGroupBox : QgsCollapsibleGroupBox
{
%TypeHeaderCode
#include <qgsextentgroupbox.h>
%End
  public:
    explicit QgsExtentGroupBox( QWidget* parent = 0 );

    enum ExtentState
    {
      OriginalExtent,  //!<  layer's extent
      CurrentExtent,   //!<  map canvas extent
      UserExtent,      //!<  extent manually entered/modified by the user
    };

    //! Setup original extent - should be called as part of initialization
    void setOriginalExtent( const QgsRectangle& originalExtent, const QgsCoordinateReferenceSystem& originalCrs );

    QgsRectangle originalExtent() const;
    const QgsCoordinateReferenceSystem& originalCrs() const;

    //! Setup current extent - should be called as part of initialization (or whenever current extent changes)
    void setCurrentExtent( const QgsRectangle& currentExtent, const QgsCoordinateReferenceSystem& currentCrs );

    QgsRectangle currentExtent() const;
    const QgsCoordinateReferenceSystem& currentCrs() const;

    //! Set the output CRS - may need to be used for transformation from original/current extent.
    //! Should be called as part of initialization and whenever the the output CRS is changed
    void setOutputCrs( const QgsCoordinateReferenceSystem& outputCrs );

    //! Get the resulting extent - in output CRS coordinates
    QgsRectangle outputExtent() const;

    QgsExtentGroupBox::ExtentState extentState() const;

  public slots:
    //! set output extent to be the same as original extent (may be transformed to output CRS)
    void setOutputExtentFromOriginal();

    //! set output extent to be the same as current extent (may be transformed to output CRS)
    void setOutputExtentFromCurrent();

    //! set output extent to custom extent (may be transformed to outut CRS)
    void setOutputExtentFromUser( const QgsRectangle& extent, const QgsCoordinateReferenceSystem& crs );

  signals:
    //! emitted when extent is changed
    void extentChanged( const QgsRectangle& r );

  protected slots:

    void on_mXMinLineEdit_textEdited( const QString & );
    void on_mXMaxLineEdit_textEdited( const QString & );
    void on_mYMinLineEdit_textEdited( const QString & );
    void on_mYMaxLineEdit_textEdited( const QString & );

  protected:
    void setOutputExtent( const QgsRectangle& r, const QgsCoordinateReferenceSystem& srcCrs, QgsExtentGroupBox::ExtentState state );
    void setOutputExtentFromLineEdit();
    void updateTitle();
};
