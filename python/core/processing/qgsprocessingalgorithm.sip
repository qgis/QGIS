class QgsProcessingAlgorithm
{
%TypeHeaderCode
#include <qgsprocessingalgorithm.h>
%End

  public:

    QgsProcessingAlgorithm();
    virtual ~QgsProcessingAlgorithm();

    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual QStringList tags() const;
    virtual QIcon icon() const;
    virtual QString svgIconPath() const;
    virtual QString group() const;

};

