/*
 * ManualClustControl.h
 *
 *  Created on: Nov 28, 2012
 *      Author: kaiyuzhao
 */


#ifndef MANUALCLUSTCTRLWIDGET_H
#define MANUALCLUSTCTRLWIDGET_H

#include <QtGui/QWidget>
#include <QLabel>
#include <QString>

#include "ui_ManualClustCtrlWidget.h"

class XmdvToolMainWnd;
class PipelineManager;
class Operator;

class DimRddtCluster;
class ManualClustDisplay;
class QScrollArea;
class ClustParaAdjustingDlg;

class ManualClustCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    ManualClustCtrlWidget(XmdvToolMainWnd *mainWnd,
                          PipelineManager* pm, int pID);
    ~ManualClustCtrlWidget();

private:
    Ui::ManualClustCtrlWin ui;
    QScrollArea* clustScrArea;

    QLabel* statusbar;

    DimRddtCluster* m_dimClust;

    ManualClustDisplay* m_clustDisplay;

    ClustParaAdjustingDlg* clustAdjDlg;

    Operator* clustViewQ;

    // The pointer to the main window
    XmdvToolMainWnd* m_mainWnd;
    // The pointer to the pipeline manager
    PipelineManager* m_pipelineManager;
    // The pipeline ID linked to this dialog
    int m_pipelineID;
public:
    void updateStatusBar(QString info);

public slots:
    void onClustBtnClicked();
    void onApplyReduction();
};

#endif // MANUALCLUSTCONTROL_H
