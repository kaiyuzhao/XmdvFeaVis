/*
 * ManualClustControl.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: kaiyuzhao
 */

/**
 *
 */

#include "ManualClustCtrlWidget.h"
#include "ManualClustDisplay.h"

#include "operator/Operator.h"
#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "pipeline/multidim/DimRPipeline.h"
#include "pipeline/Transformation.h"
#include "view/ViewManager.h"

#include "data/dimreduct/DimRddtCluster.h"
#include "interface/dimreduct/manualclust/ManualClustDisplay.h"
#include "interface/dimreduct/manualclust/ClustParaAdjustingDlg.h"

#include <QPalette>
#include <QString>

ManualClustCtrlWidget::ManualClustCtrlWidget(XmdvToolMainWnd *mainWnd,
                                       PipelineManager* pm, int pID)
    : QWidget(mainWnd)
{
    ui.setupUi(this);

    this->m_pipelineManager = pm;
    this->m_pipelineID = pID;

    Pipeline* pl = pm->getPipeline(pID);

    //DimRPipeline* dimrPL = dynamic_cast<DimRPipeline*>(pl);
    //dimrPL get reduced features;

    this->clustViewQ =
            (pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMCLUST));
    this->m_dimClust =
            dynamic_cast<DimRddtCluster*>(clustViewQ->getOutput());

    this->m_clustDisplay = new ManualClustDisplay(this,this->m_dimClust);
    //this->m_clustDisplay->setDimClust(this->m_dimClust);

    clustScrArea = new QScrollArea();
    clustScrArea->setWidget(this->m_clustDisplay);
    clustScrArea->setWidgetResizable(true);
    clustScrArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    clustScrArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    statusbar = new QLabel();
    ui.mainLayout->addWidget(this->clustScrArea);
    ui.mainLayout->addWidget(this->statusbar);
    clustAdjDlg = NULL;
    connect(ui.clustOptnBtn,SIGNAL(clicked()),this,SLOT(onClustBtnClicked()));

    connect(ui.applyBtn,SIGNAL(clicked()),this->m_dimClust,SLOT(onApplyReduction()));
    connect(ui.applyBtn,SIGNAL(clicked()),this,SLOT(onApplyReduction()));

    connect(ui.rbnStatistcs,SIGNAL(toggled(bool)),
            this->m_dimClust,SLOT(onStatisticRbnToggled(bool)));
    connect(ui.rbnSpearman,SIGNAL(toggled(bool)),
            this->m_dimClust,SLOT(onSpearmanRbnToggled(bool)));
    connect(ui.rbnCrossEntropy,SIGNAL(toggled(bool)),
            this->m_dimClust,SLOT(onCrossEntropyRbnToggled(bool)));
}

ManualClustCtrlWidget::~ManualClustCtrlWidget()
{
    SAFE_DELETE(statusbar);
    SAFE_DELETE(m_clustDisplay);
    SAFE_DELETE(clustScrArea);
    SAFE_DELETE(clustAdjDlg);
}
void ManualClustCtrlWidget::updateStatusBar(QString info) {
    this->statusbar->setText(info);
}

void ManualClustCtrlWidget::onClustBtnClicked(){
    clustAdjDlg = new ClustParaAdjustingDlg(this,this->m_dimClust);
    clustAdjDlg->show();
}

void ManualClustCtrlWidget::onApplyReduction(){
    m_pipelineManager->assemble(m_pipelineID,PL_DIMR_OP_DRCLSTVIEW);
    ViewManager* vm = dynamic_cast<ViewManager*>
            (this->m_pipelineManager->getViewManager());
            //m_mainWnd->getViewManager();
    vm->refreshDisplayByPID(m_pipelineID);
}
