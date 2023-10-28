#pragma once

#include <QtWidgets/QComboBox>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
//#include <QtCharts/QChartView>

class UI : public QMainWindow
{
    Q_OBJECT

public:
    UI(QWidget *parent = nullptr);

    int run();

protected:
    QTabWidget *tabWidget;
    QWidget *mainTab;
    QWidget *graphTab;
    QWidget *statsTab;
    // RenderArea *renderArea;
    QTableWidget *tableWidget;
    QComboBox *romDropdown;
    // QtCharts::QChartView* vbatChartView;

    void populateRoms();
};
