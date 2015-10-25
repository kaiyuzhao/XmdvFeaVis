#include "data/dimreduct/DataPartition.h"
#include "data/dimreduct/DataPartitions.h"

#include "main/XmdvTool.h"

#include "data/multidim/OkcData.h"
#include "data/dimreduct/RddtClust.h"
#include "data/dimreduct/DimRddtCluster.h"
#include "datatype/Vec2.h"
#include "view/GLCanvas.h"

#include "RddcEvlWidget.h"
#include "RddcEvlDisplay.h"
#include "main/XmdvToolTypes.h"

#include "color/ColorMap.h"
#include "color/ColorRamp.h"

#include <vector>
#include <QMouseEvent>
#include <QPoint>


RddcEvlDisplay::RddcEvlDisplay(QWidget* parent, DimRddtCluster* dimClust)
    :QGLWidget(parent),m_cntOneRow(10),m_x(-1),m_y(-1),
      m_isMouseDragged(false),m_isMousePressed(false),
      m_draggingGlyph(-1),m_glyIdxMouseOver(-1),
      m_parIdxMouseOver(-1),
      startIdx(-1),endIdx(-1)
{
    this->m_dimClust = dimClust;
    m_dimReductDlg = dynamic_cast<RddcEvlWidget*>(parent);
    this->m_canvas = new GLCanvas(this);
    //this->m_rddtClust
    cm = g_globals.colorManager;

    colors =
            new ColorMap(cm->getPredefinedColorRamp(ColorManager::SEQ_ORANGES));

    this->connect(m_dimClust,SIGNAL(sigUpdateAllViews()),
                  this,SLOT(refreshView()));
}

RddcEvlDisplay::~RddcEvlDisplay(){
    SAFE_DELETE(m_canvas);
    SAFE_DELETE(colors);
}

/*void RddcEvlDisplay::setDimClust(DimRddtCluster* dimClust){
    //disconnect(m_rddtClust,SIGNAL(m_rddtClust->sigRefreshViews()),
                     //this,SLOT(refreshView()));
    this->m_dimClust = dimClust;
    this->connect(m_dimClust,
            SIGNAL(sigRefreshViews()),
            this,
            SLOT(refreshView()));
}*/

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
    if(!mutex.tryLock())
        return;
    drawLocalDetail();
    drawProfiles();
    //drawHopOver();

    setMouseTracking(true);
    mutex.unlock();
}

void RddcEvlDisplay::mouseReleaseEvent(QMouseEvent *event){
    int x = event->x();
    int y = event->y();
    if(m_isMouseDragged){
        end = QPoint(x,y);
        RddtClust* clust = this->m_dimClust->m_currentSelection;
        if(clust==NULL)
            return;
        if(m_draggingGlyph<0 || m_draggingGlyph >= clust->m_order.size())
            return;
        int locIdx = clust->m_order[m_draggingGlyph];
        if(locIdx<0 || locIdx>=clust->m_partitionPerVariable.size())
            return;
        //int globIdx = clust->m_selected[locIdx];
        this->m_dimClust->m_currentSelection->
                doSubSetting(locIdx,startIdx,endIdx);
        this->m_isMouseDragged = false;
    }
    m_isMousePressed = false;
}

void RddcEvlDisplay::mousePressEvent(QMouseEvent *event){
    int x = event->x();
    int y = event->y();
    startIdx = -1;
    endIdx = -1;
    start = QPoint(x,y);
    //this->m_isMouseDragged=true;
    m_isMousePressed = true;
    m_draggingGlyph = this->whichGlyph(x,y);
}

