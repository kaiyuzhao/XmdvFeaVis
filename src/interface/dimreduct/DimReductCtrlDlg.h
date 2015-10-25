#ifndef  DIMREDUCTCTRLDLG_H
#define DIMREDUCTCTRLDLG_H

#include <QtGui/QDialog>
#include "ui_dimreductctrldlg.h"

class XmdvToolMainWnd;
class PipelineManager;
class Pipeline;
class InterringControl;
class ManualClustCtrlWidget;
//class HeatmapClustCtrlWidget;

class DimReductCtrlDlg : public QDialog
{
    Q_OBJECT

public:
    DimReductCtrlDlg(XmdvToolMainWnd* mainWnd, PipelineManager *pm, int pID);
    virtual ~DimReductCtrlDlg();

private:
    Ui::DimReductControlClass ui;

    PipelineManager* m_pm;
    Pipeline* m_pl;
    int m_pipelineID;

    InterringControl* sbbDlg;
    ManualClustCtrlWidget* clstDlg;

private slots:
	/**
	 * switch to the corresponding dimension reduction operator,
	 * based on with tab is activated
	 * currently, Jing Yang's InterRing and Kaiyu's cluster based
	 * dimension reduction are supported in two separated tabs
	 */
	void switchDimReductOperator(int activeTabID);

};

#endif // DIMREDUCTCONTROL_H
