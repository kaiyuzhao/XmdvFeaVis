#ifndef CLUSTHEATMAPVIEWDLG_H
#define CLUSTHEATMAPVIEWDLG_H

#include <QDialog>


class QVBoxLayout;
class HeatmapClustCtrlWidget;
class XmdvToolMainWnd;
class PipelineManager;
class Pipeline;

namespace Ui {
class ClustHeatMapViewDlg;
}

class ClustHeatMapViewDlg : public QDialog
{
    Q_OBJECT
    
public:

    explicit ClustHeatMapViewDlg(XmdvToolMainWnd *mainWnd,
                                 PipelineManager* pm, int pID);
    ~ClustHeatMapViewDlg();
    
private:
    Ui::ClustHeatMapViewDlg *ui;
    QVBoxLayout *mainLayout;
    HeatmapClustCtrlWidget* hmpDlg;
};

#endif // CLUSTHEATMAPVIEWDLG_H
