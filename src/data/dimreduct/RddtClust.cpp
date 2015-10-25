#include <RInside.h> //this header file always comes at the first;
#include "main/XmdvTool.h" //this undefines free and realloc

#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"

#include "RddtClust.h"
#include "data/dimreduct/DataPartitions.h"
#include "data/dimreduct/DataPartition.h"
#include "data/multidim/OkcData.h"
#include "data/dimreduct/DimRddtCluster.h"

#include <cassert>
#include <iterator>
#include <string>
#include <QDebug>

RddtClust::RddtClust(){

}

RddtClust::RddtClust(DimRddtCluster* rddtOp)
    :m_rddtOp(rddtOp)
{

}

RddtClust::RddtClust(const RddtClust& x){
    this->m_k = x.m_k;
    this->m_nRddtDims = x.m_nRddtDims;
    this->m_rddtDims = x.m_rddtDims;
    this->m_order = x.m_order;
   // this->m_scores = x.m_scores;
    this->m_selected = x.m_selected;
    this->m_rddtOp = x.m_rddtOp;
    this->m_partitionPerVariable = x.m_partitionPerVariable;
}

RddtClust::RddtClust(IntVector selected, DimRddtCluster* rddtOp)
                     //std::vector<int> order,
                     //std::vector<double> scores,
                     //int metric)
    :m_selected(selected),m_rddtOp(rddtOp)
{
    doLocalDimOrder();    //this->m_dimRank, this->m_metricScore,
}

RddtClust::~RddtClust(){
    if(!m_partitionPerVariable.empty()){
        for(unsigned i = 0; i<m_partitionPerVariable.size(); i++){
            SAFE_DELETE(m_partitionPerVariable[i]);
        }
        m_partitionPerVariable.clear();
    }
    this->m_order.clear();
    this->m_selected.clear();
    this->m_paraMtx.clear();
}

void RddtClust::getDimData(DoubleVector& dimData, int idx){
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_rddtOp->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    //qDebug()<<"idx qt "<<idx;
    r["currentDim"] = idx;

    std::string cmd = "currentDim <- as.integer(currentDim)+1;\n"
            //"print(currentDim);\n"
            "dimData <- M[,currentDim];\n"
            "dimData";
    SEXP evals = r.parseEval(cmd);
    NumericVector rDimData(evals);

    dimData.clear();
    for(int i = 0; i< rDimData.length(); i++){
        dimData.push_back(rDimData[i]);
    }
}

void RddtClust::doLocalDimOrder(){
    switch(this->m_rddtOp->m_rankType){
    case DimRddtCluster::CENTER:
        this->doCenter();
        break;
    case DimRddtCluster::ENTROPY:

        break;
    case DimRddtCluster::OUTLIERS:
        this->doOutliers();
        break;
    default:

        break;
    }
 }

//save redudent dimensions and non-reduct dimensions into
// m_redctDims and m_nRedctDims
void RddtClust::pickKCandidates(int k){
    //qDebug()<<"selected "<<m_selected.size()<<" m_order.size "<<m_order.size();
    assert(m_selected.size()==m_order.size());
    this->m_k = k;
    //clear previous ranking result;
    this->m_nRddtDims.clear();
    this->m_rddtDims.clear();
    //unsigned p,q;
    int i=0;

    //int x = m_selected.size();

    for(unsigned p=0;p < this->m_order.size();p++){
        if(i<k){
            this->m_nRddtDims.push_back(m_selected[ m_order [ p ] ]);
            m_colSelected[p] = true;
        }else{
            this->m_rddtDims.push_back(m_selected[ m_order [ p] ]);
        }
        i++;
    }
}
void RddtClust::redoLocalAnalysis(){
    for(unsigned i = 0; i< this->m_partitionPerVariable.size(); i++){
        DataPartitions* par = dynamic_cast<DataPartitions*>(m_partitionPerVariable[i]);
        par->doLocalAnalysis();
    }
}


