/*
 * Kaiyu Zhao Dec 2012
 *
 */

#ifndef RDDTCLUST_H
#define RDDTCLUST_H

#include "main/XmdvToolTypes.h"


/**
 * @brief The RddtClust class is the data type holding several similar data
 * dimensions, this class keeps the top k dimensions of a given metric
 * and the rest of dimensions in two vectors;
 *
 */

class DataPartitions;
class OkcData;
class DimRddtCluster;

class RddtClust
{
public:
    //default
    RddtClust();

    //deep copy
    RddtClust(const RddtClust& x);

    //cluster with empty selected dimensions;
    RddtClust(DimRddtCluster* rddtOp);

    //initialize a RdctClust object using selected dimensions
    // and full dimension order by a given metric
    RddtClust(IntVector selected, DimRddtCluster* rddtOp);

    //destructor
    virtual ~RddtClust();

public:
    //the index of dimensions in this dimension cluster
    IntVector m_selected;

    //pointer to the data used for redudency computation;
    DimRddtCluster* m_rddtOp;

    //horizontal position of selected dimentions in the heatmap view;
    BoolVector m_colSelected;

    //recommended dimensions
    IntVector m_nRddtDims;

    //dimensions that are considered redundent
    //these dimensions are rendered in faded color
    IntVector m_rddtDims;

    //list of partitionings based on the currently selected
    //variables, each "DataPartitions*" corresponds to one
    //partitioning
    std::vector<DataPartitions*> m_partitionPerVariable;

    //dimension order by current ranking metric
    IntVector m_order;

    //the subset of data user selected;
    IntVector m_activeInstances;

    // order dims by hclust
    //std::vector<int> m_dimOrd;

    //smallest distance between pairs of dimensions
    double m_bestFit;

    //largest distance between pairs of dimensions;
    double m_worstFit;

    //average distance between pairs of dimensions;
    double m_fitConsistency;

    //radius of the group of dimensions;
    double m_fitRadius;

private:

    //number of non-redudent dimensions in this redudent group;
    //top k
    int m_k;

    //ranking scores of this rdct cluster
    //DoubleVector m_scores;

    //transformation of disMtx, sort each column;
    DoubleVector m_paraMtx;

    IntVector m_paraOrdMtx;


public:
    //initialize dimension scores of each dimensions in this rddt group
    void doLocalDimOrder();

    // get the distance value between a given variable and its kth neighbor
    //neibNum is the k;
    double getParamVal(int var, int neibNum);
    //return neighbour name
    std::string getParamName(int var, int neibNum);
    //set pointer to the dimRddtOperation
    void setRddtOperator(DimRddtCluster* rddtOp);

    //rank currently selected dimensions by passing currently selected redudent
    // group of dimensions and re-order them by currently selected ranking
    // quality metric;
    // the input k is the number of dimensions to recommend as non-redudent;
    // need to update the goodness measure;
    void pickKCandidates(int k);

    //do the partition based on nBins and partitioning method;
    void doPartitions();

    //process the index of subset selected on histogram,
    //var indicate which variable this partitioning is performed on;
    //parStart and parEnd indicates start and end index of partitions on the histogram;
    void doSubSetting(int var, int parStart, int parEnd);

    //
    void redoLocalAnalysis();

    //rank the dimensions by # of outliers in that dimension;
    void doOutliers();

    //rank the dimensions by how close it is to the center of the cluster;
    void doCenter();

    //rendering the selected cluster
    void doHeatmap();

    //switch the selection status of one dimension
    //called when user left click on one column of the heatmap
    //the input horPos is the horizontal position of the clicked column on heatmap
    void switchSelectionState(int horPos);

    //remove one dimension from the cluster;
    void removeSelection(int horPos);

    //check if gDim is in this->m_rddtDims
    //and return the interator;
    IntIterator isRddt(int gDim);

    //check if gDim is in this->m_nRddtDims
    //and return the interator;
    IntIterator isNRddt(int gDim);

    //
    void getDimData(DoubleVector& dimData, int idx);

};

#endif // RDDTCLUST_H
