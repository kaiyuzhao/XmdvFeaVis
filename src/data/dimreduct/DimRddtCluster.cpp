/*
 * DimClusterMDSMap.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: kaiyuzhao
 */
#include <RInside.h> //this header file always comes at the first;
#include "main/XmdvTool.h" //this undefines free and realloc
#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"

#include "DimRddtCluster.h"
#include "data/multidim/OkcData.h"

#include "datatype/Vec2.h"


#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <QString>
#include <qDebug>
#include <cassert>

//#include "datatype/Vec2.h"

/*DimRddtCluster::DimRddtCluster()
    :dim_size(0),nBins(10),m_starting_dim(-1),eps(0.1),npts(1)
{
    cm = g_globals.colorManager;
    normal = new RGBt(0.7,0.7,0.7,1.0); //grey redundent dims
    fade = new RGBt(0.0,0.0,1.0,1.0); //blue non_redundent
    selected = new RGBt(1.0,0.0,0.0,1.0); //red
    transparent = new RGBt(0.0,0.0,0.0,0.0);//transparent
    m_currentSelection = new RddtClust();
    //default similarity metric;
    simType = SPEARMAN_CORRELATION;
    binType = EQUAL_FREQUENCY;
    rankType = CENTER;
    // TODO Auto-generated constructor stub
    //m_dispCtrl = new ClustDispCtrl();
    //m_dispCtrl->setDimClust(this);
}*/

DimRddtCluster::DimRddtCluster(PipelineManager* pm)
    :m_nBins(10),m_dimSize(0),m_startingDim(-1),
      m_height(0.2),m_heightInt(-1),
      m_numClust(0),m_numClustInt(-1)
    //m_currentSelectionIdx(-1)
{
    // TODO Auto-generated constructor stub
    m_cm = g_globals.colorManager;
    this->m_pm = pm;
    //normal = new RGBt(0.7,0.7,0.7,1.0); //grey redundent dims
    //fade = new RGBt(0.0,0.0,1.0,1.0); //blue non_redundent
    //selected = new RGBt(1.0,0.0,0.0,1.0); //red
    //transparent = new RGBt(0.0,0.0,0.0,0.0);//transparent
    m_currentSelection = NULL;
    m_simType = SPEARMAN_CORRELATION;
    m_binType = EQUAL_FREQUENCY;
    m_rankType = CENTER;
}


DimRddtCluster::~DimRddtCluster() {
    //SAFE_DELETE(fade);
    //SAFE_DELETE(normal);
    //SAFE_DELETE(selected);
    //SAFE_DELETE(transparent);
    SAFE_DELETE(m_currentSelection);
}

void DimRddtCluster::updateAllViews(){
    emit sigUpdateAllViews();
}



void DimRddtCluster::setOkcData(OkcData* data){
    this->m_inputData = data;
}


OkcData* DimRddtCluster::getOkcData(){
    return this->m_inputData;
}

void DimRddtCluster::doDataFiltering(){
    if(this->m_currentSelection==NULL)
        return;
    if(this->m_currentSelection->m_activeInstances.empty())
        return;
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
    //this RInside object must be initialized before using any Rcpp functionalities
    RInside r = m_mwd->getRinstance();

    int size = this->m_currentSelection->m_activeInstances.size();
    NumericVector instanceIdx(size);
    for(int i = 0; i<size; i++){
        instanceIdx[i] = this->m_currentSelection->m_activeInstances[i];
    }
    r["selectInst"] = instanceIdx;
    std::string rcmd = "M <- M[selectInst,]";
    r.parseEval(rcmd);

    //save current selected data points
    this->m_activeInstances = this->m_currentSelection->m_activeInstances;
    //m_currentSelection = NULL;
}

void DimRddtCluster::restoreDataFiltering(){
    initDataInR();
}
void DimRddtCluster::initDataInR(){
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
    //this RInside object must be initialized before using any Rcpp functionalities
    RInside r = m_mwd->getRinstance();
    DoubleVector buf;
    int dims = this->m_inputData->getDimSize();
    this->m_dimSize = dims;
    int rows = this->m_inputData->getOrigDataSize();
    this->m_dimName.clear();
    for(int i = 0; i<dims;i++){
        m_dimName.push_back(std::string(m_inputData->names[i]));
    }
    //initialize Rcpp matrix structure, this structure can be applied to
    //R algorithm directly;
    NumericMatrix M(rows,dims);

    //copy data into Rcpp data structure
    for (int i=0; i<dims; i++) {
        m_inputData->GetDimensionData(buf,i);
        for (int j=0; j<rows; j++) {
            M(j,i) = buf[j];
        }
    }
    //this operation applies the matrix M in cpp to
    //R object M
    r["M"] = M;
}

