#include "RddtClustEvlDlg.h"
#include "interface/dimreduct/partition/RddcEvlWidget.h"

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
    this->resize(500,500);
    setWindowTitle( tr("Dimension Redundency Analysis") );
    m_pipelineID = pID;
    m_pm = pm;
    m_pl = pm->getPipeline(pID); //get the dim reduction pipeline

    ViewManager* vm= mainWnd->getViewManager();
    ViewWindow* win = vm->getActiveViewWindow();

    evalLayout = new QVBoxLayout();

    evlWidget = new RddcEvlWidget(mainWnd,
                mainWnd->getPipelineManager(), win->getPipelineID());
    evalLayout->addWidget(evlWidget);

    this->setLayout(evalLayout);

}

RddtClustEvlDlg::~RddtClustEvlDlg()
{
    SAFE_DELETE(evlWidget);
    SAFE_DELETE(evalLayout);
    //delete ui;
}
