#include "HeatmapDisplay.h"
#include "data/dimreduct/DimRddtCluster.h"
#include "data/dimreduct/RddtClust.h"

#include "interface/dimreduct/heatmap/HeatmapClustCtrlWidget.h"
#include "view/GLCanvas.h"

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDebug>
#include <string>


HeatmapDisplay::HeatmapDisplay(QWidget* parent,DimRddtCluster* dimClust)
    :m_dimClust(dimClust),m_currentX(-1),m_currentY(-1),m_currentGridSize(-1.0)
{
    //strPainter = new QPainter(this);
    m_hmpClustCtrlDlg = dynamic_cast<HeatmapClustCtrlWidget*>(parent);
    this->m_canvas = new GLCanvas(this);
    this->cm =  g_globals.colorManager;
    connect(m_dimClust,SIGNAL(sigUpdateAllViews()),
            this,SLOT(refreshView()));
    //strPainter->beginNativePainting();
}

HeatmapDisplay::~HeatmapDisplay(){
    SAFE_DELETE(m_canvas);
}


void HeatmapDisplay::initializeGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void HeatmapDisplay::resizeGL(int width, int height)
{
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
void HeatmapDisplay::paintGL(){
    setMouseTracking(true);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawHeatMap();
    drawSelection();
    drawHopOver();
    drawVarNames();
}


void HeatmapDisplay::drawVarNames(){
    //qDebug()<<"drawVarNames";
    double height = floor(CANVAS_BORD/(2*CANVAS_BORD+1)*this->height());
    double width =  this->width();

    RddtClust* currentClust = NULL;
    if(this->m_dimClust==NULL)
        return;
    currentClust = this->m_dimClust->m_currentSelection;
    if(currentClust == NULL)
        return;
    int r = 0;
    r= currentClust->m_selected.size();
    if(r!= currentClust->m_order.size())
        return;
    if(r<=0)
        return;
    int start_x = 10+(CANVAS_BORD/(1+2*CANVAS_BORD)*width)/sqrt(2.0); //CANVAS_BORD*width;//move right
    int start_y = 10-(CANVAS_BORD/(1+2*CANVAS_BORD)*width)/sqrt(2.0); //CANVAS_BORD*width;//move up
    double offset = (double)width*(((double)1)/(1+2*CANVAS_BORD))/((double)r*sqrt(2.0));

    QImage img(width,height,QImage::Format_ARGB32);
    QPainter p;
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(),Qt::white);
    //p.setBrush(Qt::red);
    //p.drawRect(rect());
    p.setBrush(Qt::black);
    p.save();
    p.translate(0,0);
    p.rotate(45);
    QFont f;
    f.setFamily("Arial");
    f.setPointSize(8);
    //QFont font("Times",5,QFont::Normal);
    //font.setPixelSize();
    //p.setFont(font);
    f.setStyleStrategy(QFont::PreferAntialias);
    p.setFont(f);
    for(int i = 0; i<r; i++){
        int x = (int)(start_x+offset*(double)i);
        int y = (int)(start_y-offset*(double)i);
        int locIdx = currentClust->m_order[i];
        int globIdx = currentClust->m_selected[locIdx];
        bool selected = currentClust->m_colSelected[i];
        std::string name = this->m_dimClust->m_dimName[globIdx];
        QString qname = QString::fromAscii(name.c_str());

        if(selected){
            p.setPen(Qt::red);
            p.drawText(x,y,qname);
        }else{
            p.setPen(Qt::black);
            p.drawText(x,y,qname);
        }
    }

    p.restore();
    p.end();

    QImage gldata = QGLWidget::convertToGLFormat(img);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslated(CANVAS_BORD,0,0);
    glDepthMask(0); //before glDrawPixels().
    glDrawPixels(img.width(), img.height(), GL_RGBA, GL_UNSIGNED_BYTE, gldata.bits());
    glDepthMask(1); //Restore
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void HeatmapDisplay::mouseReleaseEvent(QMouseEvent *event){

}

void HeatmapDisplay::mousePressEvent(QMouseEvent *event){
    int x =event->x();
    int y = this->height() - event->y();
    if(this->m_dimClust==NULL){
        return;
    }
    if(this->m_dimClust->m_currentSelection==NULL){
        return;
    }

    RddtClust* currentClust = this->m_dimClust->m_currentSelection;
    int horCol = this->whichHozPos(x,y);

    if(horCol<0 || horCol >= currentClust->m_colSelected.size())
        return;

    if(event->button()==Qt::LeftButton){
        //if current dimension is unselected
        //make current dimension selected;
        //else make current dimension unselected;
        //int var = this->whichVar(x,y);
        //int locVar = this->whichLocVar();
        //int idx = this->m_dimClust->m_currentSelection->isNRddt(globVar);
        currentClust->switchSelectionState(horCol);
        this->m_dimClust->refreshClusters();
        currentClust->redoLocalAnalysis();
        this->m_dimClust->refreshCurrentList();
    }

    if(event->buttons()==Qt::RightButton){
        //remove the corresponding index from this->m_colSelected
        //remove the current selection from the cluster
        //reordering the dims;
        //remove either from m_rddtDims or from m_nRddtDims;
        //add current selection to m_outliers of m_dimClust
        currentClust->removeSelection(horCol);
        this->m_dimClust->refreshClusters();
        if(currentClust!=NULL){
            currentClust->doHeatmap();
            currentClust->redoLocalAnalysis();
        }
    }
    //do something and then update other views;
    this->m_dimClust->updateAllViews();
}

void HeatmapDisplay::mouseMoveEvent(QMouseEvent *event){
    //qDebug()<<"mouse move HeatmapDisplay";
    //qDebug("HeatmapDisplay");
    int x = event->x();
    int y = this->height()-event->y();

    m_currentX = x;
    m_currentY = y;
    int n = this->howManyNeighs(x,y);
    double dist = this->whatDistThres(x,y);
    std::string name = this->whatName(x,y);
    //qDebug("HeatmapDisplay1");
    if(dist<0)
        return;

    int globIdx = this->whichGlobVar(x,y);
    //qDebug("HeatmapDisplay2");
    if(globIdx<0 || globIdx>this->m_dimClust->m_dimName.size()){
        //qDebug()<<"globIdx "<<globIdx<<" ; m_dimClust->m_dimName.size() "
         //      <<this->m_dimClust->m_dimName.size();
        return;
    }
    //qDebug("HeatmapDisplay3");
    std::string vname = this->m_dimClust->m_dimName[globIdx];
    QString strVarName = "variable: "+QString::fromAscii(vname.c_str());
    QString strNumNei = " neighbours: "+QString::number(n);
    QString strDist = " distance: "+QString::number(dist,'f',3);
    QString strName = " name: "+QString::fromAscii(name.c_str());
    this->m_hmpClustCtrlDlg->updateStatusBar(strVarName+strNumNei+strDist+strName);
    //qDebug("this->repaint()");
    this->repaint();
}

void HeatmapDisplay::mouseDoubleClickEvent ( QMouseEvent * event){

}

void HeatmapDisplay::drawHopOver(){
    //qDebug()<<"drawHopOver";
    int x = m_currentX;

    int y = m_currentY;

    //may return -1 indicating not over any grid;
    int v = whichLocVar(x,y);

    //may return -1 indicating not over any grid;
    int n = howManyNeighs(x,y);

    //check whichVar and howManyNeighs returns non -1;
    if(v<0 || n<0)
        return;
    if(this->m_dimClust==NULL)
        return;
    if(this->m_dimClust->m_currentSelection==NULL)
        return;

    int r = this->m_dimClust->m_currentSelection->m_selected.size();

    Vec2 tmp(x,y);

    Vec2 data = this->m_canvas->mapScreenToData(tmp);

    int i = (int)((double)r*data.X);
    int j = (int)((double)r*data.Y);

    if(data.X>1 || data.Y>1)
        return;

    double topx = (double)i/((double)r);
    double topy = (double)j/((double)r);

    unsigned long c = this->cm->getColor(ColorManager::BRUSH_SELECTED1);
    this->m_canvas->setForeground(c);
    this->m_canvas->drawRectangle(topx,topy,
                                  m_currentGridSize,m_currentGridSize);
}

void HeatmapDisplay::drawHeatMap(){
    //qDebug()<<"drawHeatmap";
    RddtClust* currentClust = NULL;
    if(this->m_dimClust==NULL)
        return;
    currentClust = this->m_dimClust->m_currentSelection;
    if(currentClust == NULL)
        return;
    int r = 0;
    r= currentClust->m_selected.size();
    if(r!= currentClust->m_order.size())
        return;
    if(r==0)
        return;
    m_currentGridSize = (double)1/(double)r;
    for(int i = 0; i<r; i++){
        int var = currentClust->m_order[i];
        //initialize the colSelected status to all false;
        for(int j = 0; j<r; j++){
            float val = (float)(currentClust->getParamVal(var,j));
            unsigned long color =
                    this->cm->getColor(ColorManager::BRUSH_UNSELECTED,val);
            this->m_canvas->setForeground(color);
            double x = (double)i/((double)r);
            double y = (double)(r-j-1)/((double)r);
            this->m_canvas->fillRectangle(x,y,m_currentGridSize,m_currentGridSize);
        }
    }
}

int HeatmapDisplay::whichHozPos(int x, int y){
    RddtClust* currentClust = NULL;
    if(this->m_dimClust == NULL)
        return -1;
    currentClust = this->m_dimClust->m_currentSelection;
    if(currentClust == NULL)
        return -1;
    int r = currentClust->m_selected.size();
    Vec2 tmp(x,y);
    Vec2 data = this->m_canvas->mapScreenToData(tmp);
    //double size = this->width()/(double)r;
    int var = (int)((double)data.X*double(r));
    int vIdx = var>=0&&var<r?var:-1;
    return vIdx;
}

int HeatmapDisplay::whichLocVar(int x, int y){
    RddtClust* currentClust = NULL;
    if(this->m_dimClust == NULL)
    {
        //qDebug()<<"this->m_dimClust == NULL";
        return -1;
    }
    currentClust = this->m_dimClust->m_currentSelection;
    if(currentClust == NULL){
        //qDebug()<<"currentClust == NULL";
        return -1;
    }
    int vIdx = whichHozPos(x,y);
    if(vIdx<0 || vIdx >= currentClust->m_order.size())
    {
        //qDebug()<<"vIdx "<<vIdx;
        ///qDebug()<<"vIdx<0 || vIdx >= currentClust->m_order.size()";
        return -1;
    }
    int local = currentClust->m_order[vIdx];
    //qDebug()<<"vIdx "<<vIdx;
    return local;
}

int HeatmapDisplay::whichGlobVar(int x, int y){
    int var = this->whichLocVar(x,y);
    int n = this->howManyNeighs(x,y);
    //check whichVar and howManyNeighs returns non -1;
    if(var < 0 || n < 0)
    {
        //qDebug()<<"var<0 n<0";
        //qDebug()<<"var "<<var;
        //qDebug()<<"n "<<n;
        return -1;
    }
    if(m_dimClust==NULL)
    {
        //qDebug()<<"m_dimClust==NULL";
        return -1;
    }
    if(m_dimClust->m_currentSelection==NULL)
    {
        //qDebug()<<"m_dimClust->m_currentSelection==NULL";
        return -1;
    }
    if(var>=m_dimClust->m_currentSelection->m_selected.size())
    {
        //qDebug()<<"var>=m_dimClust->m_currentSelection->m_selected.size()";
        return -1;
    }
    int globIdx = this->m_dimClust->m_currentSelection->m_selected[var];
    if(globIdx<0 || globIdx >m_dimClust->m_dimName.size())
    {
        //qDebug()<<"var "<<var;
        //qDebug()<<"globIdx<0 || globIdx >m_dimClust->m_dimName.size()";
        return -1;
    }
    //qDebug()<<"this->m_dimClust->m_currentSelection->m_selected[var]";
    return globIdx;
}

int HeatmapDisplay::howManyNeighs(int x, int y){
    if(this->m_dimClust==NULL)
        return -1;
    if( this->m_dimClust->m_currentSelection==NULL)
        return -1;
    int r = this->m_dimClust->m_currentSelection->m_selected.size();
    if(r==0)
        return -1;
    Vec2 tmp(x,this->height()-y);
    Vec2 data = this->m_canvas->mapScreenToData(tmp);
    int num = (int)((double)r*data.Y);
    return num>=0&&num<r?num:-1;
}

std::string HeatmapDisplay::whatName(int x, int y){
    int v = whichLocVar(x,y);
    int n = howManyNeighs(x,y);
    //RddtClust* clust = NULL;
    if(v < 0 && n< 0){
        return "";
    }
    if(this->m_dimClust==NULL)
        return "";
    if(m_dimClust->m_currentSelection==NULL)
        return "";
    std::string t = this->m_dimClust->m_currentSelection->getParamName(v,n);
    return t;
}

double HeatmapDisplay::whatDistThres(int x,int y){
    int v = whichLocVar(x,y);
    int n = howManyNeighs(x,y);
    //RddtClust* clust = NULL;
    if(v < 0 && n< 0){
        return -1.0;
    }
    if(this->m_dimClust==NULL)
        return -1.0;
    if(m_dimClust->m_currentSelection==NULL)
        return -1.0;
    double t = this->m_dimClust->m_currentSelection->getParamVal(v,n);
    return t;
}

//this is the only place to call repaint
void HeatmapDisplay::refreshView(){
    this->repaint();
}

void HeatmapDisplay::drawSelection(){
    //qDebug()<<"drawSelection";
    BoolIterator blIt;
    RddtClust* currentClust = NULL;
    currentClust = this->m_dimClust->m_currentSelection;
    if(currentClust ==NULL){
        return;
    }
    blIt = currentClust->m_colSelected.begin();
    int inc = 0;
    double woffset = (double)1/(double)this->width();
    double hoffset = (double)1/(double)this->height();
    for(; blIt!=currentClust->m_colSelected.end(); ++blIt){
        if((*blIt)){
            double lefttop_x = (double)inc/((double)currentClust->m_colSelected.size());
            double lefttop_y = 0;
            double width = this->m_currentGridSize;
            double height = 1;
            unsigned long c = this->cm->getColor(ColorManager::BRUSH_SELECTED1);
            m_canvas->setForeground(c);
            m_canvas->drawRectangle(lefttop_x+woffset,lefttop_y+hoffset,
                                    width-2*woffset,height-2*woffset);
        }
        inc++;
    }

}

/*void HeatmapDisplay::setDimClust(DimRddtCluster* dimClust){
    this->m_dimClust = dimClust;
}*/

/*void HeatmapDisplay::drawSelection(){
    //this->m_dimClust->renderringHeatmap();
    //std::vector<Vec2> positions = this->m_dimClust->getPos();
    //std::vector<unsigned long> color = this->m_dimClust->m_heatmpViewColors;
    //std::vector<int> order = this->m_dimClust->getDimOrder();
    for(unsigned i = 0; i<color.size(); i++){
       // qDebug()<<"order["<<i<<"] "<<order[i]<<"\n";
      //  qDebug()<<"color "<<(color[order[i]]).R<<" "<<color[order[i]].G<<" "<<color[order[i]].B<<"\n";
      //  qDebug()<<"color "<<(color[i]).R<<" "<<color[i].G<<" "<<color[i].B<<"\n";
        if(color[ order[ i ] ] ==0){
            //transparent;
        }else{
            m_canvas->setForeground(color[order[i]]);
            double woffset = (double)1/(double)this->width();
            double hoffset = (double)1/(double)this->height();
            double lefttop_x = (double)i/(double)color.size();
            double lefttop_y = 0;
            double width = this->m_currentGridSize;
            double height = 1;
            m_canvas->drawRectangle(lefttop_x+woffset,lefttop_y+hoffset,
                                    width-2*woffset,height-2*woffset);
        }
        //m_canvas->drawPoint(positions[i].X,positions[i].Y,5);
    }
}*/