void DimRddtCluster::computeInR(){

    //switching between different metrics;
    switch(this->m_simType){
    case DimRddtCluster::CROSS_ENTROPY:
        //doCrossEntropy()
        break;
    case DimRddtCluster::SPEARMAN_CORRELATION:
        this->doSpearman();
        break;
    case DimRddtCluster::STATISTICS:
        //using k order of moment
        this->doStatistics();
        break;
    default:
        break;
    }
    this->m_currentSelection = NULL;
    this->doClustering(m_height,m_numClust);
    this->doRanking(1); //this may be tweeked by the user;
    this->doLayout(); //compute MDS layout of outliers+clusters
    this->refreshCurrentList();
    if(0<(int)this->m_clusters.size()){
        //draw selected cluster;
        this->doSelectedClust(0);
    }
    this->updateAllViews();
}

void DimRddtCluster::doStatistics(){
    using namespace Rcpp;
    int dims = this->m_dimSize;
    //spearsman correlation part;
    XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    std::string rcmd = "mins<-apply(M,2,min);\n"
            "maxs<-apply(M,2,max);\n"
            "norm<-apply(M,1,'-',mins);\n"
            "norm<-t(norm*(1/(maxs-mins)));\n"
            "mu <- colMeans(norm);\n"
            "kthmom2<-apply(norm,2,sd);\n"
            "norm <- t(apply(norm,1,'-',mu));\n"
            "kthmom3<-colMeans(norm^3);\n"
            "kthmom4<-colMeans(norm^4);\n"
            "stat<-cbind(mu,kthmom2);\n"
            "stat<-cbind(stat,kthmom3);\n"
            "stat<-cbind(stat,kthmom4);\n"
            "distances<-dist(stat);\n"
            "distances<-as.matrix(distances);\n"
            "distances";
    SEXP evals =  r.parseEval(rcmd);
    NumericMatrix distRmtx(evals);
    m_disMtx.clear();
    for (int i=0; i<dims; i++) {
        for (int j=0; j<dims; j++) {
            double d = distRmtx(i,j);
            m_disMtx.push_back(d);
        }
    }
}

//compute the similarity matrix using spearman metric
void DimRddtCluster::doSpearman(){
    using namespace Rcpp;
    int dims = this->m_dimSize;
    //spearsman correlation part;
    XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    std::string rcmd =
            //"require(flexmix);\n"
            "testz <- cor(x=M,method=\"spearman\");\n"
            "testz[is.na(testz)] <- 0;\n"
            "distances <- (1-abs(testz));\n"
            "distances";

    SEXP evals =  r.parseEval(rcmd);
    NumericMatrix distRmtx(evals);
    m_disMtx.clear();
    for (int i=0; i<dims; i++) {
        for (int j=0; j<dims; j++) {
            double d = distRmtx(i,j);
            m_disMtx.push_back(d);
        }
    }
}

Vec2Vector DimRddtCluster::getPos(){
    return this->m_dimLayoutPos;
}
void DimRddtCluster::updateClustDistParas(int val){
    //by default the range is 0 to 99;
    //convert it to 0-1

    this->m_currentSelection = NULL;
    this->m_currentSelectionIdx = -1;
    double dist = (double)val/(double)99;
    int step = dist*30.0;//30 steps on the slider bar;
    if(this->m_heightInt!=step){
        if(!mutex.tryLock())
            return;
        this->m_height = dist;
        this->doClustering(this->m_height,this->m_numClust);
        this->doRanking(1); //this may be tweeked by the user;
        this->doLayout(); //compute MDS layout of outliers+clusters
        this->refreshCurrentList();
        this->updateAllViews();
        m_heightInt = step;
        mutex.unlock();
    }
}

