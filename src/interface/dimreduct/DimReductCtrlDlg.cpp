#include "DimReductCtrlDlg.h"
#include "interface/dimreduct/interring/interringcontrol.h"
#include "interface/dimreduct/manualclust/ManualClustCtrlWidget.h"

#include "main/XmdvToolMainWnd.h"
#include "main/XmdvTool.h"

#include "pipeline/Transformation.h"
#include "pipeline/PipelineManager.h"
#include "pipeline/Pipeline.h"
#include "view/ViewWindow.h"
#include "view/ViewManager.h"
#include "operator/Operator.h"

#include <QVBoxLayout>
#include <qDebug>

DimReductCtrlDlg::DimReductCtrlDlg(XmdvToolMainWnd *mainWnd,
                                   PipelineManager* pm, int pID)
:QDialog(mainWnd)
{
	ui.setupUi(this);
    this->setWindowTitle(tr("Dimension Reduction"));
    this->resize(600,800);
    m_pipelineID = pID;
    m_pm = pm;
    m_pl = pm->getPipeline(pID); //get the dim reduction pipeline

	ViewManager* vm= mainWnd->getViewManager();
	ViewWindow* win = vm->getActiveViewWindow();

    QVBoxLayout* interringlayout = new QVBoxLayout();
	sbbDlg = new InterringControl(mainWnd,
				mainWnd->getPipelineManager(), win->getPipelineID());
	interringlayout->addWidget(sbbDlg);

	QVBoxLayout* clustLayout = new QVBoxLayout();
    clstDlg = new ManualClustCtrlWidget(mainWnd,
                                        mainWnd->getPipelineManager(), win->getPipelineID());
	clustLayout->addWidget(clstDlg);

	ui.interringtab->setLayout(interringlayout);
	ui.clusteringtab->setLayout(clustLayout);

	//fprintf(stderr, "I will be printed immediately1\n");
    connect(ui.dimreducttab,SIGNAL(currentChanged(int)),this,
            SLOT(switchDimReductOperator(int)));
	//fprintf(stderr, "I will be printed immediately2\n");
    //qDebug()<<"test"<<"\n";


}

DimReductCtrlDlg::~DimReductCtrlDlg()
{
    SAFE_DELETE(sbbDlg);
    SAFE_DELETE(clstDlg);
    //SAFE_DELETE(hmpDlg);
}

void DimReductCtrlDlg::switchDimReductOperator(int activeTab){
	//if activeTab = -1, meaning no tab is currently active;
	//fprintf(stderr, "activeTab %i\n",activeTab);
	//Pipeline pl = m_pm->getPipeline(pID);

    Operator* interringView = m_pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMTREE);
    Operator* interringReduct = m_pl->getMainTransformation()->getOperator(PL_DIMR_OP_DRVIEW);
    Operator* clusterView = m_pl->getMainTransformation()->getOperator(PL_DIMR_OP_DIMCLUST);
    Operator* clusterReduct = m_pl->getMainTransformation()->getOperator(PL_DIMR_OP_DRCLSTVIEW);

	if(ui.dimreducttab->currentWidget() == ui.interringtab){
        qDebug()<<"activeTab = interringtab\n";
        interringView->setEnabled(true);
        interringReduct->setEnabled(true);

        clusterView->setEnabled(false);
        clusterReduct->setEnabled(false);

        //switching tab triggers the event that
        //the tree is rebuilt,
        //seems plausible for now;
        //m_pm->assemble(m_pipelineID, PL_DIMR_OP_DIMTREE);


        //reassemble  here
		//refreshview;
	}


    if(ui.dimreducttab->currentWidget() == ui.clusteringtab){
        qDebug()<<"activeTab = clusteringtab";
        interringView->setEnabled(false);
        interringReduct->setEnabled(false);

        clusterView->setEnabled(true);
        clusterReduct->setEnabled(true);

        //switching tab triggers the event that,
        //the cluster layout of the dimensions is rebuilt,
        //seems plausible for now;
        //m_pm->assemble(m_pipelineID, PL_DIMR_OP_DIMTREE);

	}

	//testing code, disable the interring reduction,
	//but the tree generating operator is active,
	//and it must be active to generate the hierarchical tree.

}
