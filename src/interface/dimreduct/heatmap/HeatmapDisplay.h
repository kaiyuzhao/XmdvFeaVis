/**
 * Kaiyu Jan 2013
 *
 */

#ifndef HEATMAPDISPLAY_H
#define HEATMAPDISPLAY_H
#include "main/XmdvTool.h"
#include "color/ColorManager.h"
#include "main/XmdvToolTypes.h"

#include <QGLWidget>
#include <vector>



class HeatmapClustCtrlWidget;
class GLCanvas;
class QWidget;
class DimRddtCluster;

/**
 *
 * @brief The HeatMapDisplay class
 * show redudencies in heatmap
 * allow users to select redudency groups
 *
 */
class HeatmapDisplay: public QGLWidget{
    Q_OBJECT
public:
    HeatmapDisplay(QWidget* parent, DimRddtCluster* dimClust);
    virtual ~HeatmapDisplay();

    //void setDimClust(DimRddtCluster* dimClust);
    ColorManager* cm;
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    //void paintEvent(QPaintEvent *event);
    //void resizeEvent(QResizeEvent *);
protected:
    // Overload these functions defined in QWidget
    // to enable the acceptance of mouse event
    void mouseReleaseEvent(QMouseEvent *m_event);
    void mousePressEvent(QMouseEvent *m_event);
    void mouseMoveEvent(QMouseEvent *m_event);
    void mouseDoubleClickEvent ( QMouseEvent * m_event);

private:
    HeatmapClustCtrlWidget* m_hmpClustCtrlDlg;

    DimRddtCluster* m_dimClust;

    GLCanvas* m_canvas;

    int m_currentX;

    int m_currentY;

    double m_currentGridSize;

    //draw variable names at the bottom of the m_canvas;
    void drawVarNames();

    //draw rect where the mouse is over;
    void drawHopOver(); //black;

    //draw the original heatmap;
    void drawHeatMap();

    //highlight the currently selected dimension x
    // and other dimensions belong to the group of x;
    void drawSelection();

    //return the horizontal grid of the cursor
    // on the heatmap grids;
    int whichHozPos(int x, int y);

    //which variable is the cursor over at; the returned value is the index on m_selected;
    //in another words the returned value is the index within a cluster, local index;
    int whichLocVar(int x, int y);

    //which global index is the cursor over at;
    int whichGlobVar(int x, int y);

    //vertical position of the mouse;
    int howManyNeighs(int x, int y);

    //actual value of the current color rect;
    double whatDistThres(int x, int y);

    //name of the grid mouse over;
    std::string whatName(int x, int y);

    //switch column selection on/off;
    void switchVisualSelection();

public slots:
    void refreshView();
};

#endif // HEATMAPDISPLAY_H
