#pragma once
#include <QDialog>

#include "shared_structs.h"

namespace Ui
{
class Dialog_Firmware_Settings;
}

class QSerialPort;

class Dialog_Firmware_Settings : public QDialog
{
    Q_OBJECT

  public:
    explicit Dialog_Firmware_Settings(QWidget * parent = 0);
    ~Dialog_Firmware_Settings();

    void clear();

    void set_selected_port(QSerialPort * port);

  public slots:
    void on_pushButton_refresh_clicked();

    void on_pushButton_upload_clicked();

    void on_toolButton_browse_clicked();

    void on_pushButton_boot_into_clicked();

    void on_toolButton_cancel_clicked();

  private:
    void firmware_update_data_ready();

    void firmware_version_response();

    void boot_into_data_ready();

    void populate_list_widget();

    bool reset_fw_update;

    bool reset_fw_get;

    bool reset_fw_boot;

    int selected_remote_firmware_;
    Ui::Dialog_Firmware_Settings * ui;
    QSerialPort * selected_port_;
    QVector<Firmware_Header> remote_versions_;
    Firmware_Header fwh_;
    QByteArray loaded_updated_fw_data_;
};