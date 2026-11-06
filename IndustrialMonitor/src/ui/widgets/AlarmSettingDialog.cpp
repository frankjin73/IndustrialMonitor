#include "AlarmSettingDialog.h"

AlarmSettingDialog::AlarmSettingDialog(QWidget* parent, const QMap<quint8, Threshold>& ths)
	: QDialog(parent)
	, ui(new Ui::AlarmSettingDialogClass())
	, m_ths(ths)
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
		auto defaultThs = getDefaultThresholds();
	});

}

AlarmSettingDialog::~AlarmSettingDialog()
{
	delete ui;
}

void AlarmSettingDialog::initUi() {
	ui->thresholdTableWidget->clearContents();
	ui->thresholdTableWidget->setRowCount(m_ths.size());
	ui->thresholdTableWidget->verticalHeader()->setVisible(false);

	int row = 0;
	for (auto it = m_ths.cbegin(); it != m_ths.cend(); ++it) {
		auto th = it.value();
		QTableWidgetItem* itemCh = new QTableWidgetItem(QString("%1").arg(it.key()));
		itemCh->setFlags(itemCh->flags() & ~Qt::ItemIsEditable); // 通道号禁止修改
		ui->thresholdTableWidget->setItem(row, 0, itemCh);

		// 名称
		QTableWidgetItem* name = new QTableWidgetItem(th.sensorName);
		name->setFlags(name->flags() & ~Qt::ItemIsEditable); 
		ui->thresholdTableWidget->setItem(row, 1, name);

		QDoubleSpinBox* spinMin = new QDoubleSpinBox(ui->thresholdTableWidget);
		spinMin->setRange(-9999.0, 9999.0);
		spinMin->setDecimals(2);	// 设置保留2位小数
		spinMin->setSingleStep(0.1);	// 设置步长
		spinMin->setValue(th.minValue);
		ui->thresholdTableWidget->setCellWidget(row, 2, spinMin); // 塞入控件

		QDoubleSpinBox* spinMax = new QDoubleSpinBox(ui->thresholdTableWidget);
		spinMax->setRange(-9999.0, 9999.0);
		spinMax->setDecimals(2);
		spinMax->setSingleStep(0.1);
		spinMax->setValue(th.maxValue);
		ui->thresholdTableWidget->setCellWidget(row, 3, spinMax);

		++row;
	}
}