void DimRddtCluster::updateClustMptsParas(int val){
    //by default the range is 0 to 99;
    //convert it to 1 - 10, 4 is magic number
    try
    {

        this->m_currentSelection = NULL;
        this->m_currentSelectionIdx = -1;

        double n = (double)val/(double)99;
        this->m_numClust = (int)(n*(double)this->getDimSize());
        if(this->m_numClustInt!=m_numClust){
            if(!mutex.tryLock())
                return;
            this->doClustering(this->m_height,this->m_numClust);
            this->doRanking(1); //this may be tweeked by the user;
            this->doLayout(); //compute MDS layout of outliers+clusters
            this->refreshCurrentList();
            this->updateAllViews();
            this->m_numClustInt = m_numClust;
            mutex.unlock();
        }
        return;
    }
    catch (int e)
    {
        qDebug() << "An exception occurred. Exception Nr. " << e << endl;
        return;
    }
}

double DimRddtCluster::getDistance(int r, int c){
    //int dimsize = this->dim_size;
    if((unsigned int)(r*m_dimSize+c) >= this->m_disMtx.size()){
        return -1.0; //problem with distance vector size;
    }
    return m_disMtx[r*m_dimSize+c];
}

int DimRddtCluster::getDimSize(){
    return this->m_dimSize;
}

void DimRddtCluster::findNeighbours(IntVector &neighbs,
                                    int idx, DoubleVector dist,
                                    BoolVector &visited,
                                    double eps, unsigned int npts){
    //unsigned int dim;
    //dim = this->m_dimClust->getDimSize();
    if(!visited[idx])
    {
        visited[idx]=1;
        IntVector region;
        findRegion(region,idx,dist,eps);
        if(region.size()>=npts){
            neighbs.push_back(idx);
        }
        unsigned int j;
        for(j=0;j<region.size();j++)
        {
            int exp_idx = region[j];
            if(!visited[exp_idx])
            {
                visited[exp_idx]=1;
                IntVector exp_region;
                findRegion(exp_region,exp_idx,dist,eps);
                if(exp_region.size()>=npts)
                {
                    neighbs.push_back(exp_idx);
                    unsigned int k;
                    for(k=0;k<exp_region.size();k++){
                        region.push_back(exp_region[k]);
                    }
                }
            }
        }
        //sort the cluster before outputting
        if(!neighbs.empty()){
            std::sort(neighbs.begin(), neighbs.end());
        }
    }
}

void DimRddtCluster::findRegion(IntVector &result, int idx,
                                DoubleVector dist, double eps){
    unsigned int i,dim;
    dim = this->m_dimSize;
    result.clear();
    for(i=0;i<dim;i++){
        if(dist[dim*idx+i] < eps && (unsigned int)idx!=i){//
            result.push_back(i);
        }
    }
}

void DimRddtCluster::doClustering(double treeHeight, int numClusts){
    //clear calculation result from previous result;
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();

    try
    {
        if(!m_clusters.empty()){
            std::vector<RddtClust*>::iterator cit;
            std::vector<RddtClust*>::iterator end;

            cit = m_clusters.begin();
            end = m_clusters.end();

            for(;cit!=end;++cit){
                SAFE_DELETE(*(cit));
            }
        }
        //int num = 0;
        //num = (int)(numClusts*(double)this->getDimSize());
        m_clusters.clear();
        m_outliers.clear();

        r["treeHeight"] = treeHeight;
        r["numClusts"] = numClusts;

        std::string rcmd;
        if(numClusts==0){
            rcmd = "clusts<-hclust(as.dist(distances),method=\"ward\",members=NULL);\n"
                    "clusts$height <- (clusts$height-min(clusts$height))/(max(clusts$height)-min(clusts$height));\n"
                    "cutclust <- cutree(clusts,h = treeHeight);\n"
                    //"print(cutclust-1);\n"
                    "cutclust-1";//list of dimensions belong to different clusters;
        }else{
            rcmd = "clusts<-hclust(as.dist(distances),method=\"ward\",members=NULL);\n"
                    "clusts$height <- (clusts$height-min(clusts$height))/(max(clusts$height)-min(clusts$height));\n"
                    "cutclust <- cutree(clusts,h = treeHeight, k=numClusts);\n"
                    //"print(cutclust-1);\n"
                    "cutclust-1";//list of dimensions belong to different clusters;
        }
        SEXP evals = r.parseEval(rcmd);
        DoubleVector dimMembers = Rcpp::as< std::vector<double> >(evals);

        rcmd = "max(cutclust);\n";
        evals = r.parseEval(rcmd);
        int num = Rcpp::as<int>(evals);

        // qDebug()<<"num "<<num;

        for(int i = 0; i< num; i++){
            RddtClust* newClust = new RddtClust(this);
            this->m_clusters.push_back(newClust);
        }

        DoubleIterator dit = dimMembers.begin();
        int i = 0;
        for(;dit!=dimMembers.end(); ++dit){
            int clustId = *dit;
            //qDebug()<<"clustId "<<clustId;
            assert(clustId<num);
            RddtClust* c = dynamic_cast<RddtClust*>(m_clusters[clustId]);
            if(c!=NULL){
                c->m_selected.push_back(i);
            }else{
                return;
            }
            i++;
        }

        cleanCluster();

        for(int i = 0; i< m_clusters.size(); i++){
            RddtClust* c = dynamic_cast<RddtClust*>(m_clusters[i]);
            if(c!=NULL){
                c->doLocalDimOrder();
            }else{
                qDebug()<<"problem here";
            }
        }
        std::sort(m_clusters.begin(),m_clusters.end(),clustSortComp);
        initializeActiveList();
    }
    catch (int e)
    {
        qDebug() << "An exception occurred. Exception Nr. " << e<<"";
        return;
    }
}

