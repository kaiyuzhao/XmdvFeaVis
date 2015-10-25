/*
 * ManualClustDisplay.h
 *
 *  Created on: Nov 28, 2012
 *      Author: kaiyuzhao
 */

#ifndef MANUALCLUSTDISPLAY_H_
#define MANUALCLUSTDISPLAY_H_

#include <QGLWidget>

#include "color/ColorManager.h"
#include "main/XmdvToolTypes.h"


class ManualClustCtrlWidget;
class DimRddtCluster;
class RddtClust;
class GLCanvas;

class ManualClustDisplay :public QGLWidget{
    Q_OBJECT
public:
    ManualClustDisplay(QWidget *parent, DimRddtCluster* dimClust);
	virtual ~ManualClustDisplay();

    ColorManager* cm;
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

private:

    QRectVector m_rects;

    QTimer* m_timer;

    //int m_event_x;

    //int m_event_y;

    // The pointer to the dimension reduction dialog
    ManualClustCtrlWidget* m_dimClustCtrlDlg;
    // The pointer to the GLCanvas helper class
    GLCanvas* m_canvas;
    //pointer to cluster computation results
    DimRddtCluster* m_dimClust;

    //draw dimensions on 2D space based on MDS layout;
    void drawDimensions();

    //which dimension is the user's mouse pointing to
    //providing the position of the mouse
    int whichDim(int x, int y);

    //return the name of the dimension that the user's
    //mouse points to
    QString caption(int x, int y);

    //int singleClickflag;

public slots:
    void updateCurrentView();

private slots:
    void singleClick();
};

#endif /* MANUALCLUSTDISPLAY_H_ */
