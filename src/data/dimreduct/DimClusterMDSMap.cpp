/*
 * DimClusterMDSMap.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: kaiyuzhao
 */
#include <RInside.h>
//#include <Rcpp/CharacterVector.h>

#include "DimClusterMDSMap.h"

#include "data/multidim/OkcData.h"
#include "pipeline/PipelineManager.h"
#include "main/XmdvToolMainWnd.h"
#include "datatype/Vec2.h"
#include "data/dimreduct/RddtClust.h"
//#include "datatype/RGBt.h"

#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <QString>
#include <qDebug>

//#include "datatype/Vec2.h"

DimClusterMDSMap::DimClusterMDSMap()
    :dim_size(0),m_starting_dim(-1),eps(0.1),npts(1)
{
    normal = new RGBt(0.7,0.7,0.7,1.0); //grey redundent dims
    fade = new RGBt(0.0,0.0,1.0,1.0); //blue non_redundent
    selected = new RGBt(1.0,0.0,0.0,1.0); //red
    transparent = new RGBt(0.0,0.0,0.0,0.0);//transparent
    m_currentSelection = new RddtClust();
    // TODO Auto-generated constructor stub
    //m_dispCtrl = new ClustDispCtrl();
    //m_dispCtrl->setDimClust(this);

}

DimClusterMDSMap::DimClusterMDSMap(PipelineManager* pm)
    :dim_size(0),m_starting_dim(-1),eps(0.1),npts(1)
{
    // TODO Auto-generated constructor stub

    this->m_pm = pm;
    normal = new RGBt(0.7,0.7,0.7,1.0); //grey redundent dims
    fade = new RGBt(0.0,0.0,1.0,1.0); //blue non_redundent
    selected = new RGBt(1.0,0.0,0.0,1.0); //red
    transparent = new RGBt(0.0,0.0,0.0,0.0);//transparent
    m_currentSelection = new RddtClust();
    //m_dispCtrl = new ClustDispCtrl();
    //m_dispCtrl->setDimClust(this);
}

DimClusterMDSMap::~DimClusterMDSMap() {

    SAFE_DELETE(fade);
    SAFE_DELETE(normal);
    SAFE_DELETE(selected);
    SAFE_DELETE(transparent);
    SAFE_DELETE(m_currentSelection);

    // TODO Auto-generated destructor stub
    //SAFE_DELETE(m_dispCtrl);
    //this->m_dimOrd.clear();
    //this->m_dimRank.clear();
    //this->m_metricScore.clear();
}

void DimClusterMDSMap::setOkcData(OkcData* data){
    this->m_inputData = data;
}

OkcData* DimClusterMDSMap::getOkcData(){
    return this->m_inputData;
}

/**
 * @brief DimClusterMDSMap::constructClustMap this function
 * works for multiple distance metrics including pearson correlation
 * @param data
 */
