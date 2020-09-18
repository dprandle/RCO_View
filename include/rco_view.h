#pragma once
#include <QMainWindow>

namespace Ui {
class RCO_View;
}

class QStandardItemModel;

class RCO_View : public QMainWindow
{
   Q_OBJECT
   
  public:
   
   explicit RCO_View(QWidget *parent = 0);
   ~RCO_View();
   
  private:
   Ui::RCO_View * ui;
   QStandardItemModel * model_;
};