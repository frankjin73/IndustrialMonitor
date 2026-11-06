#include "AlarmSettingDialog.h"

AlarmSettingDialog::AlarmSettingDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::AlarmSettingDialogClass())
{
	ui->setupUi(this);


	initUi();


	connect(ui->cancelButton, &QPushButton::clicked, this, [this]{
		// 关闭对话框并返回Rejected ,相当于直接用reject();
		done(QDialog::Rejected); 
	});

	connect(ui->okButton, &QPushButton::clicked, this, [this] {
		// 关闭对话框并返回Accepted
		done(QDialog::Accepted);
	});

	connect(ui->resetButton, &QPushButton::clicked, this, [this] {

	});

}

AlarmSettingDialog::~AlarmSettingDialog()
{
	delete ui;
}

void AlarmSettingDialog::initUi() {
	QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
}