void DimRddtCluster::cleanCluster(){

    std::vector<RddtClust*>::iterator cit = this->m_clusters.begin();
    //std::vector<RddtClust*>::iterator end = this->m_clusters.end();
    if(m_clusters.empty()){
        return;
    }
    int i = 0;
    for(; cit<this->m_clusters.end(); ){
        RddtClust* c = *(cit);
        //qDebug()<<&c->m_selected;
        if(c==NULL){
            cit = this->m_clusters.erase(cit);
            SAFE_DELETE(c);
            //this->m_outliers.push_back();
        }else if(c->m_selected.size()==1){ //m_selected pointed to 0x4
            int idx = c->m_selected[0];
            cit = this->m_clusters.erase(cit);
            SAFE_DELETE(c);
            this->m_outliers.push_back(idx);
            //qDebug()<<"delete "<<i;
        }else if(c->m_selected.size()==0){
            //should not be here
            cit = this->m_clusters.erase(cit);
            SAFE_DELETE(c);
            //++cit;
        }else{
            ++cit;
        }
        i++;
    }
}

void DimRddtCluster::refreshCurrentList(){
    if(m_clusters.size()!=m_activeClusters.size()){
        assert(m_clusters.size()!=m_activeClusters.size());
    }
    this->m_currentList.clear();
    for(unsigned i = 0; i<this->m_clusters.size(); i++){
        if(m_activeClusters[i]){
            IntIterator it;
            RddtClust* clust = dynamic_cast<RddtClust*>(m_clusters[i]);
            it = m_currentList.end();
            m_currentList.
                    insert(it,clust->m_nRddtDims.begin(),clust->m_nRddtDims.end());
        }
    }
    if(m_activeOutliers.size() != m_outliers.size()){
        assert(m_activeOutliers.size() == m_outliers.size());
    }
    //assert();
    for(unsigned i = 0; i<this->m_activeOutliers.size(); i++){
        if(m_activeOutliers[i]){
            m_currentList.push_back(m_outliers[i]);
        }
    }
    sort(m_currentList.begin(),m_currentList.end());
    IntIterator tmp = unique( m_currentList.begin(), m_currentList.end() );
    m_currentList.erase(tmp,  m_currentList.end() );
}

void DimRddtCluster::initializeActiveList(){
    this->m_activeOutliers.clear();
    this->m_activeClusters.clear();

    std::vector<RddtClust*>::iterator it;
    it = this->m_clusters.begin();

    IntIterator oIt;
    oIt = this->m_outliers.begin();

    for(;it!=m_clusters.end();++it){
        this->m_activeClusters.push_back(true);
    }

    for(;oIt!=m_outliers.end();++oIt){
        this->m_activeOutliers.push_back(true);
    }
}

