#include "ClustHeatMapViewDlg.h"
#include "main/XmdvTool.h"
#include "interface/dimreduct/heatmap/HeatmapClustCtrlWidget.h"


#include "main/XmdvToolMainWnd.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
//#include "pipeline/multidim/DimRPipeline.h"
//#include "pipeline/Transformation.h"


#include "ui_ClustHeatMapViewDlg.h"

#include <QBoxLayout>

ClustHeatMapViewDlg::ClustHeatMapViewDlg(XmdvToolMainWnd *mainWnd,
                                         PipelineManager* pm, int pID)
    :QDialog(mainWnd),
    ui(new Ui::ClustHeatMapViewDlg)
{
    ui->setupUi(this);
    this->resize(500,500);
    setWindowTitle( tr("Cluster Detail View") );
    mainLayout =  new QVBoxLayout();

    //ViewManager* vm= mainWnd->getViewManager();
    //ViewWindow* win = vm->getActiveViewWindow();
    //Pipeline* pl = pm->getPipeline(pID);
    //QVBoxLayout* hmpLayout = new QVBoxLayout();

    //hmpLayout->addWidget(hmpDlg);
    //ui.heatmaptab->setLayout(hmpLayout);

    hmpDlg = new HeatmapClustCtrlWidget(mainWnd, pm, pID);
    mainLayout->addWidget(hmpDlg);
    this->setLayout(mainLayout);
}

ClustHeatMapViewDlg::~ClustHeatMapViewDlg()
{
    SAFE_DELETE(mainLayout);
    SAFE_DELETE(hmpDlg);
    delete ui;
}
