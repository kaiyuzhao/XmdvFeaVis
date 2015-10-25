#ifndef CLUSTPARAADJUSTINGDLG_H
#define CLUSTPARAADJUSTINGDLG_H




#include <QDialog>

class DimRddtCluster;

namespace Ui {
class ClustParaAdjustingDlg;
}

class ClustParaAdjustingDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit ClustParaAdjustingDlg(QWidget *parent, DimRddtCluster* dimClust);
    ~ClustParaAdjustingDlg();
    
private slots:
    void onDistValueChanged(int value);
    void onNPtsValueChanged(int value);

private:
    Ui::ClustParaAdjustingDlg *ui;
    DimRddtCluster* m_dimClust;
};

#endif // CLUSTPARAADJUSTINGDLG_H
