/**
 * Partitions based on one variable in
 * a redundent group;
 */
#ifndef DATAPARTITIONS_H
#define DATAPARTITIONS_H

#include "main/XmdvToolTypes.h"
#include <vector>


class RddtClust;
class DataPartition;

typedef std::vector< std::pair<int, double> > PairVector;

class DataPartitions
{
public:
    DataPartitions();
    virtual ~DataPartitions();

    //pointer to redudency operator;
    RddtClust* m_rddtClust;

    // variable this partitioning is based on
    int m_variable;

    //list of data partitions, size of m_partitions
    //equal to the number of bins;
    std::vector<DataPartition*> m_partitions;

    //goodness measure of each partition; on this variable;
    IntVector m_partitionPerformance;

public:

    //void setOkcData(OkcData* okcData);
    void setRddtClust(RddtClust* rddtClust);

    //binning data into smaller subsets
    void cutData(DoubleVector dimData);

    void doLocalAnalysis();

    //frequence based partitionning
    void doFrequency(DoubleVector dimData);

    //width based partitioning;
    void doRange(DoubleVector dimData);

    void setVarIdx(int idx);

    void pairSort(PairVector &dimOrData);

    void initPairs(PairVector &dimOrData, DoubleVector dimData);
private:
    // return true if fir comes before sec
    static bool pairSortComp(std::pair<int, double> fir, std::pair<int, double> sec){
        return (fir.second<sec.second);
    }
};

#endif // DATAPARTITIONS_H
