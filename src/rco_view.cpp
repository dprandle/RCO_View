#include <QStandardItemModel>
#include <QStandardItem>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <qdebug.h>

#include "shared_structs.h"
#include "rco_view.h"
#include "ui_rco_view.h"

RCO_View::RCO_View(QWidget * parent) : QMainWindow(parent), ui(new Ui::RCO_View), model_(new QStandardItemModel)
{
    ui->setupUi(this);
    ui->actionUpdate_Firmware->setEnabled(false);

    model_->setHorizontalHeaderLabels({"Node", "Firmware", "IP Address"});
    build_treeview_from_serial_();

    auto func = [=](const QItemSelection & selected, const QItemSelection &) {
        ui->actionUpdate_Firmware->setDisabled(selected.isEmpty() || selected.first().isEmpty());
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
        connect(sp, &QSerialPort::readyRead, this, &RCO_View::data_ready);
        serial_ports[spi_iter->portName()] = sp;
        ++spi_iter;
        ++row_ind;
    }

    ui->treeView->setModel(model_);
}

void RCO_View::on_actionUpdate_Firmware_triggered()
{
    auto sp = selected_serial_port();
    if (!sp)
        return;
    if (!sp->open(QSerialPort::ReadWrite))
    {
        ui->textEdit->insertPlainText("Error opening port " + sp->portName() + ": " + sp->errorString());
        return;
    }
    sp->write("FWU\r");
    Firmware_Header fwh;
    fwh.byte_size = 70000;
    fwh.v_major = 1;
    fwh.v_minor = 0;
    fwh.v_minor = 0;
    sp->write((char*)fwh.data, FIRMWARE_HEADER_SIZE);

    char payload[70000];
    sp->write(payload, 70000);
}

void RCO_View::data_ready()
{
    QSerialPort * port = static_cast<QSerialPort *>(sender());
    QByteArray data = port->readAll();
    for (int i = 0; i < data.size(); ++i)
    {
        if (data[i] == '\n')
            port->close();
        else if (data[i] == '\b')
            ui->textEdit->textCursor().deletePreviousChar();
        else
            ui->textEdit->textCursor().insertText(data);
    }
}

#include <moc_rco_view.cpp>