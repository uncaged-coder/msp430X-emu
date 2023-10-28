#pragma once

#include "UI.h"

class ICUWidget : public QWidget
{
    Q_OBJECT
public:
    ICUWidget(QWidget *parent = nullptr);
    void moveActuator(int tick);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int actuatorTick;
    int mapTickToDiameter(int tick);
};

class UIIcu : public UI
{
    Q_OBJECT

public:
    UIIcu(QWidget *parent = nullptr);
    int run();

private:
    ICUWidget *icuWidget;
};
