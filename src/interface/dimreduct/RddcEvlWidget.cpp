#include "RddcEvlWidget.h"
#include "operator/Operator.h"
#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "pipeline/multidim/DimRPipeline.h"
#include "pipeline/Transformation.h"

#include "data/dimreduct/DimRddtCluster.h"
#include "interface/dimreduct/RddcEvlDisplay.h"

#include <QScrollArea>


RddcEvlWidget::RddcEvlWidget(XmdvToolMainWnd *mainWnd,
                             PipelineManager* pm, int pID) :
    QWidget(mainWnd)
{
    ui.setupUi(this);
    Pipeline* pl = pm->getPipeline(pID);

    //DimRPipeline* dimrPL = dynamic_cast<DimRPipeline*>(pl);
    //dimrPL get reduced features;

    this->clustViewQ =
            (pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMCLUST));
    this->m_dimClustPos =
            dynamic_cast<DimRddtCluster*>(clustViewQ->getOutput());

    m_rddcEvlDisplay = new RddcEvlDisplay(this);
    this->m_rddcEvlDisplay->setDimClust(this->m_dimClustPos);

    evalScrArea = new QScrollArea();
    evalScrArea->setWidget(this->m_rddcEvlDisplay);
    evalScrArea->setWidgetResizable(true);
    evalScrArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    evalScrArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    ui.verticalLayout_2->addWidget(this->evalScrArea);
    //ui.verticalLayout_2->addWidget(this->clustScrArea);
}

RddcEvlWidget::~RddcEvlWidget()
{
    //delete ui;
}
