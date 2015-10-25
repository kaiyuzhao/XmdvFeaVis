/*
 * ManualClustDisplay.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: kaiyuzhao
 */



#include "ManualClustDisplay.h"
#include "main/XmdvTool.h"
#include "datatype/RGBt.h"

//#include "color/ColorManager.h"
#include "view/GLCanvas.h"
#include "data/multidim/OkcData.h"
#include "data/dimreduct/DimRddtCluster.h"
#include "data/dimreduct/RddtClust.h"

#include "interface/dimreduct/manualclust/ManualClustCtrlWidget.h"

#include <QPoint>
#include <QString>
#include <qDebug>

#include <string>
#include <math.h>
#include <QTimer>
#include <algorithm>
#include <QMouseEvent>
#include <cassert>
#include "color/ColorMap.h"
#include "color/ColorRamp.h"

//#include "main/XmdvToolTypes.h"




ManualClustDisplay::ManualClustDisplay(QWidget* parent,
                                       DimRddtCluster* dimClust)
    :QGLWidget(parent),m_dimClust(dimClust)
{
    this->cm =  g_globals.colorManager;
    m_dimClustCtrlDlg = dynamic_cast<ManualClustCtrlWidget*>(parent);
    this->m_canvas = new GLCanvas(this);
    //this->setVisible(false);
    connect(m_dimClust, SIGNAL(sigUpdateAllViews()),
            this,SLOT(updateCurrentView()));
    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(singleClick()));
}

ManualClustDisplay::~ManualClustDisplay() {
    // TODO Auto-generated destructor stub
    // SAFE_DELETE(m_currentSelection)
    SAFE_DELETE(m_canvas);
}

/*void ManualClustDisplay::setDimClust(DimRddtCluster* dimClust){
    m_dimClust = dimClust;
}*/

void ManualClustDisplay::initializeGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void ManualClustDisplay::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho ( (-CANVAS_BORD), (1+CANVAS_BORD),
              (1+CANVAS_BORD), (-CANVAS_BORD), -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void ManualClustDisplay::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawDimensions();
    setMouseTracking(true);
}

void ManualClustDisplay::drawDimensions(){
    Vec2Vector positions = this->m_dimClust->getPos();
    RddtClust* cluster;
    this->m_rects.clear();
    double pixh = (double)1/(double)this->height();
    double pixw = (double)1/(double)this->width();
    double maxSize = (double)1/(double)30;
    double minSize = (double)2/(double)100;

    double glyphSize = (double)1/18;
    double barWidth = glyphSize/4;
    double barHeight = glyphSize;
    ColorMap* colors =
            new ColorMap(cm->getPredefinedColorRamp(ColorManager::QUAL_SET1));
    cluster = dynamic_cast<RddtClust*>(this->m_dimClust->m_clusters[0]);
    double numDims;
    if(cluster==NULL)
        numDims = this->m_dimClust->getDimSize();
    else
        numDims= cluster->m_selected.size();
    for(unsigned i = 0; i<positions.size(); i++){
        if(i<this->m_dimClust->m_clusters.size()){//m_clusters.size()>0;
            cluster = dynamic_cast<RddtClust*>(this->m_dimClust->m_clusters[i]);
            if(cluster==NULL)
                return;
            DoubleVector barHeights;
            //first bar size of cluster;

            if(numDims==0)
                numDims=1;
            double currentNumDims = cluster->m_selected.size();
            double bar1 = barHeight*currentNumDims/numDims;
            barHeights.push_back(bar1);
            //second bar consistency;
            double mean = cluster->m_fitConsistency;
            double maxMean = 1; //max variance of data between [0,1] is 0.5;
            double bar2 = barHeight*mean/maxMean;
            barHeights.push_back(bar2);
            double range = 1.0;
            //third bar max;
            double maxDist = cluster->m_worstFit;
            double bar3 = barHeight*maxDist/range;
            barHeights.push_back(bar3);
            //forth bar min;
            double minDist = cluster->m_bestFit;
            double bar4 = barHeight*minDist/range;
            barHeights.push_back(bar4);

            double x = positions[i].X;
            double y = positions[i].Y;
            double blx = x-0.5*glyphSize;//bottom left
            double bly = y+0.5*glyphSize;//bottom left
            double tlx = blx;
            double tly = y-0.5*glyphSize;
            for(unsigned i = 0; i< barHeights.size(); i++){
                double val = (double)(i+1)/(double)(barHeights.size());

                unsigned long co = colors->getColor(val);
                m_canvas->setForeground(co,1.0);
                double h = barHeights[i];
                m_canvas->fillRectangle(blx+(double)i*barWidth,bly-h,barWidth,h);
            }
            Vec2 p = m_canvas->mapDataToScreen(Vec2(tlx,tly));
            QRect rect(p.X,p.Y,glyphSize*this->width(),glyphSize*this->height());
            this->m_rects.push_back(rect);
            if(this->m_dimClust->m_activeClusters[i]){
                unsigned long co =
                        this->cm->getColor(ColorManager::BRUSH_SELECTED1);
                m_canvas->setForeground(co);
                m_canvas->drawRectangle(tlx,tly,glyphSize,glyphSize);
            }
            if(i==this->m_dimClust->m_currentSelectionIdx){
                m_canvas->setForeground(0);
                m_canvas->drawCompleteCircle(tlx+0.5*glyphSize, tly+0.5*glyphSize,
                                             glyphSize*0.75,100);
               //m_canvas->drawRectangle(tlx-2*pixw,tly-2*pixh,glyphSize+4*pixw,glyphSize+4*pixh);
            }
        }else{
            int outlierIdx = i-this->m_dimClust->m_clusters.size();
            unsigned long co =
                    this->cm->getColor(ColorManager::BRUSH_UNSELECTED,0.3);
            m_canvas->setForeground(co,1.0);
            m_canvas->fillRectangle(positions[i].X,positions[i].Y,minSize,minSize);

            Vec2 p = m_canvas->mapDataToScreen(positions[i]);
            QRect rect(p.X,p.Y,minSize*this->width(),minSize*this->height());
            this->m_rects.push_back(rect);
            if(this->m_dimClust->m_activeOutliers[outlierIdx]){
                unsigned long co =
                        this->cm->getColor(ColorManager::BRUSH_SELECTED1);
                m_canvas->setForeground(co);
                m_canvas->drawRectangle(positions[i].X,positions[i].Y,minSize,minSize);
            }
        }
    }
    SAFE_DELETE(colors);
}

