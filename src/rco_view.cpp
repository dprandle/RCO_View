#include <QStandardItemModel>

#include "rco_view.h"
#include "ui_rco_view.h"

RCO_View::RCO_View(QWidget *parent):
QMainWindow(parent),
ui(new Ui::RCO_View),
model_(new QStandardItemModel)
{
   ui->setupUi(this);
   ui->actionUpdate_Firmware->setEnabled(false);
}

RCO_View::~RCO_View()
{
    delete ui;
}

#include <moc_rco_view.cpp>