void DimClusterMDSMap::computeInR(OkcData* data){
     XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
     RInside r = m_mwd->getRinstance();
    //this RInside object must be initialized before using any Rcpp functionalities
    std::vector<double> buf;
    int dims = data->getDimSize();
    dim_size = dims;
    int rows = data->getOrigDataSize();

    //initialize Rcpp matrix structure, this structure can be applied to
    //R algorithm directly;
    Rcpp::NumericMatrix M(rows,dims);

    for (int i=0; i<dims; i++) {
        data->GetDimensionData(buf,i);
        for (int j=0; j<rows; j++) {
            M(j,i) = buf[j];
        }
    }
    //this operation applies the matrix M in cpp to
    //R object M
    r["M"] = M;

    //spearsman correlation part;
    std::string rcmd =
            "testz <- cor(x=M,method=\"spearman\");\n"
            "dists <- (1-testz);\n"
            "dis <-as.dist(dists);\n"
            "fit <- cmdscale(dis,eig=TRUE, k=2);\n"
            "p<-(fit$points+1)/2;\n" //normalize to 0-1
            "p"; //fit is the result returned by parseEval;
    SEXP eval = r.parseEval(rcmd);
    Rcpp::NumericMatrix dimPos(eval);

    int nrows = dimPos.nrow();
    dim_layout_pos.clear();
    dim_name.clear();
    for (int i = 0; i < nrows; i++){
        double x = dimPos(i,0);
        double y = dimPos(i,1);
        Vec2 pos( x , y );
        dim_layout_pos.push_back(pos);
        dim_name.push_back(std::string(data->names[i]));
    }


    std::string rdistcmd = "as.matrix(dists/2)\n";
    eval =  r.parseEval(rdistcmd);
    Rcpp::NumericMatrix distRmtx(eval);
    disMtx.clear();
    for (int i=0; i<dims; i++) {
        for (int j=0; j<dims; j++) {
            disMtx.push_back(distRmtx(i,j));
        }
    }

    // dimension ranking part; using sd by default
    std::string rankcmd="vars<-apply(M,2,sd);\n"
            "vars<-as.matrix(vars);\n"
            "rownames(vars)<-1:nrow(vars);\n"
            "vars<-as.matrix(vars[order(vars[,1],decreasing=TRUE),]);\n"
            "as.numeric(rownames(vars))-1"; //order in r starts from 0;

    eval =  r.parseEval(rankcmd);
    Rcpp::NumericVector rDimRank(eval);
    //charactervector might not work
    //std::vector<std::string> buff =
           // Rcpp::as< std::vector< std::string> >(dimOrder);
    std::string scorecmd="as.numeric(vars[,1])";
    eval = r.parseEval(scorecmd);
    Rcpp::NumericVector rScoreMtx(eval);

    std::string clustOrdcmd ="sdist<-apply(dists/2,1,sort);\n"
            "rddcy<-colSums(sdist);\n"
            "order(rddcy);\n";
    eval = r.parseEval(clustOrdcmd);
    Rcpp::NumericVector rClustOrd(eval);

    this->m_dimRank.clear();
    this->m_metricScore.clear();
    this->m_dimOrd.clear();
    //this->m_dimIdx.clear();
    //for(int j = 0; j< dims; j++){
        //m_dimIdx.push_back(0);
    //}

    //qDebug()<<"m_dimIdx size "<<m_dimIdx.size();
    for(int i=0; i<dims; i++){
        int tmp = ((int)(rDimRank[i]));
        m_dimRank.push_back(tmp);
        double score = rScoreMtx[i];
        m_metricScore.push_back(score);
        int ord = (int)(rClustOrd[i])-1; //index in R starting from 1;
        m_dimOrd.push_back(ord);
        qDebug()<<ord<<" ord";
        //m_dimIdx[ord] = i;
    }

    std::string paramValCmd = "sdist";
    eval = r.parseEval(paramValCmd);
    Rcpp::NumericMatrix rParamVal(eval);
    paraMtx.clear();
    for(int i = 0; i<dims; i++){
        for(int j = 0; j<dims; j++){
            double v = rParamVal(i,j);
            this->paraMtx.push_back(v);
        }
    }
    //clear enviornment
    r.parseEval("rm(list = ls(all = TRUE))");
}

std::vector<Vec2> DimClusterMDSMap::getPos(){
    return this->dim_layout_pos;
}

std::vector<std::string> DimClusterMDSMap::getNames(){
    return this->dim_name;
}

double DimClusterMDSMap::getDistance(int r, int c){
    int dimsize = this->getNames().size();
    if((unsigned int)(r*dimsize+c) >= this->disMtx.size()){
        return -1.0; //problem with distance vector size;
    }
    return disMtx[r*dimsize+c];
}

//return value should be in [0,1], -1 indicates error
double DimClusterMDSMap::getParamVal(int var, int neibNum){
    int dimsize = this->getNames().size();
    if((unsigned int)(neibNum*dimsize+var) >= this->paraMtx.size()){
        return -1.0; //problem with distance vector size;
    }
    return paraMtx[neibNum*dimsize+var];
}

std::vector<double> DimClusterMDSMap::getDistance(){
    return this->disMtx;
}

int DimClusterMDSMap::getDimSize(){
    return this->dim_size;
}

