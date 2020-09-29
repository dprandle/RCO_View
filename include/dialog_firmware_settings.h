#pragma once
#include <QDialog>

namespace Ui {
class Dialog_Firmware_Settings;
}

class Dialog_Firmware_Settings : public QDialog
{
   Q_OBJECT
   
  public:
   
   explicit Dialog_Firmware_Settings(QWidget *parent = 0);
   ~Dialog_Firmware_Settings();
   
  private:
   Ui::Dialog_Firmware_Settings * ui;
};