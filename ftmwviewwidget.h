#ifndef FTMWVIEWWIDGET_H
#define FTMWVIEWWIDGET_H

#include <QWidget>

#include <QList>

#include "fid.h"

class FtmwConfig;

namespace Ui {
class FtmwViewWidget;
}

class FtmwViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FtmwViewWidget(QWidget *parent = 0);
    ~FtmwViewWidget();
    void prepareForExperiment(const FtmwConfig config);

signals:
    void rollingAverageShotsChanged(int);
    void rollingAverageReset();

public slots:
    void newFidList(QList<Fid> fl);
    void updateShotsLabel(qint64 shots);
    void showFrame(int num);
    void fidTest(Fid f);


private:
    Ui::FtmwViewWidget *ui;

    QList<Fid> d_fidList;
};

#endif // FTMWVIEWWIDGET_H