void DimRddtCluster::refreshClusters(){
    //if only one element left then destruct this cluster;
    //erase this cluster from m_clusters in dimClust;
    std::vector<RddtClust*>::iterator it;

    it = m_clusters.begin();

    BoolIterator blIt;
    blIt = this->m_activeClusters.begin();
    int i = 0;
    while(i< m_clusters.size() && it!=m_clusters.end() && blIt!= m_activeClusters.end()){
        RddtClust* currentClust = (*it);
        if(currentClust->m_selected.size()>1){
            ++it; ++blIt;
            continue;
        }
        //remove the last element as well if there are only two element before removel;
        //add the last element into m_outliers;
        int last = currentClust->m_selected.back();
        currentClust->m_selected.clear();
        currentClust->m_colSelected.clear();
        this->m_outliers.push_back(last);
        //since it was removed from a cluster,
        //it is not active by default;
        this->m_activeOutliers.push_back(false);
        //clean up
        SAFE_DELETE(this->m_currentSelection);
        this->m_currentSelection=NULL;
        //this->m_currentSelectionIdx = -1;
        it = this->m_clusters.erase(it);
        SAFE_DELETE(currentClust);
        blIt = this->m_activeClusters.erase(blIt);
        i++;
    }
    this->doLayout();
    this->refreshCurrentList();
}


void DimRddtCluster::generateDimReductResult(OkcData* okcdata){
    // the data size of the new dataset is equal to that of input dataset
    //by default
    int data_size = this->getOkcData()->getDataSize();
    /*if(m_currentSelection!=NULL && !m_currentSelection->m_activeInstances.empty()){
         data_size = this->m_currentSelection->m_activeInstances.size();
    }*/

    // the dimension size is the lenght of current dimension list

    if(this->m_currentSelectionIdx>-1){
        //if(this->m_currentSelection!=NULL){
        //modify the data_size, so that only brushed data points are stored;
        if(this->m_activeInstances.empty()){
            data_size = this->getOkcData()->getDataSize();
        }else{
            data_size = this->m_activeInstances.size();
        }

        //}
    }

    if(m_currentList.empty()){
        for(int i = 0; i< this->getDimSize(); i++){
            m_currentList.push_back(i);
        }
    }
    int dim_size = this->m_currentList.size();

    // Set the data and dimension size for the new okcdata
    okcdata->data_size = data_size;
    okcdata->dims = dim_size;
    // Since this an intermediate result, its filename is null
    okcdata->filepath[0] = 0;
    // initialize all arrays
    okcdata->names.resize(dim_size);
    okcdata->dim_min.resize(dim_size);
    okcdata->dim_max.resize(dim_size);
    okcdata->cg_dim_min.resize(dim_size);
    okcdata->cg_dim_max.resize(dim_size);
    okcdata->cardinality.resize(dim_size);
    okcdata->data_buf->resize(dim_size * data_size);
    // The data buffer is owned by this okcdata itself
    okcdata->m_isBaseOkcData = true;

    // Get the min, max values and cardinality for each dimension
    std::vector<double> dim_min, dim_max, cg_dim_min, cg_dim_max;
    std::vector<int> cardinality;

    m_inputData->getDimMinArr(dim_min);
    m_inputData->getDimMaxArr(dim_max);
    m_inputData->getCgDimMinArr(cg_dim_min);
    m_inputData->getCgDimMaxArr(cg_dim_max);
    m_inputData->getCardinality(cardinality);

    //int origDimSize = this->m_inputData->getDimSize();

    for(int i = 0; i< dim_size; i++){
        int p = m_currentList[i];

        int namelen = strlen(m_inputData->names[p]);
        okcdata->names[i] = new char[namelen+1];
        strcpy(okcdata->names[i],m_inputData->names[p]);

        okcdata->dim_min[i] = dim_min[p];//use p if use m_inputData
        okcdata->dim_max[i] = dim_max[p];
        okcdata->cg_dim_min[i] = cg_dim_min[p];
        okcdata->cg_dim_max[i] = cg_dim_max[p];
        okcdata->cardinality[i] = cardinality[p];

        DoubleVector data_buf;
        m_inputData->GetDimensionData(data_buf,p);
        for(int j = 0; j< data_size; j++){
            int idx = j;
            if(!this->m_activeInstances.empty()){
                idx = this->m_activeInstances[j];
            }
            (*(okcdata->data_buf))[j*dim_size+i] = data_buf[idx];
        }
        /*
        okcdata->getDimMinArr(dim_min);
        okcdata->getDimMaxArr(dim_max);
        okcdata->getCgDimMinArr(cg_dim_min);
        okcdata->getCgDimMaxArr(cg_dim_max);
        okcdata->getCardinality(cardinality);
        */

    }


    /*std::vector<double> data_buf;
    for (int i=0; i<data_size; i++) {
        m_inputData->getData(data_buf, i);
        for (int d=0; d<dim_size; d++) {
            int h = this->m_currentList[d];

        }
    }*/

    // calculate min, max, cardinality of each selected dimension in
    // the current dimension list
    // Convert the current dimension list to an array
    /*void** dim_arr_voidp = current_list->TransToArray();
    InterRingDimCluster** dim_arr = new InterRingDimCluster*[dim_size];
    int i, d;
    for (d=0; d<dim_size; d++) {
        dim_arr[d] = (InterRingDimCluster*)dim_arr_voidp[d];
        // Get the name from dimension cluster node
        int namelen = strlen(dim_arr[d]->name);
        okcdata->names[d] = new char[namelen+1];
        strcpy(okcdata->names[d], dim_arr[d]->name);
        // calculate the dim_min
        okcdata->dim_min[d] = dim_arr[d]->calculate(dim_min, dim_min, dim_max);
        // calculate the dim_max
        okcdata->dim_max[d] = dim_arr[d]->calculate(dim_max, dim_min, dim_max);
        // calculate the cg_dim_min
        okcdata->cg_dim_min[d] = dim_arr[d]->calculate(cg_dim_min, dim_min, dim_max);
        // calculate the cg_dim_max
        okcdata->cg_dim_max[d] = dim_arr[d]->calculate(cg_dim_max, dim_min, dim_max);
        // calculte the cardinality
        okcdata->cardinality[d] = dim_arr[d]->calculate(cardinality);
    }*/

    // Calculate each datapoint

}

