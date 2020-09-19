#pragma once
#include <QMainWindow>

namespace Ui
{
class RCO_View;
}

class QStandardItemModel;
class QSerialPort;

const int COLUMN_NODE = 0;
const int COLUMN_FIRMWARE = 1;
const int COLUMN_IP = 2;

class RCO_View : public QMainWindow
{
    Q_OBJECT

  public:
    explicit RCO_View(QWidget * parent = 0);
    ~RCO_View();

    QSerialPort * selected_serial_port();

public slots:
    void on_actionUpdate_Firmware_triggered();

  private:
    void build_treeview_from_serial_();

    void data_ready();

    Ui::RCO_View * ui;
    QStandardItemModel * model_;
    QMap<QString, QSerialPort*> serial_ports;
};