// do partitions on each variable selected in the redundent group
void RddtClust::doPartitions(){
    DoubleVector dimData;

    this->m_partitionPerVariable.clear();
    //m_OkcData->GetDimensionData(&dimData,);
    for(unsigned i = 0; i<this->m_nRddtDims.size(); i++){

        dimData.clear();
        this->getDimData(dimData,m_nRddtDims[i]);
        //this->m_rddtOp->getOkcData()->
                //GetDimensionData(dimData,m_nRddtDims[i]);

        DataPartitions* partitions = new DataPartitions();
        partitions->setRddtClust(this);
        partitions->setVarIdx(m_nRddtDims[i]);
        partitions->cutData(dimData);
        m_partitionPerVariable.push_back(partitions);
    }
    for(unsigned i = 0; i<this->m_rddtDims.size(); i++){
        dimData.clear();
        //this->m_rddtOp->getOkcData()->
                //GetDimensionData(dimData,m_rddtDims[i]);
        //dimData.clear();
        this->getDimData(dimData,m_rddtDims[i]);
        DataPartitions* partitions = new DataPartitions();
        partitions->setRddtClust(this);
        partitions->setVarIdx(m_rddtDims[i]);
        partitions->cutData(dimData);
        m_partitionPerVariable.push_back(partitions);
    }
}

//var should be the local index; the data dimension we partition on
void RddtClust::doSubSetting(int var, int parStart, int parEnd){
    m_activeInstances.clear();
    if(var<0 || var>= this->m_partitionPerVariable.size())
        return;
    DataPartitions* ps = dynamic_cast<DataPartitions*>
            (m_partitionPerVariable[var]);
    for(unsigned i = 0; i< ps->m_partitions.size(); i++){
        if(i>=parStart && i<=parEnd){
            DataPartition* p = dynamic_cast<DataPartition*>
                    (ps->m_partitions[i]);
            m_activeInstances.insert(m_activeInstances.end(),
                                     p->m_parIndices.begin(),p->m_parIndices.end());
        }
    }
    int size = m_activeInstances.size();
    std::sort(m_activeInstances.begin(),m_activeInstances.end());
}

void RddtClust::doOutliers(){
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_rddtOp->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    int dims = this->m_selected.size();
    if(dims<2)
        return;
    //int numSelected = this->m_selected.size();
    NumericVector nvSelected(dims);
    for(int i = 0; i<dims; i++){
        nvSelected[i] = m_selected[i];
    }
    r["selected"] = nvSelected;

    std::string rankcmd = "vars<-apply(M[,selected],2,sd);\n"
            "vars<-as.matrix(vars);\n"
            "rownames(vars)<-1:nrow(vars);\n"
            "vars<-as.matrix(vars[order(vars[,1],decreasing=TRUE),]);\n"
            "as.numeric(rownames(vars))-1"; //order in r starts from 0;

    SEXP evals =  r.parseEval(rankcmd);
    NumericVector rDimRank(evals);

    //ranking scores;
    std::string scorecmd="as.numeric(vars[,1])";
    evals = r.parseEval(scorecmd);
    NumericVector rScoreMtx(evals);

    //save dimension statistics: rank order, rank score and heatmap order;
    this->m_order.clear();
   // this->m_scores.clear();
    for(int i=0; i<dims; i++){
        int tmp = ((int)(rDimRank[i]));
        //this->m_dimRank;
        m_order.push_back(tmp);
       // double score = rScoreMtx[i];
        //m_scores.push_back(score);
    }
}

