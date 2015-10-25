/**
 * Jan 2013 Kaiyu Zhao
 * One partition
 */

#ifndef DATAPARTITION_H
#define DATAPARTITION_H

#include <RInside.h>
#include "main/XmdvTool.h" //this undefines free and realloc
#include "main/XmdvToolTypes.h"
#include <vector>

class DataPartitions;

class DataPartition
{
public:
    DataPartition();

    virtual ~DataPartition();

    int m_partitionIdx;

    //including all other partitions based on same variables
    //also contains input parameters;
    DataPartitions* m_partitions;

    int m_cntInc;

    int m_cntDec;

    double m_mean;

public:
    //the observations of this partition;
    IntVector m_parIndices;

    //dimensions drop out of group;
    IntVector m_dimOutofGroup;

    void setPartitions(DataPartitions* partitions);

    void compLocalDist();

private:
    //using three different metrics;
    void doCorrelation(RInside &r);

    void doStatistics(RInside &r);

    void doCrossEntropy(RInside &r);

    //global vs local redundency measure/comparision
    void gldiffMeasure(SEXP &exp);

    //local dimensions that drop out of the group generated globally
    //expMtx is the local distance matrix
    //expIdx is the index of the center of the local group
    void gldiffDetail(SEXP &exp);
};

#endif // DATAPARTITION_H
