#ifndef RDDCEVLWIDGET_H
#define RDDCEVLWIDGET_H

#include <QWidget>
#include "ui_RddcEvlWidget.h"
/*namespace Ui {
class RddcEvlWidget;
}*/

class RddcEvlDisplay;
class XmdvToolMainWnd;
class PipelineManager;
class Operator;
class QScrollArea;
class DimRddtCluster;

class RddcEvlWidget : public QWidget
{
    Q_OBJECT
    
public:
    RddcEvlWidget(XmdvToolMainWnd *mainWnd,
                  PipelineManager* pm, int pID);
    ~RddcEvlWidget();
    
private:
    Ui::RddcEvlWidget ui;
    RddcEvlDisplay* m_rddcEvlDisplay;

    QScrollArea* evalScrArea;

    DimRddtCluster* m_dimClustPos;
    RddcEvlDisplay* m_clustDisplay;

    Operator* clustViewQ;

    // The pointer to the main window
    XmdvToolMainWnd* m_mainWnd;
    // The pointer to the pipeline manager
    PipelineManager* m_pipelineManager;
};

#endif // RDDCEVLWIDGET_H