std::vector<int> DimClusterMDSMap::getDimOrder(){
    return this->m_dimOrd;
}
std::vector<int> DimClusterMDSMap::getDimIdx(){
    return this->m_dimIdx;
}

void DimClusterMDSMap::markRdctGroup(int dim){//markRedudencyGroup
    //the eps and npts are controlled by the user
    if(dim>-1){
        neighbours.clear(); //clear the previous selection;
        std::vector<bool> visited;
        unsigned int disMtxSize = this->getDimSize();
        unsigned int i;
        for(i=0;i<disMtxSize;i++){
            visited.push_back(0);
        }
        find_neighbours(neighbours,dim,
                        this->getDistance(),
                        visited,
                        this->eps, //to be controlled by user interface;
                        this->npts); // to be controlled by user interface;
        if(!neighbours.empty()){
          std::sort(neighbours.begin(), neighbours.end());
        }
        for(unsigned i = 0; i<neighbours.size(); i++){
            qDebug()<<"neighbours "<<neighbours[i];
        }
    }
}

void DimClusterMDSMap::find_neighbours(std::vector<int> &neighbs,
                                         int idx, std::vector<double> dist,
                                         std::vector<bool> &visited,
                                         double eps, unsigned int npts){
    //unsigned int dim;
    //dim = this->m_dimClust->getDimSize();
    if(!visited[idx]){
        visited[idx]=1;
        std::vector<int> region = findRegion(idx,dist,eps);
        if(region.size()>=npts){
            neighbs.push_back(idx);
        }
        unsigned int j;
        for(j=0;j<region.size();j++){
            int exp_idx = region[j];
            if(!visited[exp_idx]){
                visited[exp_idx]=1;
                std::vector<int> exp_region = findRegion(exp_idx,dist,eps);
                if(exp_region.size()>=npts){
                    neighbs.push_back(exp_idx);
                    unsigned int k;
                    for(k=0;k<exp_region.size();k++){
                        region.push_back(exp_region[k]);
                    }
                }
            }
        }
    }
}

std::vector<int> DimClusterMDSMap::findRegion(int idx,
                                                std::vector<double> dist, double eps){
    std::vector<int> temp;
    unsigned int i,dim;
    dim = this->getDimSize();
    for(i=0;i<dim;i++){
        if(dist[dim*idx+i] < eps && (unsigned int)idx!=i){//
            temp.push_back(i);
        }
    }
    return temp;
}

void DimClusterMDSMap::refreshSelection(){
    if(m_currentView.empty()){
        return;
    }

    unsigned size = this->m_currentView.size();
    RddtClust lastSelection = this->m_currentView.at(size-1);

    std::vector<int> last_nRddtDims = lastSelection.m_nRddtDims;
    std::vector<int> last_rddtDims = lastSelection.m_rddtDims;

    std::sort(last_nRddtDims.begin(), last_nRddtDims.end());
    if(this->m_nRddt_all.empty()){
        m_nRddt_all = last_nRddtDims;
    }else{
        std::vector<int> tmp(m_nRddt_all.size()+last_nRddtDims.size());
        std::set_union(last_nRddtDims.begin(), last_nRddtDims.end(),
                       m_nRddt_all.begin(), m_nRddt_all.end(), tmp.begin());
        m_nRddt_all.clear();
        m_nRddt_all = tmp;
    }

    std::sort(last_rddtDims.begin(),last_rddtDims.end());
    if(this->m_rddt_all.empty()){
        m_rddt_all = last_rddtDims;
    }else{
        std::vector<int> tmp(m_rddt_all.size()+last_rddtDims.size());
        std::set_union(last_rddtDims.begin(),last_rddtDims.end(),
                       m_rddt_all.begin(),m_rddt_all.end(),tmp.begin());
        m_rddt_all.clear();
        m_rddt_all = tmp;
    }
    //last step is to remove all items that appear in non_redundent dimension
    //list from redundent dim list, to make sure the two lists are disjoint, so that
    //different colors can be applied to the two lists of dimensions
    for(unsigned i = 0; i<m_rddt_all.size(); i++){
        for(unsigned j = 0; j<m_nRddt_all.size(); j++){
            if(m_rddt_all[i]==m_nRddt_all[j]){
                //remove m_rddt_all[i]
                m_rddt_all.erase(m_rddt_all.begin()+i);
                i--;
            }
        }
    }
}