void ManualClustDisplay::mouseReleaseEvent(QMouseEvent *event){

    int x =event->x();
    int y =event->y();
    //qDebug()<<"mouse release";

    //the dimension user clicked on based on the mouse position
    //and range requirement
    int dim = whichDim(x,y);
    if(dim <0)
        return;
    int clustSize = this->m_dimClust->m_clusters.size();
    int clustMarkerSize = this->m_dimClust->m_activeClusters.size();

    int outlierSize = this->m_dimClust->m_outliers.size();
    int outlierMarkerSize = this->m_dimClust->m_activeOutliers.size();
    if(dim>=clustSize+outlierSize)
        return;

    assert(clustSize == clustMarkerSize);
    assert(outlierSize == outlierMarkerSize);

    if(event->button()==Qt::LeftButton){
        if(dim<clustSize){
            //clicked on a cluster;
            bool state = this->m_dimClust->m_activeClusters[dim];
            this->m_dimClust->m_activeClusters[dim] = state^true;
        }else{
            bool state = this->m_dimClust->m_activeOutliers[dim-clustSize];
            this->m_dimClust->m_activeOutliers[dim-clustSize] = state^true;
            //clicked on a outlier;
        }
    }
    this->m_dimClust->refreshCurrentList();
    this->m_dimClust->updateAllViews();
}

void ManualClustDisplay::singleClick(){

}

void ManualClustDisplay::mouseDoubleClickEvent ( QMouseEvent * event){
    //this->m_timer->stop();
    //this->singleClickflag = false;
    //qDebug()<<"mouse double click";
    int x =event->x();
    int y =event->y();
    //singleClickflag = 0;
    //the dimension user clicked on based on the mouse position
    //and range requirement
    int dim = whichDim(x,y);
    //cluster or outlier?
    if(dim<0){
        this->m_dimClust->m_currentSelection = NULL;
        return;
    }
    if(dim<(int)this->m_dimClust->m_clusters.size()){
        //draw selected cluster;
        this->m_dimClust->doSelectedClust(dim);
    }else{
        this->m_dimClust->m_currentSelection = NULL;
        return;
    }
    this->m_dimClust->updateAllViews(); //including current ManualClustDisplay view;
}

void ManualClustDisplay::mousePressEvent(QMouseEvent *event){

    //m_event_x = event->x();

    //m_event_y = event->y();
}

void ManualClustDisplay::mouseMoveEvent(QMouseEvent *event){
    //this->singleClickflag = false;
    //m_timer->stop();
    //qDebug()<<"mouse move manualClust";
    int x = event->x();
    int y = event->y();
    QString temp = caption(x, y);
    this->m_dimClustCtrlDlg->updateStatusBar(temp);
}


int ManualClustDisplay::whichDim(int x, int y){
    //Vec2 val = this->m_canvas->mapScreenToData(Vec2 (x,y));
    //QPoint cursorPos(x,y);
    //copy of original dimension list
    //std::vector<Vec2> dim_pos =  this->m_dimClust->getPos();
    unsigned int i;
    for(i=0;i<this->m_rects.size();i++){
        //double dist = sqrt(pow((dim_pos[i].X-val.X),2)+pow((dim_pos[i].Y-val.Y),2));
        if(m_rects[i].contains(x,y)){
            return i;
        }
    }
    return -1;
}

QString ManualClustDisplay::caption(int x, int y){
    int dim = whichDim(x,y);
    if(dim>-1){
        StringIterator it;
        it = this->m_dimClust->m_dimLayoutName.begin();
        return (*(it+dim)).c_str();
    }
    return "";
}



void ManualClustDisplay::updateCurrentView(){
    this->repaint();
}

/*void ManualClustDisplay::updateEPS(int rangeMax, int rangeMin, int slidPos){

}

void ManualClustDisplay::updateNPTS(int rangeMax, int rangeMin, int slidPos){

}
*/
