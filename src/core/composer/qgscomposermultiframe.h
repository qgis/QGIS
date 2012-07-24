#ifndef QGSCOMPOSERMULTIFRAME_H
#define QGSCOMPOSERMULTIFRAME_H

#include <QObject>
#include <QSizeF>

class QgsComposerItem;
class QgsComposition;

/**Abstract base class for composer entries with the ability to distribute the content to several frames (items)*/
class QgsComposerMultiFrame: public QObject
{
    Q_OBJECT
    public:

        enum ResizeMode
        {
            ExtendToNextPage = 0, //duplicates last frame to next page to fit the total size
            UseExistingFrames //
        };

        QgsComposerMultiFrame( QgsComposition* c );
        virtual ~QgsComposerMultiFrame();
        virtual QSizeF totalSize() = 0;

    protected:
        QgsComposition* mComposition;
        QList<QgsComposerItem*> mFrameItems;

        void recalculateFrameSizes();

    private:
        QgsComposerMultiFrame(); //forbidden
};

#endif // QGSCOMPOSERMULTIFRAME_H