void RddtClust::doCenter(){
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_rddtOp->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    int dims = this->m_selected.size();

    int numSelected = this->m_selected.size();

    //no need for single element reordering;
    if(numSelected<2)
        return;
    NumericVector nvSelected(numSelected);
    for(int i = 0; i<numSelected; i++){
        nvSelected[i] = m_selected[i];
    }
    r["selected"] = nvSelected;
     //cmd = ;
    //r.parseEval(cmd);
    if(m_selected.size()<=1){
        assert(m_selected.size()>1);
    }
    std::string rcmd = "selected<-as.integer(selected)+1;\n"  //R index starts from 1;
            "seldist <- distances[selected,selected];\n"
            //"print(distances);\n"
            "centroid <- colSums(seldist);\n"
            "ord<-as.vector(order(centroid));\n"
            "ord-1";
    SEXP evals = r.parseEval(rcmd);
    NumericVector rCntrdOrder(evals);

    this->m_order.clear();
    //this->m_scores.clear();
    for(int i=0; i<dims; i++){
        int tmp = ((int)(rCntrdOrder[i]));
        m_order.push_back(tmp);
    }
    rcmd = "max(as.dist(seldist));\n";
    evals = r.parseEval(rcmd);
    m_worstFit = Rcpp::as<double>(evals);

    rcmd = "min(as.dist(seldist));\n";
    evals = r.parseEval(rcmd);
    m_bestFit = Rcpp::as<double>(evals);

    rcmd = "mean(as.dist(seldist));\n";
    evals = r.parseEval(rcmd);
    m_fitConsistency =  Rcpp::as<double>(evals);

    rcmd = //"print(seldist);\n"
            "max(seldist[,ord[1]]);\n";
    evals = r.parseEval(rcmd);
    m_fitRadius = Rcpp::as<double>(evals);
}

void RddtClust::doHeatmap(){
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_rddtOp->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    int dims = this->m_selected.size();
    if(dims==0)
        return;
    //m_colSelected.clear();
    NumericVector nvSelected(dims);
    for(int i = 0; i<dims; i++){
        nvSelected[i] = m_selected[i];
        //this->m_colSelected.push_back(false);
    }
    r["selected"] = nvSelected;

    // for heatmap view; this applies to any similarity metrics
    std::string clustOrdcmd =
            "selected<-as.integer(selected)+1;\n" //R index starts from 1;
            "seldist <- distances[selected,selected];\n"
            //"sdist<-apply(seldist,1,sort);\n"
            "seldist";
    SEXP evals = r.parseEval(clustOrdcmd);
    //NumericVector rClustOrd(evals);

    //evals = r.parseEval(paramValCmd);
    NumericMatrix rParamVal(evals);
    m_paraMtx.clear();
    for(int i = 0; i<dims; i++){
        for(int j = 0; j<dims; j++){
            double v = rParamVal(i,j);
            this->m_paraMtx.push_back(v);
        }
    }


    std::string ordcmd =
            "ordidx <- apply(seldist,1,order);\n"
            "ordidx-1";
    evals = r.parseEval(ordcmd);
    NumericMatrix rOrd(evals);
    this->m_paraOrdMtx.clear();
    for(int i = 0; i<dims; i++){
        for(int j = 0; j<dims; j++){
            int v = rOrd(i,j);
            this->m_paraOrdMtx.push_back(v);
        }
    }

    /*this->m_dimOrd.clear();
    for(int i=0; i<dims; i++){
        int ord = (int)(rClustOrd[i]);
        m_dimOrd.push_back(ord);
    }*/
}

//return value should be in [0,1], -1 indicates error
double RddtClust::getParamVal(int var, int neibNum){
    int ordSize = this->m_paraOrdMtx.size();
    int mtxSize = this->m_paraMtx.size();
    int dimsize = this->m_selected.size();
    if((unsigned int)(neibNum*dimsize+var) >= ordSize){//mtxSize){
        return -1.0; //problem with distance vector size;
    }
    int idx = m_paraOrdMtx[neibNum*dimsize+var];
    //if()
    if(idx>=mtxSize){
        return -1.0;
    }
    return m_paraMtx[idx*dimsize+var];
}

std::string RddtClust::getParamName(int var, int neibNum){

    int ordSize = this->m_paraOrdMtx.size();
    //int mtxSize = this->m_paraMtx.size();
    int dimsize = this->m_selected.size();

    if((unsigned int)(neibNum*dimsize+var) >= ordSize){//mtxSize){
        return ""; //problem with distance vector size;
    }

    int idx = m_paraOrdMtx[neibNum*dimsize+var];

    if(idx>=ordSize){
        return "";
    }

    //int locIdx = m_order[idx];
    int globIdx = m_selected[idx];
    if(globIdx<0 || globIdx>=this->m_rddtOp->m_dimName.size())
        return "";
    return this->m_rddtOp->m_dimName[globIdx];
}

