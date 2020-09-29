#include <dialog_firmware_settings.h>
#include <ui_dialog_firmware_settings.h>

Dialog_Firmware_Settings::Dialog_Firmware_Settings(QWidget *parent):
QDialog(parent),
ui(new Ui::Dialog_Firmware_Settings)
{
   ui->setupUi(this);
}

Dialog_Firmware_Settings::~Dialog_Firmware_Settings()
{
    delete ui;
}

#include <moc_dialog_firmware_settings.cpp>