/*
 * DimClusterMDSMap.h
 *
 *  Created on: Nov 30, 2012
 *      Author: kaiyuzhao
 */


/*
 *this class is used for constructing layout of
 *dimensions in the original data space;
 */

#ifndef DIMRDDTCLUSTER_H_
#define DIMRDDTCLUSTER_H_

#include <QObject>
#include "data/Data.h"

#include "datatype/Vec2.h"
#include "datatype/RGBt.h"

#include "data/dimreduct/RddtClust.h"
#include "color/ColorManager.h"
#include "main/XmdvToolTypes.h"
#include <QMutex>

class OkcData;
class PipelineManager;


class DimRddtCluster: public QObject, public Data{
    Q_OBJECT
public:

    //parameter settings;
    //binning method
    enum BinningMethod {
        EQUAL_FREQUENCY,
        BY_RANGE
    } m_binType;

    //similarity metrics for any pair of data dimensions;
    enum SimilarityMetric{
        SPEARMAN_CORRELATION,
        CROSS_ENTROPY,
        STATISTICS
    } m_simType;

    //interestingness metrics
    enum RankingMetric{
        ENTROPY,
        OUTLIERS,
        CENTER
    } m_rankType;

    //pipeline manager
    PipelineManager* m_pm;

    //color manager
    ColorManager* m_cm;

    //number of bins for each variable;
    int m_nBins; //default 10;

    //dimension names
    StringVector m_dimName;

    //cluster name + outlier name
    StringVector m_dimLayoutName;

    //constructor
    DimRddtCluster(PipelineManager* pm);

    //destructor;
    virtual ~DimRddtCluster();

private:
    QMutex mutex;
    //colors
   // RGBt* fade;
   // RGBt* normal;
   // RGBt* selected;
   // RGBt* transparent;

    //number of dimensions;
    int m_dimSize;

    //currently selected initial starting dimension for a neighbourhood
    int m_startingDim; // -1;

    //update these two values in slot functions;
    double m_height; // = 0.5 cut off

    int m_heightInt;

    int m_numClust; // = 0 number of clusters to get

    int m_numClustInt;

    //keep the output data from prior operator
    OkcData* m_inputData;

    //dimension layout positions (cluster+outliers)
    Vec2Vector m_dimLayoutPos;

    //dimension similarities proximity matrix stored in one dimensional array;
    //need to be cleared when switch to different data dimension similarity metric
    DoubleVector m_disMtx;

    /**
     * temporary result;
     */
    //currently actively selected cluster;
    //this list is to be processed by ranking metrics
    //so that only top k is kept as non-redudent dimensions
    IntVector m_neighbours; //must be sorted before use...

    // The pointer to the current dimension list chosen by the user.
    // It will be set by dimension reduction dialog
    IntVector m_currentList;



    //comparator for clusters; based on size;
    static bool clustSortComp(RddtClust* fir, RddtClust* sec){
        return (fir->m_selected.size() > sec->m_selected.size());
    }

public:
    //the current cluster selected by the user;
    RddtClust* m_currentSelection;

    //the current cluster selected by the user, idx;
    int m_currentSelectionIdx;

    //selected data points;
    IntVector m_activeInstances;

    //indices in each m_cluster and m_outliers add to the total number of
    //dimensions; clusters;
    std::vector<RddtClust*> m_clusters;

    //indicate which clusters are currently selected for the data space;
    //that these data dimensions will be sent to next operator to be visualized;
    BoolVector m_activeClusters;

    //outliers not in any clusters;
    IntVector m_outliers;

    //outliers are currently selected for the data space;
    //that these data dimensions will be sent to next operator to be visualized;
    BoolVector m_activeOutliers;


    void initDataInR();

    //using three different distance metrics
    //compute similarity distances based on different similarity metrics
    //cluster the dimensions
    //rank data dimensions in each cluster
    //partition the data space in each cluster and verify the local redundencies;
    void computeInR();


    //filter data based on currently user specified data partitions;
    void doDataFiltering();

    void restoreDataFiltering();

    //doSelectedClust prepares necessary data for the heatmap view
    //and histogram view (partition view);
    void doSelectedClust(int clustIdx);

    //set the okc data;
    void setOkcData(OkcData* data);

    //return the original okc data;
    OkcData* getOkcData();

    //return the number of dimensions in the mds layout;
    int getDimSize();

    //return the distances between two dimensions
    //by providing row and col
    //indicating two instances in the proximity matrix
    double getDistance(int r, int c);

    //return the distance threshold for given variable and
    //number of neighbours;
    double getParamVal(int var, int neibNum);

    //return the positions of the dimensions
    Vec2Vector getPos();

    //return initial dim order for heatmap;
    IntVector getDimOrder();

    void refreshClusters();

    void generateDimReductResult(OkcData* output);


private:
    //remove cluster of size 1 from m_clusters and push it into m_outliers.
    void cleanCluster();

    //by default every cluster and every outlier are active;
    void initializeActiveList();

    //using spearman metric to calculate the distance metrics;
    void doSpearman();

    void doStatistics();
    //do MDS layout using distance metric produced by similarity metrics;
    //void doMDSlayout();

    void calculate(DoubleVector &all_data,
                   DoubleVector &dim_min, DoubleVector &dim_max);

    //compute the distance matrix between each pair of clusters and outliers
    //prepair necessary data to ManualClustDisplay;
    void doLayout();

    //do clustering based on default parameters;
    //clustering the dimensions, using reaching distance
    //and minimum number of supporting points;
    void doClustering(double treeHeight, int numClusts);

    //rank variables of each cluster based on some metric;
    void doRanking(int topk);

    //compute distance between two clusters
    double clusterDistance(RddtClust* c1, RddtClust* c2);

    //compute distance between one cluster and one outlier;
    double clustOutDistance(RddtClust* c, int o);

    //find the neighbouring dimensions of the dimension
    //the user points to
    // the density threshold eps and npts must be satisfied
    void findNeighbours(IntVector &neighbs, int idx, DoubleVector dist,
                         BoolVector &visited, double m_height, unsigned int m_numClust);

    //helper function for find_neighbours, this function query the neighbourhood of
    // the dimension the user clicked on.
    void findRegion(IntVector &result,
                    int idx, DoubleVector dist, double m_height);
    //std::vector<int> findRegion(int idx, std::vector<double> dist, double eps);

public:

    //this function collects all active cluster members from all active clusters
    //and also collects all active outliers; save the active data dimensions into the
    // IntVector m_currentList;
    void refreshCurrentList();


    //emit sigUpdateAllViews();
    void updateAllViews();

signals:
    //signals all the views to update;
    void sigUpdateAllViews();

public slots:
    //update reaching distance;
    void updateClustDistParas(int val);

    //update min points;
    void updateClustMptsParas(int val);

    void onDataFiltering();

    void onDataFilteringClear();

    void onDefaultViews();

    void onApplyReduction();

    void onStatisticRbnToggled(bool state);

    void onSpearmanRbnToggled(bool state);

    void onCrossEntropyRbnToggled(bool state);
};

#endif /* DIMRDDTCLUSTER_H_ */
