#ifndef RDDCEVLDISPLAY_H
#define RDDCEVLDISPLAY_H

#include <QGLWidget>
#include "color/ColorManager.h"
#include "main/XmdvToolTypes.h"
#include <QMutex>

class DimRddtCluster;
class RddtClust;
class DataPartitions;
class RddcEvlWidget;
class GLCanvas;
class QWidget;
class QPoint;
class Vec2;
class ColorMap;

class RddcEvlDisplay :public QGLWidget{
    Q_OBJECT
public:
    RddcEvlDisplay(QWidget* parent, DimRddtCluster* dimClust);
    virtual ~RddcEvlDisplay();

    //void setDimClust(DimRddtCluster* dimClust);
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

protected:
    // Overload these functions defined in QWidget
    // to enable the acceptance of mouse event
    void mouseReleaseEvent(QMouseEvent *m_event);
    void mousePressEvent(QMouseEvent *m_event);
    void mouseMoveEvent(QMouseEvent *m_event);
    void mouseDoubleClickEvent ( QMouseEvent * m_event );
    //void mouseClickedEvent(QMouseEvent * event);
private:
    int whichGlyph(int x, int y);

    void drawProfiles();

    void drawHopOver();

    //draw the partition the user points at
    //also draw the variables that do not belong to this group in this partition;
    void drawLocalDetail();

    //idx is the index of focused variable (local idx);
    void drawProfile(DataPartitions* partitionOneVar, Vec2 pos, double w, double h, int idx);

    //display at most 10 glyphs per row;
    int m_cntOneRow;

    QMutex mutex;

    ColorManager* cm;

    ColorMap* colors;

    RddcEvlWidget* m_dimReductDlg;

    //operator
    DimRddtCluster* m_dimClust;

    //canvas
    GLCanvas* m_canvas;

    //the rectangles contain the histogram
    QRectVector m_profileRects;

    int m_x;
    int m_y;

    //dragging interaction;
    bool m_isMouseDragged;

    bool m_isMousePressed;

    int m_draggingGlyph; //which glyph is dragging;

    int m_glyIdxMouseOver; //which glyph is mouse over;

    int m_parIdxMouseOver; //which partition is active;

    //IntVector m_idxPartition;

    QPoint start;
    QPoint end;

    int startIdx;

    int endIdx;

public slots:
    void refreshView();
    void onDataFiltering();
    void onClearDataFiltering();
};

#endif // RDDCEVLDISPLAY_H
