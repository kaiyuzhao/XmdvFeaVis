/*
 * DimClusterViewOperator.h
 *
 *  Created on: Nov 29, 2012
 *      Author: kaiyuzhao
 */

/*
 * generate cluster view of input dimensions
 * The cluster view of the dimensions are projected to
 * MDS 2 dimensional space, similar dimensions appear to
 * nearer locations
 *
 */

#ifndef DIMCLUSTERVIEWOPERATOR_H_
#define DIMCLUSTERVIEWOPERATOR_H_

#include "operator\Operator.h"

class Data;
class DimRddtCluster;

class DimClusterViewOperator: public Operator {
public:
	DimClusterViewOperator();
	virtual ~DimClusterViewOperator();

public:
	// The function setInput first do type verification
	// and then call the base class
	void setInput(Data* input);
	// Initialize m_brush
	void preOperator();
	// The virtual function to do the main work,
	// to highlight the subset in terms of the brush.
	bool doOperator();
	// Do some work after operator
	void postOperator();

private:
     DimRddtCluster* clustMap;
};

#endif /* DIMCLUSTERVIEWOPERATOR_H_ */
