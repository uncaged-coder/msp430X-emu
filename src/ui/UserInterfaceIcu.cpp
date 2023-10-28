#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

#include "UserInterfaceIcu.h"

ICUWidget::ICUWidget(QWidget *parent) : QWidget(parent), actuatorTick(0) {}

void ICUWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;

    // Draw the gray rectangle with text
    QRect rect(50, 50, 200, 100);
    painter.setBrush(QBrush(Qt::gray));
    painter.drawRect(rect);

    QFont font = painter.font();
    font.setPointSize(10); // Change the font size
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter,
                     "msp430-emu\nby toto titi gigi@giyi.gt");

    // Draw the line connected to the rectangle
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.drawLine(QPoint(rect.right(), rect.center().y()),
                     QPoint(rect.right() + 150, rect.center().y()));

    // Draw the circle (actuator)
    int diameter = mapTickToDiameter(actuatorTick);
    painter.drawEllipse(QPoint(rect.right() + 150, rect.center().y()), diameter,
                        diameter);
}

void ICUWidget::moveActuator(int tick)
{
    // validate tick value
    if (tick < 0)
        tick = 0;
    if (tick > 255)
        tick = 255;
    actuatorTick = tick;
    update(); // triggers a repaint
}

int ICUWidget::mapTickToDiameter(int tick)
{
    // Map tick value [0, 255] to diameter [10, 100]
    // This can be adjusted as needed
    return 10 + (tick * 90) / 255;
}

UIIcu::UIIcu(QWidget *parent) : UI(parent)
{
    icuWidget = new ICUWidget(this);
    icuWidget->setFixedSize(450, 200);
    QVBoxLayout *layout = new QVBoxLayout(mainTab);
    layout->addWidget(romDropdown);
    layout->addWidget(icuWidget);
    layout->addWidget(tableWidget);
}

int UIIcu::run()
{
    this->show();
    return 0;
}