void RddcEvlDisplay::mouseMoveEvent(QMouseEvent *event){

    m_x = event->x();
    m_y = event->y();

    end = QPoint(m_x,m_y);

    if(this->m_dimClust==NULL)
        return;
    RddtClust* clust = this->m_dimClust->m_currentSelection;

    if(clust==NULL)
        return;
    if(clust->m_nRddtDims.size()<1)
        return;

    if(m_isMousePressed){
        m_isMouseDragged = true;
    }
    if(m_isMouseDragged){

        if(m_draggingGlyph<0 ||
                m_draggingGlyph>= clust->m_partitionPerVariable.size())
            return;

        DataPartitions* pars = dynamic_cast<DataPartitions*>
                (clust->m_partitionPerVariable[m_draggingGlyph]);
        QRect current = this->m_profileRects[m_draggingGlyph];
        int x = current.left();
        int y = current.right();
        int width = current.width();
        //int height = current.height();
        int nBins = pars->m_partitions.size();

        int start_x = std::min(start.x(),end.x());
        int end_x = std::max(start.x(),end.x());

        int startOffset = start_x-x;
        int endOffset = end_x-x;

        double binWidth = (double)width/(double)nBins;

        startIdx = (int)(floor((double)startOffset/binWidth));
        endIdx = (int)(floor((double)endOffset/binWidth));
    }

    ////mouse move event;
    int which = this->whichGlyph(m_x,m_y);
    if(which<0 ||
            which>= clust->m_partitionPerVariable.size())
        return;
    if(m_glyIdxMouseOver != which){
       m_glyIdxMouseOver = which;
       m_parIdxMouseOver = -1;
    }

    DataPartitions* partitions = dynamic_cast<DataPartitions*>
            (clust->m_partitionPerVariable[which]);
    int globIdx = partitions->m_variable;

    if(globIdx<0 || globIdx> this->m_dimClust->m_dimName.size())
        return;
    std::string name = this->m_dimClust->m_dimName[globIdx];
    QString info(name.c_str());
    this->m_dimReductDlg->updateStatusBar(info);

    this->repaint();
}

void RddcEvlDisplay::mouseDoubleClickEvent ( QMouseEvent * event ){

}

void RddcEvlDisplay::drawProfiles(){
    if(this->m_dimClust==NULL)
        return;
    RddtClust* clust = this->m_dimClust->m_currentSelection;
    if(clust==NULL)
        return;
    if(clust->m_nRddtDims.size()<1){
        return;
    }
    int size = clust->m_partitionPerVariable.size();
    if(size<1)
        return;

    this->m_profileRects.clear();

    int numCols = (int)(sqrt((double)size))+1;
    int numRows = (int)ceil((double)size/(double)numCols);

    double glyphWidthData = (double)1/(double)numCols;
    double glyphHeightData = (double)1/(double)numRows;

    std::vector<DataPartitions*>::iterator dataParts =
            clust->m_partitionPerVariable.begin();

    int i = 0;

    for(;dataParts != clust->m_partitionPerVariable.end();++dataParts){
        int hoPos = i%numCols;
        int verPos = i/numCols;
        double x = (double)hoPos/(double)numCols;
        double y = (double)verPos/(double)numRows;

        Vec2 pos(x,y);

        drawProfile((*dataParts), pos, glyphWidthData,glyphHeightData, i);
        i++;
    }
}

