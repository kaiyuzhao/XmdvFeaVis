#include "RddtClustEvlDlg.h"
//#include "ui_RddtClustEvlDlg.h"


//#include "interface/dimreduct/interring/interringcontrol.h"
//#include "interface/dimreduct/manualclust/ManualClustCtrlWidget.h"
#include "interface/dimreduct/RddcEvlWidget.h"

#include "main/XmdvToolMainWnd.h"
#include "main/XmdvTool.h"

#include "pipeline/Transformation.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "view/ViewWindow.h"
#include "view/ViewManager.h"
#include "operator/Operator.h"

#include <QVBoxLayout>

RddtClustEvlDlg::RddtClustEvlDlg(
        XmdvToolMainWnd* mainWnd,
        PipelineManager *pm, int pID)
    :QDialog(mainWnd)
{
    ui.setupUi(this);
    setWindowTitle( tr("Dimension Redundency Evaluation") );
    m_pipelineID = pID;
    m_pm = pm;
    m_pl = pm->getPipeline(pID); //get the dim reduction pipeline

    ViewManager* vm= mainWnd->getViewManager();
    ViewWindow* win = vm->getActiveViewWindow();

    QVBoxLayout* evalLayout = new QVBoxLayout();

    evlWidget = new RddcEvlWidget(mainWnd,
                mainWnd->getPipelineManager(), win->getPipelineID());
    evalLayout->addWidget(evlWidget);

    this->setLayout(evalLayout);

}

RddtClustEvlDlg::~RddtClustEvlDlg()
{
    //delete ui;
}
