#include "ClustParaAdjustingDlg.h"
#include "ui_ClustParaAdjustingDlg.h"

#include "data/dimreduct/DimRddtCluster.h"

#include <QString>

ClustParaAdjustingDlg::ClustParaAdjustingDlg(QWidget *parent,
                                             DimRddtCluster* dimClust) :
    QDialog(parent),
    ui(new Ui::ClustParaAdjustingDlg),
    m_dimClust(dimClust)
{
    ui->setupUi(this);
    ui->distSldr->setValue(20);
    ui->distSldr->setSingleStep(10);
    ui->distVal->setText(QString("0.2"));
    ui->minPtsSldr->setValue(0);
    ui->minPtsSldr->setSingleStep(10);
    ui->minPtsVal->setText(QString("0"));
    connect(ui->distSldr,SIGNAL(valueChanged(int)),
            this,SLOT(onDistValueChanged(int)));
    connect(ui->minPtsSldr,SIGNAL(valueChanged(int)),
            this,SLOT(onNPtsValueChanged(int)));

    connect(ui->distSldr,SIGNAL(valueChanged(int)),
            m_dimClust,SLOT(updateClustDistParas(int)),Qt::QueuedConnection);
    connect(ui->minPtsSldr,SIGNAL(valueChanged(int)),
            m_dimClust,SLOT(updateClustMptsParas(int)),Qt::QueuedConnection);
}

ClustParaAdjustingDlg::~ClustParaAdjustingDlg()
{
    delete ui;
}

void ClustParaAdjustingDlg::onDistValueChanged(int value){
    //range is 0 to 99
    double val = (double)value/(double)99.0;
    QString valQstr = QString::number(val,'g',3);
    ui->distVal->setText(valQstr);
    ui->distVal->repaint();
}

void ClustParaAdjustingDlg::onNPtsValueChanged(int value){
    //range is 0 to 99
    double val = (double)value/(double)99.0;
    double n = val*(double)m_dimClust->getDimSize();
    int npts = (int)n;
    QString valQstr = QString::number(npts);
    ui->minPtsVal->setText(valQstr);
    ui->minPtsVal->repaint();
}