void RddcEvlDisplay::drawProfile(DataPartitions* partitionOneVar, Vec2 pos, double w, double h, int idx){

    double binSize = this->m_dimClust->m_nBins;

    double vSpaceData = 0.10*h;
    double hSpaceData = 0.10*w;

    double barWidth = (w-2*hSpaceData)/binSize;
    double barHeight = (h-2*vSpaceData);

    double max = (double)this->m_dimClust->m_currentSelection->m_rddtDims.size();
    double min = 0;
    int numBars = partitionOneVar->m_partitions.size();
    for(int i=0; i< numBars; i++){
        DataPartition* p =
                dynamic_cast<DataPartition*>(partitionOneVar->m_partitions[i]);
        double globFit = this->m_dimClust->m_currentSelection->m_fitConsistency;
        double localFit = p->m_mean;
        double current;
        if(localFit>globFit){
            current = globFit/localFit*0.5*barHeight;
        }else{
            current = barHeight - localFit/globFit*0.5*barHeight;
        }
        //double current = ((double)p->m_cntDec-min)/(max-min)*barHeight;

        double left = pos.X+barWidth*i+hSpaceData;
        double top = pos.Y+barHeight + vSpaceData - current;

        float val = (float)i/(float)numBars;

        if(this->m_draggingGlyph==idx && i>=this->startIdx && i<=endIdx){
            //unsigned long c =
                    //this->cm->getColor(ColorManager::BRUSH_SELECTED1);
            this->m_canvas->setForeground(RGBt(0.0,0.0,1.0));
            this->m_canvas->fillRectangle(left,top,barWidth,current);
            //draw highlighted bars;
        }else{
            unsigned long color = colors->getColor(val);
            //unsigned long color =
                    //this->cm->getColor(ColorManager::BRUSH_UNSELECTED,val);
            this->m_canvas->setForeground(color);
            this->m_canvas->fillRectangle(left,top,barWidth,current);
        }
        if(m_glyIdxMouseOver == idx &&
                i==m_parIdxMouseOver ){
            //qDebug()<<"\nm_parIdxMouseOver "<<m_parIdxMouseOver;
            this->m_canvas->setForeground(RGBt(0.0,1.0,0.0));
            this->m_canvas->fillRectangle(left,top,barWidth,current);
            m_parIdxMouseOver = -1;
        }
    }
    this->m_canvas->setForeground(0);
    int gidx = partitionOneVar->m_variable;
    std::string name="";
    if(gidx>=0 && gidx<this->m_dimClust->m_dimName.size()){
        name = this->m_dimClust->m_dimName[gidx];
    }

    //x axis
    this->m_canvas->drawLine(pos.X+hSpaceData,pos.Y+h-vSpaceData,
                             pos.X+w-hSpaceData,pos.Y+h-vSpaceData);
    //y axis;
    this->m_canvas->drawLine(pos.X+hSpaceData,pos.Y+h-vSpaceData,
                             pos.X+hSpaceData,pos.Y+vSpaceData);

    //mid line where glob = local
    // red base line
    unsigned long c = this->cm->getColor(ColorManager::BRUSH_SELECTED1);
    this->m_canvas->setForeground(c);
    this->m_canvas->drawLine(pos.X+hSpaceData,pos.Y+0.5*h,
                             pos.X+w-hSpaceData,pos.Y+0.5*h);

    //Vec2 stringPos = m
    Vec2 topLeftData(0,0);
    Vec2 bottomRightData(1,1);
    Vec2 topLeftScreen = m_canvas->mapDataToScreen(topLeftData);
    Vec2 bottomRightScreen = m_canvas->mapDataToScreen(bottomRightData);

    double width = bottomRightScreen.X-topLeftScreen.X;
    double height = bottomRightScreen.Y-topLeftScreen.Y;

    this->m_canvas->setForeground(0);
    Vec2 fontpos = this->m_canvas->
            mapDataToScreen(Vec2(pos.X+hSpaceData,pos.Y+h-vSpaceData));
    QFont font("Times", 10, QFont::Normal);
    int pixSize = vSpaceData*this->height();
    int maxPixSize = 30;
    pixSize = pixSize>maxPixSize?maxPixSize:pixSize;

    font.setPixelSize(pixSize-2);
    this->m_canvas->drawString(fontpos.X,
                               fontpos.Y+pixSize+1,(char*)name.c_str(),&font);

    this->m_canvas->setForeground(c);

    Vec2 blCharPos = this->m_canvas->
            mapDataToScreen(Vec2(pos.X,pos.Y+0.5*h));
    this->m_canvas->drawString(blCharPos.X,blCharPos.Y,
                               (char*)"g",&font);
    Vec2 wCharPos = this->m_canvas->
            mapDataToScreen(Vec2(pos.X,pos.Y));
    this->m_canvas->drawString(wCharPos.X,wCharPos.Y+pixSize,
                               (char*)"s",&font);

    Vec2 sCharPos = this->m_canvas->
            mapDataToScreen(Vec2(pos.X,pos.Y+h));
    this->m_canvas->drawString(sCharPos.X,sCharPos.Y-pixSize,
                               (char*)"w",&font);

    this->m_canvas->setForeground(RGBt(0.0,0.0,0.0));
    this->m_canvas->drawString(topLeftScreen.X,topLeftScreen.Y-0.5*pixSize,
                               (char*)"g = global similarity; s = strong; w = weak");

    Vec2 p = this->m_canvas->
            mapDataToScreen(Vec2(pos.X+hSpaceData, pos.Y+vSpaceData));

    QRect tmp((int)p.X,(int)p.Y,(int)((w-2*hSpaceData)*width),  //w-2*hSpaceData
              (int)((h-2*vSpaceData)*height));
    m_profileRects.push_back(tmp);
}