void RddtClust::switchSelectionState(int horPos){
    //the dim is a local index of the data dimensions in the cluster;
    if(horPos < 0 || horPos >= (int)this->m_order.size() ||
            horPos>=(int)this->m_colSelected.size()){
        return;
    }
    int locIdx = this->m_order[horPos];
    if(locIdx<0 || locIdx>= (int)this->m_selected.size()){
        return;
    }

    bool state = this->m_colSelected[horPos];
    this->m_colSelected[horPos] = state^true;

    int globVar = this->m_selected[locIdx];

    IntIterator it = this->isNRddt(globVar);
    int glob = *it;
    if(it!=m_nRddtDims.end()){
        //then remove from nRddt
        m_rddtDims.push_back((*it));
        m_nRddtDims.erase(it);
        //add to rddt

        return;
    }

    it = isRddt(globVar);
    glob = *it;
    if(it!=m_rddtDims.end()){
        m_nRddtDims.push_back((*it));
        m_rddtDims.erase(it);
    }
    //need to update the user changed m_currentSelection back to m_clusters;
    //since m_currentSelection is a pointer, I think the operation should have
    //changed the data inside m_clusters;

}

void RddtClust::removeSelection(int horPos){
    if(horPos<0 || horPos>= (int)this->m_colSelected.size()){
        return;
    }

    if(horPos < 0 || horPos >= (int)this->m_order.size()){
        return;
    }
    int locIdx = this->m_order[horPos];

    if(locIdx<0 || locIdx>= (int)this->m_selected.size()){
        return;
    }

    //remove from colSelected;
    BoolIterator blIt;
    blIt = m_colSelected.begin();
    std::advance(blIt,horPos);
    m_colSelected.erase(blIt);

    //remove from m_selected;
    int globVar = this->m_selected[locIdx];
    IntIterator it = m_selected.begin();
    std::advance(it,locIdx);
    m_selected.erase(it);

    //remove from either m_nRddtDims or m_rddtDims; depends which it is in;
    it = isNRddt(globVar);
    if(it!=m_nRddtDims.end()){
        //then remove from nRddt
        m_nRddtDims.erase(it);
        //add to rddt
        //currentClust->m_rddtDims.push_back((*it));

    }
    it = isRddt(globVar);
    if(it!=m_rddtDims.end()){
        m_rddtDims.erase(it);
        //currentClust->m_nRddtDims.push_back((*it));
    }

    //doLocalDimOrder
    //reorder dims after removel;i
    if((int)this->m_selected.size()>1){
        this->doLocalDimOrder();
    }

    //add the removed into m_outliers;
    this->m_rddtOp->m_outliers.push_back(globVar);
    this->m_rddtOp->m_activeOutliers.push_back(false);
}


//check if gDim is in this->m_rddtDims
IntIterator RddtClust::isRddt(int gDim){
    IntIterator it;
    it = this->m_rddtDims.begin();
    for(;it!=this->m_rddtDims.end();++it){
        if((*it)==gDim)
            return it;
    }
    return m_rddtDims.end();
}

//check if gDim is in this->m_nRddtDims
IntIterator RddtClust::isNRddt(int gDim){
    IntIterator it;
    it = this->m_nRddtDims.begin();
    for(;it!=this->m_nRddtDims.end();++it){
        if((*it)==gDim)
            return it;
    }
    return m_nRddtDims.end();
}

/*void RddtClust::setRddtOperator(DimRddtCluster *rddtOp){
    this->m_rddtOp = rddtOp;
}*/

/*void RddtClust::setOkcData(OkcData* okcData){
    this->m_OkcData = okcData;
}

void RddtClust::setNBins(int bins){
    this->nBins = bins;
}

void RddtClust::setSimType(DimClusterMDSMap::SimilarityMetric type){

}

void RddtClust::setDist(std::vector<double> &dist){
    distMtx = dist;
}*/
/*//for(q=0;q < this->m_selected.size();q++){
    int t1 = m_order[p];
    int t2 = m_selected[q];
    //if(t1==t2){
        if(i < k){
            this->m_nRddtDims.push_back(m_selected[q]);
        }else{
            this->m_rddtDims.push_back(m_selected[q]);
        }
       // i++;
   // }
}*/
