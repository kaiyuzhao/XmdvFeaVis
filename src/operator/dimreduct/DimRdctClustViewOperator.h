/*
 * DimRdctClustViewOperator.h
 *
 *  Created on: Nov 30, 2012
 *      Author: kaiyuzhao
 */

/*
 * reduce the number of dimensions by user brushing on the cluster of dimensions
 *
 */

#ifndef DIMRDCTCLUSTVIEWOPERATOR_H_
#define DIMRDCTCLUSTVIEWOPERATOR_H_

#include "operator\Operator.h"

class Data;

class DimRdctClustViewOperator: public Operator {
public:
    DimRdctClustViewOperator();
    virtual ~DimRdctClustViewOperator();

public:
	void setInput(Data* input);
	// Initialize m_brush
	void preOperator();
	// The virtual function to do the main work,
	// to highlight the subset in terms of the brush.
	bool doOperator();
	// Do some work after operator
	void postOperator();
};

#endif /* DIMRDCTCLUSTVIEWOPERATOR_H_ */
