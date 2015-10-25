/*
 * DimClusterViewOperator.cpp
 *
 *  Created on: Nov 29, 2012
 *      Author: kaiyuzhao
 */

#include "DimClusterViewOperator.h"
#include "data/multidim/OkcData.h"
#include "data/dimreduct/DimRddtCluster.h"
#include "pipeline/Pipeline.h"
#include "pipeline/PipelineManager.h"

#include <cassert>
#include <typeinfo>
#include <qDebug>

DimClusterViewOperator::DimClusterViewOperator() {
	// TODO Auto-generated constructor stub

}

DimClusterViewOperator::~DimClusterViewOperator() {
	// TODO Auto-generated destructor stub
    //qDebug()<<"~DimClusterViewOperator\n";
    //SAFE_DELETE(clustMap);
}

// The function setInput first do type verification
// and then call the base class
void DimClusterViewOperator::setInput(Data* input){
    assert(typeid(*input)==typeid(OkcData));
    Operator::setInput(input);
}
// Initialize m_brush
void DimClusterViewOperator::preOperator(){
    assert(typeid(*m_input)==typeid(OkcData));
}
// The virtual function to do the main work,
// to highlight the subset in terms of the brush.
bool DimClusterViewOperator::doOperator(){
    assert(typeid(*m_input)==typeid(OkcData));
    //OkcData* output = new OkcData();//(OkcData*)m_input;
    //qDebug()<<"DimClusterViewOperator doOperator().\n";
    //PipelineManager* pm = this->m_pipeline->getPipelineManager();
    clustMap =
            new DimRddtCluster(this->m_pipeline->getPipelineManager());

    //clustMap->m_pm = this->m_pipeline->getPipelineManager();
    OkcData* input = dynamic_cast<OkcData*>(m_input);
    clustMap->setOkcData(input);
    clustMap->initDataInR();
    clustMap->computeInR();
    //this->clustCtrller = new ClustDispCtrl();
    //this->clustCtrller->setDimClust(clustMap);
    SAFE_DELETE(m_output);
    m_output = dynamic_cast<Data*>(clustMap);
	return true;
}
// Do some work after operator
void DimClusterViewOperator::postOperator(){

}
