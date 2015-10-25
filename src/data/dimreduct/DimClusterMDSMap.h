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

#ifndef DIMCLUSTERMDSMAP_H_
#define DIMCLUSTERMDSMAP_H_


#include "data/Data.h"
#include <vector>
#include <string>
#include "datatype/Vec2.h"
#include "datatype/RGBt.h"


class OkcData;
class PipelineManager;
class RddtClust;

class DimClusterMDSMap: public Data {

public:
    DimClusterMDSMap();
    DimClusterMDSMap(PipelineManager* pm);
	virtual ~DimClusterMDSMap();

private:
    RGBt* fade;
    RGBt* normal;
    RGBt* selected;
    RGBt* transparent;

    //keep the output data from prior operator
    OkcData* m_inputData;

	//dimension names
    std::vector<std::string> dim_name;

    //dimension layout positions
    std::vector<Vec2> dim_layout_pos;

    //dimension similarities proximity matrix stored in one dimensional array;
    std::vector<double> disMtx;

    //transformation of disMtx, sort each column;
    std::vector<double> paraMtx;

    // order dims by hclust
    std::vector<int> m_dimOrd;

    // index of the dim orders;
    std::vector<int> m_dimIdx;



    //number of dimensions;
    int dim_size;

public:

    // order dims by a metric, std dev, entropy, etc.
    std::vector<int> m_dimRank;

    //actual values of this metric
    std::vector<double> m_metricScore;

    PipelineManager* m_pm;

    //input OkcData and modify dim_layout_pos
    //using three different distance metrics
    void computeInR(OkcData* data);

    //set the okc data;
    void setOkcData(OkcData* data);

    //return the original okc data;
    OkcData* getOkcData();

    //return the positions of the dimensions
    std::vector<Vec2> getPos();

    // return the names of the dimensions
    std::vector<std::string> getNames();

    //return the distances between two dimensions
    //by providing row and col
    //indicating two instances in the proximity matrix
    double getDistance(int r, int c);

    //return the distance threshold for given variable and
    //number of neighbours;
    double getParamVal(int var, int neibNum);

    //return the distance matrix in one vector;
    std::vector<double> getDistance();

    //return the number of dimensions in the mds layout;
    int getDimSize();

    //return initial dim order for heatmap;
    std::vector<int> getDimOrder();

    //return the index of the initial dim order;
    std::vector<int> getDimIdx();

    //control views;
private:
    //currently selected initial starting dimension for a neighbourhood
    int m_starting_dim; // -1;

    //update these two values in slot functions;
    double eps; // = 0.1 reaching distance, range (0,1]
    int npts; // = 1 density requirement, can be [0,50]

    //currently actively selected cluster;
    //this list is to be processed by ranking metrics
    //so that only top k is kept as non-redudent dimensions
    std::vector<int> neighbours; //must be sorted before use...

    // The pointer to the current dimension list chosen by the user.
    // It will be set by dimension reduction dialog
    std::vector<int> currentList; //TO DO

    //temporary result of merging noredudent dimensions of each redudent
    //cluster
    std::vector<int> m_nRddt_all;

    //temporary result of merging noredudent dimensions of each redudent
    //cluster
    std::vector<int> m_rddt_all;

    //find the neighbouring dimensions of the dimension
    //the user points to
    // the density threshold eps and npts must be satisfied
    void find_neighbours(std::vector<int> &neighbs, int idx, std::vector<double> dist,
                         std::vector<bool> &visited, double eps, unsigned int npts);

    //helper function for find_neighbours, this function query the neighbourhood of
    // the dimension the user clicked on.
    std::vector<int> findRegion(int idx, std::vector<double> dist, double eps);

public:
    //return new cluster
    void formNewCluster();

    //collection of redudent clusters, each cluster is a selection by the user
    std::vector<RddtClust> m_currentView; //clear this when user switch similarity
    //metrics

    //color of each dimension, depends on selections and redundencies
    std::vector<RGBt> m_MDSViewColors;

    //color of selections on the heatmap;
    std::vector<RGBt> m_heatmpViewColors;

    //the current cluster selected by the user;
    RddtClust* m_currentSelection;

    // mark a redudency group starting from input dim
    // this function calls find_neighbours to make a selection of
    // similar dimensions based on distance threshold eps
    // and density threshold npts;
    void markRdctGroup(int dim);

    //after each double click, refresh the selection
    //by merging non-redudent dimensions of each redudent group/cluster
    //and merging redudent dimensions of each group
    void refreshSelection();

    //after each single clidk, refresh the selection
    //the coloring of current selection is different from previous selections
    //thus the previous redudent groups and current goup must be merged
    void refreshCurrentSelection();

    //renderring view spaces of MDS clustering display
    void renderringMDSview();

    //renderring view spaces of heatmap
    void renderringHeatmap();

};

#endif /* DIMCLUSTERMDSMAP_H_ */
