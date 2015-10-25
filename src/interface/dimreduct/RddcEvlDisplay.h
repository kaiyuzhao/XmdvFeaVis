#ifndef RDDCEVLDISPLAY_H
#define RDDCEVLDISPLAY_H

#include <QGLWidget>

#include "data/dimreduct/DimRddtCluster.h"
#include "data/dimreduct/RddtClust.h"
#include "color/ColorManager.h"


class RddcEvlWidget;
class GLCanvas;
class QWidget;
//class RddtClustEvlDlg;
//class DimRddtCluster;

class RddcEvlDisplay :public QGLWidget{
    Q_OBJECT
public:
    RddcEvlDisplay(QWidget* parent);
    virtual ~RddcEvlDisplay();

    void setDimClust(DimRddtCluster* dimClust);
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

protected:
    // Overload these functions defined in QWidget
    // to enable the acceptance of mouse event
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent ( QMouseEvent * event );
private:
    void drawProfiles();
    void drawProfile(DataPartitions* partitionOfOneVar, int i, int size);

    ColorManager* cm;

    RddcEvlWidget* m_dimReductDlg;
    DimRddtCluster* m_rddtClust;
    GLCanvas* m_canvas;

//signals:

public slots:
    void refreshView();
};

#endif // RDDCEVLDISPLAY_H
