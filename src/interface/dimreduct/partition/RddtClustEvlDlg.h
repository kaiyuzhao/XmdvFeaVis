#ifndef RDDTCLUSTEVLDLG_H
#define RDDTCLUSTEVLDLG_H

#include "ui_RddtClustEvlDlg.h"

#include <QDialog>
#include <QBoxLayout>


class XmdvToolMainWnd;
class PipelineManager;
class Pipeline;
class RddcEvlWidget;

class RddtClustEvlDlg : public QDialog
{
    Q_OBJECT
    
public:
    RddtClustEvlDlg(XmdvToolMainWnd* mainWnd, PipelineManager *pm, int pID);
    ~RddtClustEvlDlg();
    
private:

    Ui::RddtClustEvlDlg ui;
    PipelineManager* m_pm;
    Pipeline* m_pl;
    int m_pipelineID;

    RddcEvlWidget* evlWidget;
    QVBoxLayout* evalLayout;

};

#endif // RDDTCLUSTEVLDLG_H