void DimRddtCluster::doRanking(int topk){
    //int dimsize = this->dim_size;
    for(unsigned i = 0; i<this->m_clusters.size(); i++){
        RddtClust* c = dynamic_cast<RddtClust*>(m_clusters[i]);
        for(unsigned i = 0; i<c->m_selected.size(); i++){
            c->m_colSelected.push_back(false);
        }
        c->pickKCandidates(topk);
    }
}

void DimRddtCluster::doLayout(){
    //input
    //this->m_clusters;
    //this->m_outliers;
    //end of input

    //output
    //proximity matrix of distances between each pair of clusters
    //each pair of outliers, and each pair of cluster and outlier
    //end of output

    //there are four sections of this proximity matrix, c->c, o->c, c->o, o->o
    //o->o can be grabbed from the distMtx by the outlier index
    //o->c is the transpose of c->o
    using namespace Rcpp;
    XmdvToolMainWnd* m_mwd = this->m_pm->getMainWnd();
    RInside r = m_mwd->getRinstance();
    try{
        int clustSize = this->m_clusters.size();
        int outlierSize = this->m_outliers.size();
        int size = clustSize + outlierSize;
        if(clustSize==0)
            qDebug()<<"stop here";

        NumericMatrix rddcyMapDist(size,size);

        //c->c, top left corner section;
        for(int i = 0; i<clustSize; i++){
            for(int j = 0 ; j<clustSize; j++){
                if(j > i){
                    RddtClust* c1 = dynamic_cast<RddtClust*>(this->m_clusters[i]);
                    RddtClust* c2 = dynamic_cast<RddtClust*>(this->m_clusters[j]);
                    double d = clusterDistance(c1,c2);
                    rddcyMapDist(i,j) = d;
                }else if(i==j){
                    rddcyMapDist(i,j) = 0.0;
                }else{
                    rddcyMapDist(i,j) = rddcyMapDist(j,i);
                }
            }
        }

        //o->c , bottom left corner  & c->o top right corner;
        for(int i = clustSize; i<size; i++){
            int o = this->m_outliers[ i-clustSize ];
            for(int j = 0 ; j<clustSize; j++){
                RddtClust* c = this->m_clusters[ j ];
                double d = clustOutDistance(c,o);
                rddcyMapDist(i,j) = d;
                rddcyMapDist(j,i) = d;
            }
        }

        //o->o
        for(int i = clustSize; i<size; i++){
            int o1 = this->m_outliers[i-clustSize];
            for(int j = clustSize; j<size; j++){
                int o2 = this->m_outliers[j-clustSize];
                double d = getDistance(o1,o2);
                rddcyMapDist(i,j) = d;
                rddcyMapDist(j,i) = d;
            }
        }
        if(rddcyMapDist.nrow()==1){ //one object on the screen
            r["otlyClstDist"] = rddcyMapDist;
            m_dimLayoutPos.clear();
            m_dimLayoutName.clear();
            for (int i = 0; i < clustSize; i++){
                Vec2 pos( 0.5 , 0.5 );//center on the screen;
                m_dimLayoutPos.push_back(pos);
                assert(m_clusters[i]->m_order.size()>0);
                int topCddtLocIdx = m_clusters[i]->m_order[0];
                int topCddtGlobIdx = m_clusters[i]->m_selected[topCddtLocIdx];
                std::string clustName(m_inputData->names[topCddtGlobIdx]);
                RddtClust* c = dynamic_cast<RddtClust*>(m_clusters[i]);
                int csize = c->m_selected.size();
                QString qstr = QString::number(csize);
                m_dimLayoutName.push_back("Clust "+clustName+" size: "+qstr.toStdString());
            }
            for(int i = clustSize; i<size; i++){
                Vec2 pos( 0.5 , 0.5 );//center on the screen;
                m_dimLayoutPos.push_back(pos);
                int outlierIdx = this->m_outliers[i-clustSize];
                std::string outlierName(m_inputData->names[outlierIdx]);
                m_dimLayoutName.push_back(outlierName);
            }
        }else if(rddcyMapDist.nrow()==2){ //two object on the screen
            r["otlyClstDist"] = rddcyMapDist;
            m_dimLayoutPos.clear();
            m_dimLayoutName.clear();
            for (int i = 0; i < clustSize; i++){
                Vec2 pos( 0 , (double)i/(double)clustSize);
                m_dimLayoutPos.push_back(pos);
                assert(m_clusters[i]->m_order.size()>0);
                int topCddtLocIdx = m_clusters[i]->m_order[0];
                int topCddtGlobIdx = m_clusters[i]->m_selected[topCddtLocIdx];
                std::string clustName(m_inputData->names[topCddtGlobIdx]);
                RddtClust* c = dynamic_cast<RddtClust*>(m_clusters[i]);
                int csize = c->m_selected.size();
                QString qstr = QString::number(csize);
                m_dimLayoutName.push_back("Clust "+clustName+" size: "+qstr.toStdString());
            }
            for(int i = clustSize; i<size; i++){
                Vec2 pos( 1 , 1);
                m_dimLayoutPos.push_back(pos);
                int outlierIdx = this->m_outliers[i-clustSize];
                std::string outlierName(m_inputData->names[outlierIdx]);
                m_dimLayoutName.push_back(outlierName);
            }
        }else{
            r["otlyClstDist"] = rddcyMapDist;
            std::string rcmd;
            rcmd =
                    "ocdist <- as.dist(otlyClstDist);\n"
                    "fit <-cmdscale(d=ocdist,k=2);\n"
                    "p <- fit;\n"
                    "mins<-apply(p,2,min);\n"
                    "maxs<-apply(p,2,max);\n"
                    "normdist<-apply(p,1,'-',mins);\n"
                    "normdist<-t(normdist*(1/(maxs-mins)));\n"
                    //"print(normdist);\n"
                    "normdist";
            SEXP evals = r.parseEval(rcmd);
            NumericMatrix rPos(evals);
            if(rPos.nrow()<2){
                return;
            }
            m_dimLayoutPos.clear();
            m_dimLayoutName.clear();
            for (unsigned i = 0; i < m_clusters.size(); i++){
                double x = rPos(i,0);
                double y = rPos(i,1);
                Vec2 pos( x , y );
                m_dimLayoutPos.push_back(pos);
                //the local index of the top candidate of cluster;
                //we need a local index because the default order of data dimensions in
                //local cluster is by the absolute value of index of each data dimension
                //we reorder them locally by ranking metrics thus the order of the index
                //is no longer sorted
                //if(m_clusters[i]->m_order.size()<=0){
                RddtClust* c = dynamic_cast<RddtClust*>(m_clusters[i]);
                /**
                 * unresolved strange bug here.
                 * perhaps delete RddtClust Pointer from m_clusters cause the problem;
                 */
                if(c!=NULL)
                {
                    if(c->m_selected.size()<=0)
                        return;
                    if(c->m_order.size()<=0)
                        return;
                }
                //}
                int topCddtLocIdx = c->m_order[0];
                int topCddtGlobIdx = c->m_selected[topCddtLocIdx];
                std::string clustName(m_inputData->names[topCddtGlobIdx]);

                int csize = c->m_selected.size();
                QString qstr = QString::number(csize);
                m_dimLayoutName.push_back("Clust "+clustName+" size: "+qstr.toStdString());
            }
            //outliers;
            for(int i = clustSize; i<size; i++){
                double x = rPos(i,0);
                double y = rPos(i,1);
                Vec2 pos( x , y );
                m_dimLayoutPos.push_back(pos);
                int outlierIdx = this->m_outliers[i-clustSize];
                std::string outlierName(m_inputData->names[outlierIdx]);
                m_dimLayoutName.push_back(outlierName);
            }
            //m_inputData->names[topCddtGlobIdx]
        }
    }catch (int e){
        qDebug() << "An exception occurred. Exception Nr. "<<e<<"";
    }
}

