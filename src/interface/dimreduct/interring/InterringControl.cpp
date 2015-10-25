/*
 * DimReductControl.cpp
 *
 *  Created on: Sep 22, 2009
 *      Author: xiezx
 */

/**
 * This class is the GUI of interring control, with tree structure displayed and
 * allow the user to select features based on the hierarchy of the feature tree
 */

#include "interface/dimreduct/interring/InterringControl.h"
#include "interface/dimreduct/interring/InterRingDisplay.h"


#include <QMessageBox>
#include <QString>
#include <string.h>

#include "data/dimreduct/InterRingDimClusterTree.h"
#include "datatype/LinkList.h"
#include "main/XmdvTool.h"
#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "pipeline/multidim/DimRPipeline.h"
#include "pipeline/Transformation.h"
#include "operator/Operator.h"
#include "view/ViewManager.h"
#include "view/GLCanvas.h"


InterringControl::InterringControl(XmdvToolMainWnd *mainWnd, PipelineManager* pm, int pID)
	: QWidget(mainWnd) {
	setWindowTitle( tr("Dimension Reduction") );
	// Initialize data members
	m_mainWnd = mainWnd;
	m_pipelineManager = pm;
	m_pipelineID = pID;
	m_show_selected_name = true;
	// Get the pointer to the pipeline to get the current dimension list
	Pipeline* pl = pm->getPipeline(pID);
	DimRPipeline* dimrPL = dynamic_cast<DimRPipeline*>(pl);
	m_current_list = dimrPL->getCurrentList();

	// Get the output of the operator dimTreeQ,
	// which is the dimension cluster tree //dimtreeq should not be disabled;
	dimTreeQ = (pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMTREE));

	m_dimTree = dynamic_cast<InterRingDimClusterTree*>(dimTreeQ->getOutput());

	//dimTreeQ->setEnabled(false);
	// Setup the widgets
	setupGui();
    this->resize(500,500);

	// Connect signals with slots
	connect( button_apply, SIGNAL(clicked()), this, SLOT(applyChange() ) );
	connect( button_deselectAll, SIGNAL(clicked()), this, SLOT(removeAllDimensions() ) );
}

InterringControl::~InterringControl() {
	//delete dimTreeQ;
	//delete m_dimTree;
}


void InterringControl::setupGui() {
	// Create the buttons
	button_deselectAll = new QPushButton("Deselect All", this);
	button_apply = new QPushButton("Apply", this);

	// Create the check box to control whether to show selected cluster name
	check_showName = new QCheckBox("Show name for selected nodes", this);
	check_showName->setChecked(m_show_selected_name);
	connect(check_showName, SIGNAL(stateChanged(int)), this, SLOT(showNameCheckChanged(int)) );

	// Create the label to show the hint
	labelHint = new QLabel("Note: double click nodes to select and deselect");

	// set the GLwidget to render the cluster tree
	m_interRingDisplay = new InterRingDisplay(this);
	m_interRingDisplay->setDimTree(m_dimTree);
	m_interRingDisplay->setCurrentList(m_current_list);

	scrollArea = new QScrollArea();
	scrollArea->setWidget(m_interRingDisplay);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	// Create the label to show what users can do
	// in terms of the mouse position
	labelCaption = new QLabel("");

	layout_buttons = new QHBoxLayout();
	layout_buttons->addWidget(button_deselectAll);
	layout_buttons->addWidget(button_apply);

	// Create the main layout
	mainLayout = new QVBoxLayout();
	mainLayout->addLayout(layout_buttons);
	mainLayout->addWidget(check_showName);
	mainLayout->addWidget(labelHint);
	mainLayout->addWidget(scrollArea);
	mainLayout->addWidget(labelCaption);

	this->setLayout(mainLayout);
}

void InterringControl::updateStatusBar(char* info) {
    labelCaption->setText(QString (info));
}

void InterringControl::ReassemblePipeline() {
	// re-assemble the pipeline starting from SBB operator.
	m_pipelineManager->assemble(m_pipelineID, PL_DIMR_OP_DRVIEW);
}

void InterringControl::updateVisDisplay() {
	ViewManager* vm = m_mainWnd->getViewManager();
	vm->refreshDisplayByPID(m_pipelineID);
}

void InterringControl::applyChange() {
	ReassemblePipeline();
	updateVisDisplay();
}

void InterringControl::removeAllDimensions() {
	// Clear all nodes in the linked list
	m_current_list->clear();
	// Refresh the display for the dimension cluster tree
	m_interRingDisplay->updateGL();
}

void InterringControl::showNameCheckChanged(int) {
	// Retrieve the state of check box
	m_show_selected_name = check_showName->isChecked();
	// Refresh the display for the dimension cluster tree
	m_interRingDisplay->updateGL();
}
