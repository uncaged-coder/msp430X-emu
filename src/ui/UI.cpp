#include <QtCore/QDir>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>

#include "UI.h"

UI::UI(QWidget *parent) : QMainWindow(parent)
{
    setFixedSize(800, 600); // Set the size of the window
    //
    tabWidget = new QTabWidget(this);
    mainTab = new QWidget();
    graphTab = new QWidget();
    statsTab = new QWidget();

    tabWidget->addTab(mainTab, "Main");
    tabWidget->addTab(graphTab, "Vbat Graph");
    tabWidget->addTab(statsTab, "Stats");
    setCentralWidget(tabWidget);

    // Prepare and populate the dropdown menu (rom selector) with available roms
    romDropdown = new QComboBox(this);
    QStringList roms =
        QDir("roms").entryList(QStringList() << "*.hex", QDir::Files);
    romDropdown->addItems(roms);

    // Add table widget for data display
    tableWidget = new QTableWidget(this);
    // Add code to initialize and fill the table here

    // renderArea = new RenderArea(this);

    // Add Vbat graph view
    // vbatChartView = new QtCharts::QChartView(this);

    // QVBoxLayout *layout = new QVBoxLayout(mainTab);
    // layout->addWidget(romDropdown);
    // layout->addWidget(renderArea);
    // layout->addWidget(tableWidget);
}

int UI::run()
{
    this->show();
    return 0;
}
