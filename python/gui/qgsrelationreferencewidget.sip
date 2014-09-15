
class QgsRelationReferenceWidget : QWidget
{
%TypeHeaderCode
#include <qgsrelationreferencewidget.h>
%End

  public:
    enum CanvasExtent
    {
      Fixed,
      Pan,
      Scale
    };

    explicit QgsRelationReferenceWidget( QWidget* parent );

    ~QgsRelationReferenceWidget();

    void setRelation( QgsRelation relation , bool allowNullValue );

    void setRelationEditable( bool editable );

    //! this sets the related feature using from the foreign key
    void setRelatedFeature( const QVariant &value );

    //! returns the related feature foreign key
    QVariant foreignKey();

    void setEditorContext( QgsAttributeEditorContext context, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

    bool embedForm() {return mEmbedForm;}
    void setEmbedForm( bool display );

    bool readOnlySelector() {return mReadOnlySelector;}
    void setReadOnlySelector( bool readOnly );

    bool allowMapIdentification() {return mAllowMapIdentification;}
    void setAllowMapIdentification( bool allowMapIdentification );

  protected:
    virtual void showEvent( QShowEvent* e );

    void init();

   signals:
    void foreignKeyChanged( QVariant );
};
