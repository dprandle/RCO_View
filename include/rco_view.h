#pragma once
#include <QMainWindow>

#define rco_view RCO_View::inst()
#define dtext(param) rco_view.console_print(param)

namespace Ui
{
class RCO_View;
}

class QStandardItemModel;
class QSerialPort;
class Dialog_Firmware_Settings;

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

    static RCO_View & inst();

    void console_print(const QString & param);

    Ui::RCO_View * ui;

public slots:
    void on_actionUpdate_Firmware_triggered();

  private:
    void build_treeview_from_serial_();

    static RCO_View * this_static;
    Dialog_Firmware_Settings * dfs_;
    QStandardItemModel * model_;
    QMap<QString, QSerialPort*> serial_ports;
};