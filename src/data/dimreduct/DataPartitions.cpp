#include "DataPartitions.h"

#include "data/multidim/OkcData.h"
#include "data/dimreduct/DataPartition.h"
#include "data/dimreduct/RddtClust.h"
#include "data/dimreduct/DimRddtCluster.h"
#include <cmath>


DataPartitions::DataPartitions()
{

}

DataPartitions::~DataPartitions(){
    if(!m_partitions.empty()){
        for(unsigned i = 0; i< m_partitions.size(); i++){
            SAFE_DELETE(m_partitions[i]);
        }
        m_partitions.clear();
    }
}
void DataPartitions::setRddtClust(RddtClust* rddtClust){
    this->m_rddtClust = rddtClust;
}


void DataPartitions::cutData(DoubleVector dimData){
    //http://perso.rd.francetelecom.fr/boulle/publications/BoulleIDA05.pdf
    //default 10
    switch(this->m_rddtClust->m_rddtOp->m_binType){
        case (DimRddtCluster::EQUAL_FREQUENCY):
            doFrequency(dimData);
            break;
        case DimRddtCluster::BY_RANGE:
            doRange(dimData); //Freedman–Diaconis rule
            break;
        default:
            break;
    }
    doLocalAnalysis();
}

void DataPartitions::doLocalAnalysis(){
    RddtClust* rddtClust = this->m_rddtClust;
    //DimRddtCluster* cluster = rddtClust->m_rddtOp;
    //int parVar = this->m_partitions->m_variable;
    IntVector rddtDims = rddtClust->m_rddtDims; //non selected
    IntVector nRddtDims = rddtClust->m_nRddtDims; //selected

    if((int)nRddtDims.size()<1){
        //do not draw partition histograms;
        return;
    }
    this->m_partitionPerformance.clear();
    for(unsigned i = 0; i< this->m_partitions.size(); i++){
        DataPartition* partition = dynamic_cast<DataPartition*>(m_partitions[i]);
        partition->compLocalDist();
        int inc = partition->m_cntInc;
        m_partitionPerformance.push_back(inc);
    }

}

void DataPartitions::doFrequency(DoubleVector dimData){
    m_partitions.clear();
    PairVector dimOrData;
    initPairs(dimOrData, dimData);
    pairSort(dimOrData);
    int dimsize = dimData.size();

    int nBins = this->m_rddtClust->m_rddtOp->m_nBins;
    if(nBins == 0 ){
        nBins = 1;
    }
    if(nBins>dimsize){
        nBins = dimsize;
    }
    for(int i = 0; i< nBins; i++){
        DataPartition* partition = new DataPartition();
        partition->setPartitions(this);
        partition->m_partitionIdx = i;
        this->m_partitions.push_back(partition);
    }

    int partitionsize = (int)ceil((double)dimsize/(double)nBins);
    for(unsigned i = 0; i< dimOrData.size(); i++){
        int p = (int)((double)i/(double)partitionsize);
        if(p>(int)m_partitions.size())
            p=m_partitions.size();
        int first = (dimOrData[i]).first;
        (m_partitions[p])->m_parIndices.push_back(first);
    }

}

void DataPartitions::doRange(DoubleVector dimData){
    m_partitions.clear();
    PairVector dimOrData;
    initPairs(dimOrData, dimData);
    pairSort(dimOrData);
    //todo
}

void DataPartitions::setVarIdx(int idx){
    this->m_variable = idx;
}

/*void DataPartitions::setOkcData(OkcData* okcData){
    this->m_dataInput = okcData;
}

void DataPartitions::setnBins(int nBins){
    this->m_nBins = nBins;
}



void DataPartitions::setNRddtDims(std::vector<int> &nRddtDims){
    this->m_nRddtDims = nRddtDims;
}

void DataPartitions::setRddtDims(std::vector<int> &rddtDims){
    this->m_rddtDims = rddtDims;
}*/

void DataPartitions::pairSort(PairVector &dimOrData){
    std::sort(dimOrData.begin(),dimOrData.end(),pairSortComp);
}

// return true if fir comes before sec
/*static bool DataPartitions::pairSortComp(std::pair<int, double> fir,
                                 std::pair<int, double> sec){
    return (fir.second<sec.second);
}*/

void DataPartitions::initPairs(PairVector &dimOrData, DoubleVector dimData){
    dimOrData.clear();
    for(unsigned i = 0; i< dimData.size(); i++){
        int first = (int)i;
        double second = dimData[i];
        std::pair<int, double> temp(first,second);
        dimOrData.push_back(temp);
    }
}
