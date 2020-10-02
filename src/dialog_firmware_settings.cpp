#include <QDebug>
#include <QFileDialog>
#include <QSerialPort>
#include <QFileInfo>

#include "rco_view.h"
#include "dialog_firmware_settings.h"
#include "ui_dialog_firmware_settings.h"

Dialog_Firmware_Settings::Dialog_Firmware_Settings(QWidget * parent)
    : QDialog(parent),
      reset_fw_update(false),
      reset_fw_get(false),
      reset_fw_boot(false),
      selected_remote_firmware_(-1),
      ui(new Ui::Dialog_Firmware_Settings),
      selected_port_(nullptr)
{
    ui->setupUi(this);
    ui->toolButton_cancel->setHidden(true);
    ui->label_uploading->setHidden(true);
    ui->progressBar_firmware->setHidden(true);

    connect(ui->listWidget_firmware, &QListWidget::currentItemChanged, [=](QListWidgetItem * cur, QListWidgetItem *) {
        if (cur)
        {
            QVariant fwind = cur->data(Qt::UserRole);
            if (fwind.isValid())
            {
                ui->pushButton_boot_into->setEnabled(true);
                selected_remote_firmware_ = fwind.toInt();
            }
        }
    });
}

Dialog_Firmware_Settings::~Dialog_Firmware_Settings()
{
    delete ui;
}

void Dialog_Firmware_Settings::on_pushButton_refresh_clicked()
{
    if (!selected_port_->open(QSerialPort::ReadWrite))
    {
        dtext("Error opening port " + selected_port_->portName() + ": " + selected_port_->errorString());
        return;
    }
    selected_port_->clear();

    setEnabled(false);
    ui->listWidget_firmware->clear();
    connect(selected_port_, &QSerialPort::readyRead, this, &Dialog_Firmware_Settings::firmware_version_response);
    dtext("\nRequesting firmware versions...");
    selected_port_->write("GFV\r");
}

void Dialog_Firmware_Settings::clear()
{
    ui->listWidget_firmware->clear();
    remote_versions_.clear();
}

void Dialog_Firmware_Settings::set_selected_port(QSerialPort * port)
{
    clear();
    selected_port_ = port;
    ui->pushButton_upload->setEnabled(port != nullptr && !loaded_updated_fw_data_.isEmpty());
    ui->pushButton_refresh->setEnabled(port != nullptr);
    if (selected_port_)
        setWindowTitle("Remote Firmware (" + selected_port_->portName() + ")");
}

void Dialog_Firmware_Settings::on_pushButton_upload_clicked()
{
    if (loaded_updated_fw_data_.isEmpty())
    {
        dtext("Can't upload - no firmware data loaded");
    }

    if (!selected_port_->open(QSerialPort::ReadWrite))
    {
        dtext("Error opening port " + selected_port_->portName() + ": " + selected_port_->errorString());
        return;
    }
    selected_port_->clear();

    ui->toolButton_cancel->setHidden(false);
    ui->label_uploading->setHidden(false);
    ui->progressBar_firmware->setHidden(false);
    ui->progressBar_firmware->setValue(0);
    ui->groupBox_2->setEnabled(false);
    ui->pushButton_upload->setEnabled(false);
    ui->toolButton_browse->setEnabled(false);

    connect(selected_port_, &QSerialPort::readyRead, this, &Dialog_Firmware_Settings::firmware_update_data_ready);
    dtext("Beginning firmware upload...\n");
    selected_port_->write("FWU\r");
    selected_port_->write((char *)fwh_.data, FIRMWARE_HEADER_SIZE);
    selected_port_->write(loaded_updated_fw_data_.data(), loaded_updated_fw_data_.size());
}

void Dialog_Firmware_Settings::on_toolButton_browse_clicked()
{
    QString fname = QFileDialog::getOpenFileName(this, "Raspberry Pi Firmware", "../firmware", "*.rpi");
    if (fname.isEmpty())
    {
        dtext("Operation aborted by user!\n");
        return;
    }

    QFile firmware(fname);
    if (!firmware.open(QIODevice::ReadOnly))
    {
        dtext("Could not open file " + fname + "!\n");
        return;
    }
    QFileInfo finf(fname);
    parse_filename(finf.fileName().toUtf8().constData(), fwh_);

    loaded_updated_fw_data_ = firmware.readAll();
    firmware.close();
    fwh_.byte_size = loaded_updated_fw_data_.size();

    ui->lineEdit_version->setEnabled(true);
    ui->pushButton_upload->setEnabled(selected_port_ != nullptr);
    ui->lineEdit_version->setText("Firmware v" + QString::number(fwh_.v_major) + "." + QString::number(fwh_.v_minor) + "." + QString::number(fwh_.v_patch));
    ui->lineEdit_path->setText(fname);
}

void Dialog_Firmware_Settings::on_toolButton_cancel_clicked()
{
    reset_fw_update = true;
    firmware_update_data_ready();
}