int RddcEvlDisplay::whichGlyph(int x, int y){

    for(unsigned i = 0; i< m_profileRects.size(); i++){
        if(m_profileRects[i].contains(x,y)){
            return (int)i;
        }
    }
    return -1;
}
void RddcEvlDisplay::drawLocalDetail(){

    if(this->m_dimClust==NULL)
        return;
    RddtClust* clust = this->m_dimClust->m_currentSelection;
    if(clust==NULL)
        return;
    if(clust->m_nRddtDims.size()<1){
        return;
    }
    int size = clust->m_partitionPerVariable.size();

    if(size<1)
        return;
    int which = this->whichGlyph(m_x,m_y);

    if(which<0 ||
            which>= clust->m_partitionPerVariable.size())
        return;
    //m_parIdxMouseOver = -1;
    QRect current = this->m_profileRects[which];
    int x = current.left();
    int x_right = current.right();
    int width = current.width();
    //int height = current.height();
    DataPartitions* partitions = dynamic_cast<DataPartitions*>
            (clust->m_partitionPerVariable[which]);
    int nBins = partitions->m_partitions.size();

    double binWidth = (double)width/(double)nBins;
    int offset = -1;
    if(m_x>x && m_x <x_right){
        offset = m_x-x;
    }
    //qDebug()<<"glyph idx "<<which;
    //qDebug()<<"m_x "<<m_x<<" x "<<x;
    m_parIdxMouseOver = (int)(floor((double)offset/binWidth));
    //qDebug()<<"mouse over par idx "<<m_parIdxMouseOver<<" offset "<<offset;
    if(offset>=width || offset <= 0)
    {
        m_parIdxMouseOver = -1;
        return;
    }

    DataPartition* partitionMouseOver = dynamic_cast<DataPartition*>
            (partitions->m_partitions[m_parIdxMouseOver]);

    IntIterator itDimOutofGroup = partitionMouseOver->m_dimOutofGroup.begin();

    for(;itDimOutofGroup!=partitionMouseOver->m_dimOutofGroup.end();itDimOutofGroup++){
        int varIdx = (*itDimOutofGroup);

        int locIdx = -1;
        for(int i=0; i<clust->m_order.size(); i++){
            int tmp = clust->m_order[i];
            if(tmp==varIdx)
                locIdx = i;
        }
        //int locIdx = clust->m_order[varIdx];

        if(locIdx<0 || locIdx>=clust->m_partitionPerVariable.size())
            return;

        QRect tmp = this->m_profileRects[locIdx];

        int size = clust->m_partitionPerVariable.size();
        if(size<2)
            return;

        Vec2 pos(tmp.left(),tmp.bottom());
        Vec2 pos2(tmp.left()+tmp.width(),tmp.bottom()-tmp.height());

        Vec2 topCorner = this->m_canvas->mapScreenToData(pos);
        Vec2 bottomRightCorner =
                this->m_canvas->mapScreenToData(pos2);

        double glyphWidthData = bottomRightCorner.X - topCorner.X;
        double glyphHeightData = (topCorner.Y - bottomRightCorner.Y);

        //unsigned long c = this->cm->getColor(RGBt(0.0,1.0,0.0));
        this->m_canvas->setForeground(RGBt(1.0,0.8,0.0));
        this->m_canvas->fillRectangle(topCorner.X,(double)1-topCorner.Y,
                                      glyphWidthData,glyphHeightData);
    }

}

void RddcEvlDisplay::drawHopOver(){
    if(this->m_dimClust==NULL)
        return;
    RddtClust* clust = this->m_dimClust->m_currentSelection;
    if(clust==NULL)
        return;
    if(clust->m_nRddtDims.size()<1){
        return;
    }
    int which = this->whichGlyph(m_x,m_y);
    if(which<0 || which>=m_profileRects.size())
        return;
    QRect tmp = this->m_profileRects[which];
    int size = clust->m_partitionPerVariable.size();
    if(size<2)
        return;
    int numCols = (int)(sqrt((double)size))+1;
    int numRows = (int)ceil((double)size/(double)numCols);

    double glyphWidthData = (double)1/(double)numCols;
    double glyphHeightData = (double)1/(double)numRows;

    Vec2 pos(tmp.left(),tmp.bottom());
    Vec2 pos2(tmp.left()+tmp.width(),tmp.bottom()-tmp.height());

    Vec2 topCorner = this->m_canvas->mapScreenToData(pos);
    Vec2 bottomRightCorner =
            this->m_canvas->mapScreenToData(pos2);

    glyphWidthData = bottomRightCorner.X - topCorner.X;
    glyphHeightData = (topCorner.Y - bottomRightCorner.Y);

    unsigned long c = this->cm->getColor(ColorManager::BRUSH_SELECTED1);
    this->m_canvas->setForeground(c);
    this->m_canvas->drawRectangle(topCorner.X,(double)1-topCorner.Y,
                                  glyphWidthData,glyphHeightData);

}

// i'th variable in the redundent group of "size" dimensions


void RddcEvlDisplay::refreshView(){
    this->repaint();
}

void RddcEvlDisplay::onDataFiltering(){
    this->startIdx = -1;
    this->endIdx = -1;
    //this->repaint();
}

void RddcEvlDisplay::onClearDataFiltering(){
    this->startIdx = -1;
    this->endIdx = -1;
}
