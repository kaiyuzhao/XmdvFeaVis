#include "data/dimreduct/DataPartition.h"
#include "RddcEvlDisplay.h"
#include "data/dimreduct/DataPartitions.h"
#include "main/XmdvTool.h"

#include "view/GLCanvas.h"
#include "datatype/Vec2.h"
#include "data/multidim/OkcData.h"

#include "RddcEvlWidget.h"
#include "data/dimreduct/RddtClust.h"



#include <vector>

RddcEvlDisplay::RddcEvlDisplay(QWidget* parent)
:QGLWidget(parent)
{
    m_dimReductDlg = dynamic_cast<RddcEvlWidget*>(parent);
    this->m_canvas = new GLCanvas(this);
    cm = g_globals.colorManager;
}

RddcEvlDisplay::~RddcEvlDisplay(){
    SAFE_DELETE(m_canvas);
}

void RddcEvlDisplay::setDimClust(DimRddtCluster* dimClust){
    //disconnect(m_rddtClust,SIGNAL(m_rddtClust->sigRefreshViews()),
                     //this,SLOT(refreshView()));
    this->m_rddtClust = dimClust;
    this->connect(m_rddtClust,
            SIGNAL(sigRefreshViews()),
            this,
            SLOT(refreshView()));
}

void RddcEvlDisplay::initializeGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void RddcEvlDisplay::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho ( (-CANVAS_BORD), (1+CANVAS_BORD), (1+CANVAS_BORD), (-CANVAS_BORD), -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void RddcEvlDisplay::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawProfiles();
    setMouseTracking(true);
}

void RddcEvlDisplay::mouseReleaseEvent(QMouseEvent *event){

}

void RddcEvlDisplay::mousePressEvent(QMouseEvent *event){

}

void RddcEvlDisplay::mouseMoveEvent(QMouseEvent *event){

}

void RddcEvlDisplay::mouseDoubleClickEvent ( QMouseEvent * event ){

}

void RddcEvlDisplay::drawProfiles(){
    if(this->m_rddtClust!=NULL){
        RddtClust* clust = this->m_rddtClust->m_currentSelection;
        if(clust!=NULL){
            std::vector<DataPartitions*>::iterator dataParts =
                    clust->m_partitionPerVariable.begin();
            int i = 0;
            int size = clust->m_partitionPerVariable.end()
                    -clust->m_partitionPerVariable.begin();
            for(;dataParts != clust->m_partitionPerVariable.end();++dataParts){
                drawProfile((*dataParts), i++, size);
            }
        }

    }
}

// i'th variable in the redundent group of "size" dimensions
void RddcEvlDisplay::drawProfile(DataPartitions* partitionOfOneVar, int i, int size){
    //
    //RddtClust* clust = this->m_rddtClust->m_currentView;
    int varIdx = partitionOfOneVar->m_variable;
    //std::string name = this->m_rddtClust->getNames()[varIdx];
    double x = (double)i/(double)size;
    double glyphSize = (double)1/(double)size;
    double height = glyphSize*2;
    double width = glyphSize/4*3;
    Vec2 pos(x,0.15);
    //Vec2 textPos(x, 0.1+height);
    //this->m_canvas->setForeground(0);
    //Vec2 scPos = this->m_canvas->mapDataToScreen(textPos);
    //m_canvas->drawVString(scPos.X,scPos.Y,name.c_str());
    //glLoadIdentity();
    //this->m_canvas->drawVString(;
    //float val =
    //this->cm->getColor(ColorManager::BRUSH_UNSELECTED,val);
    for(unsigned i=0; i< partitionOfOneVar->m_partitions.size(); i++){
        double s = partitionOfOneVar->m_partitions.size();
        DataPartition* p = partitionOfOneVar->m_partitions[i];
        float total = (double)this->m_rddtClust->m_currentSelection->m_rddtDims.size();
        float valup = (p->cntDec)/total;
        float valdown = p->cntInc/total;
        double up = height*valup;
        double down = height*valdown;
        double barWidth = width/s;
        double rectIdx = p->partitionIdx;
        double upx = pos.X+barWidth*rectIdx;
        double upy = pos.Y-up;
        float val = (float)((double)p->partitionIdx/s);
        unsigned long colorup =
                this->cm->getColor(ColorManager::BRUSH_UNSELECTED,val);
        this->m_canvas->setForeground(colorup);
        this->m_canvas->fillRectangle(upx,upy,barWidth,up);
    }
    this->m_canvas->setForeground(0);
    this->m_canvas->drawLine(pos.X,pos.Y,pos.X+width,pos.Y);
    this->m_canvas->drawLine(pos.X,pos.Y,pos.X,pos.Y-height);
}

void RddcEvlDisplay::refreshView(){
    //this->repaint();
}