void Dialog_Firmware_Settings::firmware_update_data_ready()
{
    static bool reading_progress = false;
    bool handled = false;

    if (reset_fw_update)
    {
        reset_fw_update = false;
        disconnect(selected_port_, &QSerialPort::readyRead, this, &Dialog_Firmware_Settings::firmware_update_data_ready);
        selected_port_->clear();
        selected_port_->close();

        ui->progressBar_firmware->setValue(0);
        ui->toolButton_cancel->setHidden(true);
        ui->label_uploading->setHidden(true);
        ui->progressBar_firmware->setHidden(true);
        ui->groupBox_2->setEnabled(true);
        ui->pushButton_upload->setEnabled(true);
        ui->toolButton_browse->setEnabled(true);

        reading_progress = false;
        return;
    }

    QByteArray data = selected_port_->readAll();
    for (int i = 0; i < data.size(); ++i)
    {
        if (data[i] == '\n')
        {
            dtext("Finished uploading firmware!\n");
            reset_fw_update = true;
            firmware_update_data_ready();
            on_pushButton_refresh_clicked();
            return;
        }
        else if (data[i] == '\r')
        {
            reading_progress = true;
            handled = true;
            dtext("\n");
        }
        else if (reading_progress)
        {
            handled = true;
            // Minus 12 to translate - never want newline char by accident!
            int8_t perc = data[i];
            perc = perc - 32;
            ui->progressBar_firmware->setValue(perc);
        }
    }
    if (!handled)
        dtext(data);
}

void Dialog_Firmware_Settings::firmware_version_response()
{
    static bool first_byte = true;
    static bool recieving_data = false;
    static int cur_ind = 0;
    bool handled = false;

    if (reset_fw_get)
    {
        reset_fw_get = false;
        disconnect(selected_port_, &QSerialPort::readyRead, this, &Dialog_Firmware_Settings::firmware_version_response);
        selected_port_->clear();
        selected_port_->close();
        first_byte = true;
        recieving_data = false;
        cur_ind = 0;
        setEnabled(true);
        return;
    }

    QByteArray data = selected_port_->readAll();

    for (int i = 0; i < data.size(); ++i)
    {
        if (recieving_data)
        {
            if (first_byte)
            {
                first_byte = false;
                int8_t sz = data[i];
                remote_versions_.resize(sz);
                dtext("Recieved " + QString::number(sz) + " versions of firmware\n");
                data.remove(i, 1);
                handled = true;
                continue;
            }

            int rem_ind = cur_ind / FIRMWARE_HEADER_SIZE;
            int fw_data_ind = cur_ind % FIRMWARE_HEADER_SIZE;
            if (!remote_versions_.isEmpty())
                remote_versions_[rem_ind].data[fw_data_ind] = data[i];

            ++cur_ind;
            if (cur_ind == FIRMWARE_HEADER_SIZE * remote_versions_.size() || remote_versions_.isEmpty())
            {
                dtext("Finished recieving remote firmware versions!\n");
                populate_list_widget();
                reset_fw_get = true;
                firmware_version_response();
                return;
            }
            handled = true;
        }
        else if (data[i] == '\r')
        {
            dtext("\n");
            recieving_data = true;
            handled = true;
        }
    }
    if (!handled)
        dtext(data);
}

void Dialog_Firmware_Settings::populate_list_widget()
{
    for (int i = 0; i < remote_versions_.size(); ++i)
    {
        QListWidgetItem * fw_item = new QListWidgetItem;
        const char * txt = parse_firmware_header(remote_versions_[i]);
        QString str(txt);
        if (remote_versions_[i].byte_size == 1)
            str += " (Currently in Use)";
        fw_item->setText(str);
        fw_item->setData(Qt::UserRole, i);
        ui->listWidget_firmware->addItem(fw_item);
    }
    if (ui->listWidget_firmware->count() == 0)
    {
        QListWidgetItem * fw_item = new QListWidgetItem;
        fw_item->setText("No firmware found on remote device!");
        fw_item->setData(Qt::UserRole, -1);
        ui->listWidget_firmware->addItem(fw_item);
    }
}

void Dialog_Firmware_Settings::on_pushButton_boot_into_clicked()
{
    if (selected_remote_firmware_ > remote_versions_.size())
    {
        dtext("Cannot reboot into firmware - invalid selection - this is a bug\n");
    }

    if (!selected_port_->open(QSerialPort::ReadWrite))
    {
        dtext("Error opening port " + selected_port_->portName() + ": " + selected_port_->errorString());
        return;
    }
    selected_port_->clear();

    setEnabled(false);
    ui->listWidget_firmware->clear();
    connect(selected_port_, &QSerialPort::readyRead, this, &Dialog_Firmware_Settings::boot_into_data_ready);
    dtext("\nRequesting reboot into firmware " + QString(parse_firmware_header(remote_versions_[selected_remote_firmware_])));
    selected_port_->write("RBUFW\r");
    selected_port_->write((char *)remote_versions_[selected_remote_firmware_].data, FIRMWARE_HEADER_SIZE);
}

void Dialog_Firmware_Settings::boot_into_data_ready()
{
    bool handled = false;
    static bool received_inital_ack = false;

    if (reset_fw_boot)
    {
        reset_fw_boot = false;
        received_inital_ack = false;
        disconnect(selected_port_, &QSerialPort::readyRead, this, &Dialog_Firmware_Settings::boot_into_data_ready);
        selected_port_->clear();
        selected_port_->close();
        setEnabled(true);
        return;
    }

    QByteArray data = selected_port_->readAll();

    for (int i = 0; i < data.size(); ++i)
    {
        if (data[i] == '\r')
        {
            if (received_inital_ack)
            {
                dtext("\nSuccessfully rebooted into firmware " + QString(parse_firmware_header(remote_versions_[selected_remote_firmware_])) + "\n");
                reset_fw_boot = true;
                boot_into_data_ready();
                on_pushButton_refresh_clicked();
                return;
            }
            else
            {
                dtext("\n");
                handled = true;
                received_inital_ack = true;
            }
        }
    }
    if (!handled)
        dtext(data);
}

#include <moc_dialog_firmware_settings.cpp>