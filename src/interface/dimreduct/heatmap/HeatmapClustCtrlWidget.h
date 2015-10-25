#ifndef HEATMAPCLUSTCTRLWIDGET_H
#define HEATMAPCLUSTCTRLWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include "ui_HeatmapClustCtrlWidget.h"

/*namespace Ui {
class HeatmapClustCtrlWidget;
}*/

class XmdvToolMainWnd;
class PipelineManager;
class Operator;

class HeatmapDisplay;
class HeatmapColorBar;
class DimRddtCluster;
class QScrollArea;
class QVBoxLayout;

class HeatmapClustCtrlWidget : public QWidget
{
    Q_OBJECT
    
public:
    HeatmapClustCtrlWidget(XmdvToolMainWnd *mainWnd,
                                    PipelineManager* pm, int pID);
    ~HeatmapClustCtrlWidget();
    
private:
    Ui::HeatmapClustCtrlWidget ui;
    QVBoxLayout* scrAreaLayout;
    QScrollArea* hmpScrArea;

    QLabel* statusbar;

    DimRddtCluster* m_dimClust;
    HeatmapDisplay* m_heatmapDisp;
    HeatmapColorBar* m_heatmapColorBar;

    Operator* clustViewQ;

    // The pointer to the main window
    XmdvToolMainWnd* m_mainWnd;
    // The pointer to the pipeline manager
    PipelineManager* m_pipelineManager;
    // The pipeline ID linked to this dialog
    int m_pipelineID;

public:
    void setDimClust(DimRddtCluster* dimClust);
    void updateStatusBar(QString info);

public slots:
    void onApplyReduction();
};

#endif // HEATMAPCLUSTCTRLWIDGET_H