double DimRddtCluster::clusterDistance(RddtClust* c1, RddtClust* c2){
    IntIterator idx1;
    IntIterator idx2;
    double size1 = c1->m_selected.size();
    double size2 = c2->m_selected.size();
    if(c1==c2)
    {
        return 0.0;
    }
    double distance = 0.0;
    for(idx1 = c1->m_selected.begin(); idx1 != c1->m_selected.end(); ++idx1){
        for(idx2 = c2->m_selected.begin(); idx2 != c2->m_selected.end(); ++idx2){
            double d = getDistance(*(idx1),*(idx2));
            distance += d; // d is normalized to 0-1, there should not be any overflows;
        }
    }
    return distance/(size1*size2);
}

double DimRddtCluster::clustOutDistance(RddtClust* c, int o){
    IntIterator idx;
    double size = c->m_selected.size();
    //it must be gaurateed that o is not in the m_selected vector of c;
    //if o is in m_selected, there won't be any bug in this function, but it indicates
    //the clustering algorithm was wrong;
    double distance = 0;
    //++idx is more efficient than idx++;
    for(idx = c->m_selected.begin(); idx!=c->m_selected.end(); ++idx){
        int clustIdx = *(idx);
        double d = getDistance(clustIdx,o);
        distance+=d;
    }
    return distance/size;
}

