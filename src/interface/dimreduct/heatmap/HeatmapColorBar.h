#ifndef HEATMAPCOLORBAR_H
#define HEATMAPCOLORBAR_H
#include "main/XmdvTool.h"
#include "color/ColorManager.h"

#include <QGLWidget>

#include "main/XmdvToolTypes.h"

class HeatmapClustCtrlWidget;
class GLCanvas;
class QWidget;
class DimRddtCluster;

class HeatmapColorBar : public QGLWidget
{
    Q_OBJECT
public:
    HeatmapColorBar(QWidget* parent, DimRddtCluster* dimClust);
    virtual ~HeatmapColorBar();

    ColorManager* cm;

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

private:
    DimRddtCluster* m_dimClust;
    GLCanvas* m_canvas;
    HeatmapClustCtrlWidget* m_hmpClustCtrlDlg;
    int m_numGrid;

    void drawColorBar();
signals:
    
public slots:
    void updateView();
};

#endif // HEATMAPCOLORBAR_H