void DimClusterMDSMap::refreshCurrentSelection(){

}

void DimClusterMDSMap::formNewCluster(){
    SAFE_DELETE(m_currentSelection);
    this->m_currentSelection = new RddtClust(neighbours,
                         this->m_dimRank,
                            this->m_metricScore, 0);
}

void DimClusterMDSMap::renderringHeatmap(){
    m_heatmpViewColors.clear();
    int dimSize = this->getDimSize();
    m_heatmpViewColors.resize(dimSize);
    this->renderringMDSview();

    for(unsigned i = 0; i< m_heatmpViewColors.size(); i++){
        m_heatmpViewColors[i] = this->m_MDSViewColors[i];
    }

    /*for(unsigned i = 0; i<dimSize;i++){
        if(m_MDSViewColors[i].R== 0.7 &&
                m_MDSViewColors[i].G == 0.7 &&
                m_MDSViewColors[i].B == 0.7 &&
                m_MDSViewColors[i].A ==1){
                //nothing
        }else{
            m_heatmpViewColors[i] = this->m_MDSViewColors[i];
        }

    }*/
    /*qDebug()<<"heatmap selected: "<<neighbours.size()<<"\n";
    qDebug()<<"heatmap selected: "<<m_nRddt_all.size()<<"\n";
    qDebug()<<"heatmap selected: "<<m_rddt_all.size()<<"\n";
    if(neighbours.empty() && m_nRddt_all.empty() && m_rddt_all.empty()){

    }

    std::vector<int> last_nRddtDims;
    std::vector<int> last_rddtDims;
    if(this->m_currentSelection!=NULL){
        last_nRddtDims = m_currentSelection->m_nRddtDims;
        std::sort(last_nRddtDims.begin(),last_nRddtDims.end());

        last_rddtDims = m_currentSelection->m_rddtDims;
        std::sort(last_rddtDims.begin(),last_rddtDims.end());
    }

    //index for non-redundent list of all dimensions,
    //redundent list of all dimensions,current selection of non-redundent,
    //current selectoin of redundent
    unsigned nrddt_i,rddt_i,c_nrddt_i,c_rddt_i;
    nrddt_i = 0; rddt_i = 0; c_nrddt_i = 0; c_rddt_i=0;
    //first draw non-redundent, current selection,
    //then draw redundent current selection,
    //last draw non-redundent previous selection,
    //and redundent previous selection
    for(unsigned i = 0; i<dimSize;i++){
        if(c_nrddt_i<last_nRddtDims.size() && i==last_nRddtDims[c_nrddt_i]){
            //overide the color of this dimension, by the current selection
            m_heatmpViewColors[i] = *(this->selected);
            //m_canvas->setForeground();
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            c_nrddt_i++;
            if(nrddt_i < m_nRddt_all.size() && i==m_nRddt_all[nrddt_i]){
                nrddt_i++;
            }
            if(rddt_i < m_rddt_all.size() && i==m_rddt_all[rddt_i]){
                rddt_i++;
            }
        }else if(c_rddt_i<last_rddtDims.size() && i==last_rddtDims[c_rddt_i]){
            //overide the color of this dimension, by the current selection
            m_heatmpViewColors[i] = *(this->fade);
            //m_canvas->setForeground(*(this->fade));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            c_rddt_i++;
            if(nrddt_i < m_nRddt_all.size() && i==m_nRddt_all[nrddt_i]){
                nrddt_i++;
            }
            if(rddt_i < m_rddt_all.size() && i==m_rddt_all[rddt_i]){
                rddt_i++;
            }
        }else if(nrddt_i < m_nRddt_all.size() && i==m_nRddt_all[nrddt_i]){
            m_heatmpViewColors[i] = *(this->selected);
            //m_canvas->setForeground(*(this->selected));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            nrddt_i++;
        }else if(rddt_i < m_rddt_all.size() && i == m_rddt_all[rddt_i]){
            m_heatmpViewColors[i] = *(this->fade);
            //m_canvas->setForeground(*(this->fade));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            rddt_i++;
        }else{ //unexplored dimensions;
            m_heatmpViewColors[i] = *(this->transparent);
            //m_canvas->setForeground(*(this->normal));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
        }
    }*/
}

