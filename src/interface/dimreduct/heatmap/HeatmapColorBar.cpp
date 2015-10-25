#include "HeatmapColorBar.h"

#include "data/dimreduct/DimRddtCluster.h"
#include "data/dimreduct/RddtClust.h"

#include "interface/dimreduct/heatmap/HeatmapClustCtrlWidget.h"
#include "view/GLCanvas.h"

#include <QWidget>

HeatmapColorBar::HeatmapColorBar(QWidget* parent,DimRddtCluster* dimClust)
    :m_dimClust(dimClust)
{
    this->setMinimumHeight(40);//colorbar height
    this->setMaximumHeight(50);//colorbar height
    m_hmpClustCtrlDlg = dynamic_cast<HeatmapClustCtrlWidget*>(parent);
    this->m_canvas = new GLCanvas(this);
    this->cm =  g_globals.colorManager;
    m_numGrid = 15; //draw 15 grids by default

    connect(m_dimClust,SIGNAL(sigUpdateAllViews()),this,SLOT(updateView()));
}

void HeatmapColorBar::updateView(){
    repaint();
}

HeatmapColorBar::~HeatmapColorBar(){
    SAFE_DELETE(m_canvas);
}

void HeatmapColorBar::initializeGL(){
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void HeatmapColorBar::resizeGL(int width, int height){
    glViewport(0, 0, width, height);
    //glPushAttrib(GL_ALL_ATTRIB_BITS);
    //glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho ( (-CANVAS_BORD), (1+CANVAS_BORD),
              (1+CANVAS_BORD), (-CANVAS_BORD), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void HeatmapColorBar::paintGL(){
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //draw the color map here:

    drawColorBar();
}

void HeatmapColorBar::drawColorBar(){
    bool showMaxDist = false;
    double maxDistTicker = 0;
    if(this->m_dimClust->m_currentSelectionIdx>-1){
        if(this->m_dimClust->m_currentSelection!=NULL){
            showMaxDist = true;
            maxDistTicker = this->m_dimClust->m_currentSelection->m_worstFit;
        }
    }

    for(int i = 0; i<this->m_numGrid; i++){
        double val = (double)i/(double)m_numGrid;
        double w = (double)1/(double)m_numGrid;
        unsigned long color =
                this->cm->getColor(ColorManager::BRUSH_UNSELECTED,val);
        this->m_canvas->setForeground(color);
        this->m_canvas->fillRectangle(val,0,w,0.5);
        QString start = QString::number(0);
        QString end = QString::number(1);
        Vec2 startPos = this->m_canvas->mapDataToScreen(Vec2(0,1));
        Vec2 endPos = this->m_canvas->mapDataToScreen(Vec2(1,1));
        this->m_canvas->drawString(startPos.X,startPos.Y,(char*)(start.toAscii().data()));
        this->m_canvas->drawString(endPos.X,endPos.Y,(char*)(end.toAscii().data()));
    }
    if(showMaxDist){
        unsigned long color = this->cm->getColor(ColorManager::BRUSH_SELECTED1);
        this->m_canvas->setForeground(color);
        this->m_canvas->drawLine(maxDistTicker,0,maxDistTicker,0.5);
        Vec2 ticker = this->m_canvas->mapDataToScreen(Vec2(maxDistTicker,1));
        QString curMax = QString::number(maxDistTicker,'g',3);
        curMax = "Current max:"+curMax;
        this->m_canvas->drawString(ticker.X,ticker.Y,(char*)(curMax.toAscii().data()));

    }
}
