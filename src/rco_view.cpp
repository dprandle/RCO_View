#include <QStandardItemModel>
#include <QStandardItem>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <qdebug.h>

#include "shared_structs.h"
#include "rco_view.h"
#include "dialog_firmware_settings.h"
#include "ui_rco_view.h"

RCO_View * RCO_View::this_static = nullptr;

RCO_View::RCO_View(QWidget * parent) : QMainWindow(parent), ui(new Ui::RCO_View), dfs_(new Dialog_Firmware_Settings(this)), model_(new QStandardItemModel)
{
    this_static = this;

    ui->setupUi(this);

    model_->setHorizontalHeaderLabels({"Node", "Firmware", "IP Address"});
    build_treeview_from_serial_();

    auto func = [=](const QItemSelection & selected, const QItemSelection &) {
        dfs_->set_selected_port(selected_serial_port());
    };

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, func);
}

RCO_View::~RCO_View()
{
    while (serial_ports.begin() != serial_ports.end())
    {
        serial_ports.begin().value()->close();
        delete serial_ports.begin().value();
        serial_ports.erase(serial_ports.begin());
    }
    delete dfs_;
    delete ui;
}

QSerialPort * RCO_View::selected_serial_port()
{
    auto sel = ui->treeView->selectionModel()->selectedRows();
    if (sel.isEmpty())
        return nullptr;
    auto fiter = serial_ports.find(sel.first().data(Qt::EditRole).toString());
    if (fiter != serial_ports.end())
    {
        return *fiter;
    }
    return nullptr;
}

void RCO_View::console_print(const QString & param)
{
    rco_view.ui->textEdit->textCursor().insertText(param);
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
        QSerialPort * sp = new QSerialPort(*spi_iter);
        sp->setBaudRate(QSerialPort::Baud9600);
        sp->setFlowControl(QSerialPort::HardwareControl);
        sp->setDataBits(QSerialPort::Data8);
        sp->setStopBits(QSerialPort::OneStop);
        sp->setParity(QSerialPort::NoParity);
        serial_ports[spi_iter->portName()] = sp;
        ++spi_iter;
        ++row_ind;
    }

    ui->treeView->setModel(model_);
}

void RCO_View::on_actionUpdate_Firmware_triggered()
{
    dfs_->show();
    dfs_->raise();
}

RCO_View & RCO_View::inst()
{
    return *this_static;
}

#include <moc_rco_view.cpp>