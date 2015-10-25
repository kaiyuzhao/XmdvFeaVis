/*
 * DimRdctClustViewOperator.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: kaiyuzhao
 */

#include "DimRdctClustViewOperator.h"
#include "data/multidim/OkcData.h"
#include "pipeline/multidim/OkcPipeline.h"
#include "data/dimreduct/DimRddtCluster.h"

#include <cassert>
#include <typeinfo>
#include <qDebug>

DimRdctClustViewOperator::DimRdctClustViewOperator() {
	// TODO Auto-generated constructor stub

}

DimRdctClustViewOperator::~DimRdctClustViewOperator() {
	// TODO Auto-generated destructor stub

}

void DimRdctClustViewOperator::setInput(Data* input){
	//Verify that we have a correct data type for input
    assert(typeid(*input)==typeid(DimRddtCluster));
	Operator::setInput(input);
}
// Initialize m_brush
void DimRdctClustViewOperator::preOperator(){
     assert(typeid(*m_input)==typeid(DimRddtCluster));
}
// The virtual function to do the main work,
// to highlight the subset in terms of the brush.
bool DimRdctClustViewOperator::doOperator(){
    assert(typeid(*m_input)==typeid(DimRddtCluster));

    //SAFE_DELETE(m_output);

    DimRddtCluster* input = dynamic_cast<DimRddtCluster*>(m_input);
    OkcData* output = new OkcData();
    input->generateDimReductResult(output);

    //m_output = output;//new OkcData(((DimRddtCluster*)m_input)->getOkcData());

    SAFE_DELETE(m_output);
    m_output = dynamic_cast<Data*>(output);

    // Because we get a new dataset having the different structure
    // from the original data, we need to refresh the brush storage
    OkcPipeline* okc_pl = dynamic_cast<OkcPipeline*> (getPipeline());
    okc_pl->refreshBrushStorage(output);
    return true;

    /*


    //input is type of Data;
    InterRingDimClusterTree* input = (InterRingDimClusterTree*)m_input;

    OkcData* output = new OkcData();
    input->generateDimReductResult(output, getCurrentList());

    SAFE_DELETE(m_output);
    m_output = dynamic_cast<Data*>(output);

    // Because we get a new dataset having the different structure
    // from the original data, we need to refresh the brush storage
    OkcPipeline* okc_pl = dynamic_cast<OkcPipeline*> (getPipeline());
    okc_pl->refreshBrushStorage(output);

    */
}
// Do some work after operator
void DimRdctClustViewOperator::postOperator(){

}
