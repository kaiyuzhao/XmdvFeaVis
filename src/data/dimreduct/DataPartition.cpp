#include "DataPartition.h"
#include "data/multidim/OkcData.h"
#include "data/dimreduct/DimRddtCluster.h"
#include "DataPartitions.h"
#include "data/dimreduct/RddtClust.h"
#include "pipeline/PipelineManager.h"
#include "main/XmdvToolMainWnd.h"

#include <qDebug>


DataPartition::DataPartition():
    m_cntInc(-1),
    m_cntDec(-1)
{

}

DataPartition::~DataPartition(){

}

void DataPartition::setPartitions(DataPartitions* partitions){
    this->m_partitions = partitions;
}

void DataPartition::compLocalDist(){
    //using namespace Rcpp;

    //the variable current partition based on;
    RddtClust* rddtClust = this->m_partitions->m_rddtClust;
    DimRddtCluster* cluster = rddtClust->m_rddtOp;
    //int parVar = this->m_partitions->m_variable;
    IntVector rddtDims = rddtClust->m_rddtDims; //non selected
    IntVector nRddtDims = rddtClust->m_nRddtDims; //selected
    //std::vector<int> obIndices = this->m_parIndices;
    DimRddtCluster::SimilarityMetric simType = cluster->m_simType;
    RInside r = cluster->m_pm->getMainWnd()->getRinstance();
    OkcData* input = cluster->getOkcData();

    int rows = this->m_parIndices.size();
    int cols = rddtDims.size()+nRddtDims.size();

    Rcpp::NumericMatrix partData(rows,cols);
    for(unsigned i = 0; i< nRddtDims.size(); i++){
        DoubleVector dimData;
        input->GetDimensionData(dimData,nRddtDims[i]);
        for(unsigned j = 0; j<m_parIndices.size(); j++){
            partData(j,i) = dimData[m_parIndices[j]];
        }
    }
    for(unsigned i = 0; i< rddtDims.size(); i++){
        DoubleVector dimData;
        input->GetDimensionData(dimData,rddtDims[i]);
        int p = i+nRddtDims.size();
        for(unsigned j = 0; j<m_parIndices.size(); j++){
            partData(j,p) = dimData[m_parIndices[j]];
        }
    }
    r["m"] = partData;
    //done with data partition;
    switch(simType){
    case DimRddtCluster::CROSS_ENTROPY:
        doCrossEntropy(r);
        break;
    case DimRddtCluster::SPEARMAN_CORRELATION:
        doCorrelation(r);
        break;
    case DimRddtCluster::STATISTICS:
        doStatistics(r);
        break;
    default:
        break;
    }
}


void DataPartition::doCorrelation(RInside &r){
    //using namespace Rcpp;
    std::string rcmd =
            //"#print(m);\n"
            "options(warn=-1);\n"
            "testz <- cor(x=m,method=\"spearman\");\n"
            "testz[is.na(testz)] <- 0;\n"
            "dis <- (1-abs(testz));\n"
            //"print(dis);\n"
            "dis";
    SEXP exp = r.parseEval(rcmd);
    //Rcpp::NumericMatrix result(exp);
    gldiffMeasure(exp);
    rcmd = "centroid <- colSums(dis);\n"
            "idx <- as.vector(order(centroid));\n"
            "dis[,idx[1]]";
    SEXP evalsIdx = r.parseEval(rcmd);
    gldiffDetail(evalsIdx);
}

void DataPartition::doStatistics(RInside &r){
    std::string rcmd =
            "mins<-apply(m,2,min);\n"
            "maxs<-apply(m,2,max);\n"
            "norm<-apply(m,1,'-',mins);\n"
            "norm<-t(norm*(1/(maxs-mins)));\n"
            "mu <- colMeans(norm);\n"
            "kthmom2<-apply(norm,2,sd);\n"
            "norm <- t(apply(norm,1,'-',mu));\n"
            "kthmom3<-colMeans(norm^3);\n"
            "kthmom4<-colMeans(norm^4);\n"
            "stat<-cbind(mu,kthmom2);\n"
            "stat<-cbind(stat,kthmom3);\n"
            "stat<-cbind(stat,kthmom4);\n"
            "dis<-dist(stat);\n"
            "dis<-as.matrix(dis);\n"
            "dis";
    SEXP evals = r.parseEval(rcmd);
    gldiffMeasure(evals);
    rcmd = "centroid <- colSums(dis);\n"
            "idx <- as.vector(order(centroid));\n"
            "dis[,idx[1]]\n";
    SEXP evalsIdx = r.parseEval(rcmd);
    gldiffDetail(evalsIdx);
}

void DataPartition::doCrossEntropy(RInside &r){

}


//global vs local differences;
void DataPartition::gldiffMeasure(SEXP &exp){
    Rcpp::NumericMatrix localDist(exp);
    RddtClust* rddtClust = this->m_partitions->m_rddtClust;
    DimRddtCluster* cluster = rddtClust->m_rddtOp;
    //int parVar = this->m_partitions->m_variable;
    IntVector rddtDims = rddtClust->m_rddtDims; //non selected
    IntVector nRddtDims = rddtClust->m_nRddtDims; //selected
    IntVector selected = rddtClust->m_selected;

    if(nRddtDims.size()<1){
        return;
    }
    //not possible greater than selected.size
    //just in case;
    if(nRddtDims.size()>=selected.size()){
        m_cntInc = 0;
        m_cntDec = (int)nRddtDims.size();
        return;
    }

    m_cntInc = 0;
    m_cntDec = 0;

    int offset = nRddtDims.size();
    DoubleVector gvDistDiff;
    //int ro = result.rows();
    int ro = localDist.rows();

    for(int j = offset; j<ro; j++){ //row
        for(int i = 0; i< offset; i++){//col
            double tmp = localDist(j,i);
            int r = nRddtDims[i];
            int c = rddtDims[j-offset];
            double g = cluster->getDistance(r,c);
            double val = tmp;
            gvDistDiff.push_back(val);
        }
    }

    m_mean = 0;
    for(unsigned i = 0 ; i<rddtDims.size(); i++){
        int begin = i*offset;
        int end = begin+offset-1;
        double min =
                *std::min_element(gvDistDiff.begin()+begin, gvDistDiff.begin()+end);
        m_mean+=min;
        if(min>this->m_partitions->m_rddtClust->m_worstFit){
            m_cntInc++; //local similarity threshold is greater than global
        }else{
            m_cntDec++; //local similarity threshold is smaller than global;
        }
    }
    m_mean = m_mean/(double)rddtDims.size();
}


// we can detect which variables drop out of the group in the parition.
void DataPartition::gldiffDetail(SEXP &exp){

    Rcpp::NumericVector dimDist(exp);
    RddtClust* rddtClust = this->m_partitions->m_rddtClust;
    //DimRddtCluster* cluster = rddtClust->m_rddtOp;

    double clustRadius = rddtClust->m_fitRadius;
    //DoubleVector distance;
    //distance.clear();
    for(int i = 0; i< dimDist.length(); i++){
        //distance.push_back(dimDist[i]);
        double val = dimDist[i];
        if(val<clustRadius){
            this->m_dimOutofGroup.push_back(i);
        }
    }
}