void DimClusterMDSMap::renderringMDSview(){

    m_MDSViewColors.clear();
    int dimSize = this->getDimSize();
    m_MDSViewColors.resize(dimSize);

    if(neighbours.empty() && m_nRddt_all.empty() && m_rddt_all.empty()){
        //m_canvas->setForeground(*(this->normal));
        //m_canvas->drawPoints(poss,5);
        for(unsigned i = 0; i< m_MDSViewColors.size(); i++){
            m_MDSViewColors[i] = *(this->normal);
        }
        return;
    }
    //qDebug()<<"MDS selected: "<<neighbours.size()<<"\n";
    //qDebug()<<"MDS selected: "<<m_nRddt_all.size()<<"\n";
    //qDebug()<<"MDS selected: "<<m_rddt_all.size()<<"\n";
    std::vector<int> last_nRddtDims;
    std::vector<int> last_rddtDims;
    if(this->m_currentSelection!=NULL){
        last_nRddtDims = m_currentSelection->m_nRddtDims;
        std::sort(last_nRddtDims.begin(),last_nRddtDims.end());

        last_rddtDims = m_currentSelection->m_rddtDims;
        std::sort(last_rddtDims.begin(),last_rddtDims.end());
    }

    //index for non-redundent list of all dimensions,
    //redundent list of all dimensions,current selection of non-redundent,
    //current selectoin of redundent
    unsigned nrddt_i,rddt_i,c_nrddt_i,c_rddt_i;
    nrddt_i = 0; rddt_i = 0; c_nrddt_i = 0; c_rddt_i=0;
    //first draw non-redundent, current selection,
    //then draw redundent current selection,
    //last draw non-redundent previous selection,
    //and redundent previous selection
    for(int i = 0; i<dimSize;i++){
        if(c_nrddt_i<last_nRddtDims.size() && i==last_nRddtDims[c_nrddt_i]){
            //overide the color of this dimension, by the current selection
            m_MDSViewColors[i] = *(this->selected);
            //m_canvas->setForeground();
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            c_nrddt_i++;
            if(nrddt_i < m_nRddt_all.size() && i==m_nRddt_all[nrddt_i]){
                nrddt_i++;
            }
            if(rddt_i < m_rddt_all.size() && i==m_rddt_all[rddt_i]){
                rddt_i++;
            }
        }else if(c_rddt_i<last_rddtDims.size() && i==last_rddtDims[c_rddt_i]){
            //overide the color of this dimension, by the current selection
            m_MDSViewColors[i] = *(this->fade);
            //m_canvas->setForeground(*(this->fade));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            c_rddt_i++;
            if(nrddt_i < m_nRddt_all.size() && i==m_nRddt_all[nrddt_i]){
                nrddt_i++;
            }
            if(rddt_i < m_rddt_all.size() && i==m_rddt_all[rddt_i]){
                rddt_i++;
            }
        }else if(nrddt_i < m_nRddt_all.size() && i==m_nRddt_all[nrddt_i]){
            m_MDSViewColors[i] = *(this->selected);
            //m_canvas->setForeground(*(this->selected));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            nrddt_i++;
        }else if(rddt_i < m_rddt_all.size() && i == m_rddt_all[rddt_i]){
            m_MDSViewColors[i] = *(this->fade);
            //m_canvas->setForeground(*(this->fade));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
            rddt_i++;
        }else{ //unexplored dimensions;
            m_MDSViewColors[i] = *(this->normal);
            //m_canvas->setForeground(*(this->normal));
            //m_canvas->drawPoint(poss[i].X,poss[i].Y);
        }
    }
}
