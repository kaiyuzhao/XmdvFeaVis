#include "RddcEvlWidget.h"
#include "operator/Operator.h"
#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "pipeline/multidim/DimRPipeline.h"
#include "pipeline/Transformation.h"
#include "view/ViewManager.h"

#include "data/dimreduct/DimRddtCluster.h"
#include "interface/dimreduct/partition/RddcEvlDisplay.h"

#include <QScrollArea>
#include <QLabel>
#include <QString>


RddcEvlWidget::RddcEvlWidget(XmdvToolMainWnd *mainWnd,
                             PipelineManager* pm, int pID) :
    QWidget(mainWnd)
{
    ui.setupUi(this);
    this->m_pipelineID = pID;
    this->m_pipelineManager = pm;

    Pipeline* pl = pm->getPipeline(pID);

    //DimRPipeline* dimrPL = dynamic_cast<DimRPipeline*>(pl);
    //dimrPL get reduced features;

    this->clustViewQ =
            (pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMCLUST));
    this->m_dimClust =
            dynamic_cast<DimRddtCluster*>(clustViewQ->getOutput());

    m_rddcEvlDisplay = new RddcEvlDisplay(this, this->m_dimClust);
    //this->m_rddcEvlDisplay->setDimClust(this->m_dimClust);

    m_evalScrArea = new QScrollArea();
    m_evalScrArea->setWidget(this->m_rddcEvlDisplay);
    m_evalScrArea->setWidgetResizable(true);
    m_evalScrArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_evalScrArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    ui.verticalLayout->addWidget(this->m_evalScrArea);
    //ui.verticalLayout_2->addWidget(this->clustScrArea);

    m_statusBar = new QLabel();
    ui.verticalLayout->addWidget(this->m_statusBar);

    connect(ui.applyBtn,SIGNAL(clicked()),m_rddcEvlDisplay,SLOT(onDataFiltering()));
    connect(ui.applyBtn,SIGNAL(clicked()),m_rddcEvlDisplay,SLOT(onClearDataFiltering()));

    connect(ui.applyBtn,SIGNAL(clicked()),m_dimClust,SLOT(onDataFiltering()));
    connect(ui.clearBtn,SIGNAL(clicked()),m_dimClust,SLOT(onDataFilteringClear()));

    connect(ui.applyBtn,SIGNAL(clicked()),this,SLOT(onApplyReduction()));
    connect(ui.clearBtn,SIGNAL(clicked()),this,SLOT(onApplyReduction()));

}

RddcEvlWidget::~RddcEvlWidget()
{
    //delete ui;
    SAFE_DELETE(m_statusBar);
    SAFE_DELETE(m_evalScrArea);

}

void RddcEvlWidget::updateStatusBar(QString info){
    this->m_statusBar->setText(info);
}

void RddcEvlWidget::onApplyReduction(){
    m_pipelineManager->assemble(m_pipelineID,PL_DIMR_OP_DRCLSTVIEW);
    ViewManager* vm = dynamic_cast<ViewManager*>
            (this->m_pipelineManager->getViewManager());
            //m_mainWnd->getViewManager();
    vm->refreshDisplayByPID(m_pipelineID);
}
