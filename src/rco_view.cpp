#include <QStandardItemModel>
#include <QStandardItem>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <qdebug.h>

#include "rco_view.h"
#include "ui_rco_view.h"

RCO_View::RCO_View(QWidget * parent) : QMainWindow(parent), ui(new Ui::RCO_View), model_(new QStandardItemModel)
{
    ui->setupUi(this);
    ui->actionUpdate_Firmware->setEnabled(false);

    model_->setHorizontalHeaderLabels({"Node", "Firmware", "IP Address"});
    build_treeview_from_serial_();
}

RCO_View::~RCO_View()
{
    delete ui;
}

void RCO_View::build_treeview_from_serial_()
{
    auto spi_list = QSerialPortInfo::availablePorts();
    ui->treeView->setModel(nullptr);

    auto spi_iter = spi_list.begin();
    while (spi_iter != spi_list.end())
    {
        qDebug() << "Vendor id: " << spi_iter->vendorIdentifier() << "  Product id: " << spi_iter->productIdentifier();
        if (!spi_iter->hasProductIdentifier() || !spi_iter->hasVendorIdentifier() || spi_iter->portName().contains("cu"))
        {
            qDebug() << "Item " << spi_iter->portName() << " does not have vendor or product id";
            spi_iter = spi_list.erase(spi_iter);
            continue;
        }
        ++spi_iter;
    }

    model_->setRowCount(spi_list.size());
    int row_ind = 0;
    spi_iter = spi_list.begin();
    while (spi_iter != spi_list.end())
    {
        QStandardItem * node_item = model_->item(row_ind, COLUMN_NODE);
        if (!node_item)
        {
            node_item = new QStandardItem;
            model_->setItem(row_ind, node_item);
        }
        node_item->setText(spi_iter->portName());
        ++spi_iter;
        ++row_ind;
    }

    ui->treeView->setModel(model_);
}

#include <moc_rco_view.cpp>