void DimRddtCluster::doSelectedClust(int clustIdx){
    this->m_currentSelectionIdx = clustIdx;
    this->m_currentSelection = m_clusters[clustIdx];
    m_currentSelection->pickKCandidates(1); //pick one by default;
    m_currentSelection->doHeatmap();
    m_currentSelection->doPartitions();
}


void DimRddtCluster::onDataFiltering(){
    //this->m_currentSelection=NULL;
    this->m_currentList.clear();
    this->doDataFiltering();
    this->computeInR();
}

void DimRddtCluster::onDataFilteringClear(){
    this->m_currentSelection=NULL;
    this->m_currentList.clear();
    //this->m_currentSelectionIdx=-1;
    this->m_activeInstances.clear();
    this->initDataInR();
    this->computeInR();
}

void DimRddtCluster::onDefaultViews(){
    this->m_currentSelection=NULL;
    this->m_currentList.clear();
    this->m_height = 0.1;
    this->m_numClust = 1;
    this->initDataInR();
    this->computeInR();
}

void DimRddtCluster::onApplyReduction(){
    this->refreshCurrentList();
}

void DimRddtCluster::onStatisticRbnToggled(bool state){
    if(state){
        this->m_currentSelection=NULL;
        this->m_currentList.clear();
        this->m_simType = STATISTICS;
        this->computeInR();
    }
}

void DimRddtCluster::onSpearmanRbnToggled(bool state){
    if(state){
        this->m_currentSelection=NULL;
        this->m_currentList.clear();
        this->m_simType = SPEARMAN_CORRELATION;
        this->computeInR();
    }
}

void DimRddtCluster::onCrossEntropyRbnToggled(bool state){
    if(state){
        this->m_currentSelection=NULL;
        this->m_currentList.clear();
        this->computeInR();
    }
}
