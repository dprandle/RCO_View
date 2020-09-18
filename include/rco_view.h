#pragma once
#include <QMainWindow>

namespace Ui
{
class RCO_View;
}

class QStandardItemModel;

const int COLUMN_NODE = 0;
const int COLUMN_FIRMWARE = 1;
const int COLUMN_IP = 2;

class RCO_View : public QMainWindow
{
    Q_OBJECT

  public:
    explicit RCO_View(QWidget * parent = 0);
    ~RCO_View();

  private:
    void build_treeview_from_serial_();

    Ui::RCO_View * ui;
    QStandardItemModel * model_;
};