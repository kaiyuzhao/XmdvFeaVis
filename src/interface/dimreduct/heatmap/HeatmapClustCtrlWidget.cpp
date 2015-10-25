#include "HeatmapClustCtrlWidget.h"
#include "ui_HeatmapClustCtrlWidget.h"


#include "operator/Operator.h"
#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "pipeline/multidim/DimRPipeline.h"
#include "pipeline/Transformation.h"
#include "view/ViewManager.h"

#include "data/dimreduct/DimRddtCluster.h"
#include "interface/dimreduct/heatmap/HeatmapDisplay.h"
#include "interface/dimreduct/heatmap/HeatmapColorBar.h"

#include <QScrollArea>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>

HeatmapClustCtrlWidget::HeatmapClustCtrlWidget(XmdvToolMainWnd *mainWnd,
                                               PipelineManager* pm, int pID)
    :QWidget(mainWnd)
{
    ui.setupUi(this);

    m_pipelineManager = pm;
    m_pipelineID = pID;
    Pipeline* pl = pm->getPipeline(pID);

    //DimRPipeline* dimrPL = dynamic_cast<DimRPipeline*>(pl);
    //dimrPL get reduced features;

    this->clustViewQ =
            (pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMCLUST));
    this->m_dimClust =
            dynamic_cast<DimRddtCluster*>(clustViewQ->getOutput());

    this->m_heatmapColorBar = new HeatmapColorBar(this,this->m_dimClust);

    this->m_heatmapDisp = new HeatmapDisplay(this, this->m_dimClust);
    //this->m_heatmapDisp->setDimClust(this->m_dimClust);
   // m_heatmapDisp->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    hmpScrArea = new QScrollArea();
    //scrAreaLayout = new QVBoxLayout();
    //scrAreaLayout->addWidget(this->m_heatmapDisp);

    hmpScrArea->setWidget(m_heatmapDisp);
   // m_heatmapDisp->layout()->setSizeConstraint(QLayout::SetMaximumSize);
    hmpScrArea->setWidgetResizable(true);
    hmpScrArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    hmpScrArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    statusbar = new QLabel();
    ui.verticalLayout_2->addWidget(this->m_heatmapColorBar);
    ui.verticalLayout_2->addWidget(this->hmpScrArea);
    ui.verticalLayout_2->addWidget(this->statusbar);

    connect(ui.pbt_apply,SIGNAL(clicked()),this->m_dimClust,SLOT(onApplyReduction()));
    connect(ui.pbt_apply,SIGNAL(clicked()),this,SLOT(onApplyReduction()));
}

HeatmapClustCtrlWidget::~HeatmapClustCtrlWidget()
{
    //delete ui;
    SAFE_DELETE(statusbar);
    SAFE_DELETE(m_heatmapDisp);
    SAFE_DELETE(m_heatmapColorBar);
    SAFE_DELETE(hmpScrArea);
}

void HeatmapClustCtrlWidget::setDimClust(DimRddtCluster* dimClust){
    this->m_dimClust = dimClust;
}

void HeatmapClustCtrlWidget::updateStatusBar(QString info) {
    this->statusbar->setText(info);
}

void HeatmapClustCtrlWidget::onApplyReduction(){
    m_pipelineManager->assemble(m_pipelineID,PL_DIMR_OP_DRCLSTVIEW);
    ViewManager* vm = dynamic_cast<ViewManager*>
            (this->m_pipelineManager->getViewManager());
            //m_mainWnd->getViewManager();
    vm->refreshDisplayByPID(m_pipelineID);
}
