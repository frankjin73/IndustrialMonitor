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
		m_ths.clear();
		for (int row = 0; row < ui->thresholdTableWidget->rowCount(); ++row) {
			quint8 id = -1;
			auto itemID = ui->thresholdTableWidget->item(row,0);
			if (itemID) {
				id = static_cast<quint8>(itemID->text().toInt());
			}

			Threshold th;
			auto itemName = ui->thresholdTableWidget->item(row, 1);
			if (itemName) {
				th.sensorName = itemName->text();
			}

			auto min = qobject_cast<QDoubleSpinBox *>(ui->thresholdTableWidget->cellWidget(row, 2));
			if (min) {
				th.minValue = min->value();
			}

			auto max = qobject_cast<QDoubleSpinBox*>(ui->thresholdTableWidget->cellWidget(row, 3));
			if (max) {
				th.maxValue = max->value();
			}

			auto checkWidget = ui->thresholdTableWidget->cellWidget(row, 4);
			QCheckBox* checkBox = checkWidget->findChild<QCheckBox*>();
			if (checkBox) {
				th.enabled = checkBox->isChecked();
			}

			m_ths.insert(id,th);
		}
		
		
		// 关闭对话框并返回Accepted
		done(QDialog::Accepted);
	});

	connect(ui->resetButton, &QPushButton::clicked, this, [this] {
		 m_ths = getDefaultThresholds();
		 initUi();
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
	ui->thresholdTableWidget->setColumnWidth(0, 80);
	ui->thresholdTableWidget->setColumnWidth(1, 80);
	ui->thresholdTableWidget->setColumnWidth(2, 80);
	ui->thresholdTableWidget->setColumnWidth(3, 80);
	ui->thresholdTableWidget->setColumnWidth(4, 80);

	int row = 0;
	for (auto it = m_ths.cbegin(); it != m_ths.cend(); ++it) {
		auto th = it.value();
		QTableWidgetItem* itemCh = new QTableWidgetItem(QString("%1").arg(it.key()));
		itemCh->setFlags(itemCh->flags() & ~Qt::ItemIsEditable); // 通道号禁止修改
		itemCh->setTextAlignment(Qt::AlignCenter); // 水平居中 + 垂直居中
		ui->thresholdTableWidget->setItem(row, 0, itemCh);

		// 名称
		QTableWidgetItem* name = new QTableWidgetItem(th.sensorName);
		name->setFlags(name->flags() & ~Qt::ItemIsEditable); 
		name->setTextAlignment(Qt::AlignCenter); // 水平居中 + 垂直居中
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

		QCheckBox* checkBox = new QCheckBox(ui->thresholdTableWidget);

		// 为了让 CheckBox 在单元格里居中，可以包一层 Widget
		QWidget* checkWidget = new QWidget();
		QHBoxLayout* checkLayout = new QHBoxLayout(checkWidget);
		checkLayout->addWidget(checkBox);
		checkLayout->setAlignment(Qt::AlignCenter);
		checkLayout->setContentsMargins(0, 0, 0, 0);
		
		checkBox->setEnabled(true);
		checkBox->setChecked(th.enabled);
		setRowEnable(row, th.enabled);
		ui->thresholdTableWidget->setCellWidget(row, 4, checkWidget); // 塞入控件
		connect(checkBox, &QCheckBox::toggled, this, [=](bool checked) {
			setRowEnable(row, checked);
		});

		++row;
	}
}

void AlarmSettingDialog::setRowEnable(int row, bool checked) {
	QTableWidgetItem* ID = ui->thresholdTableWidget->item(row, 0);
	ID->setFlags(checked ? (ID->flags() | Qt::ItemIsEnabled)
						 : (ID->flags() & ~Qt::ItemIsEnabled));

	QTableWidgetItem* name = ui->thresholdTableWidget->item(row, 1);
	name->setFlags(checked ? (name->flags() | Qt::ItemIsEnabled)
						   : (name->flags() & ~Qt::ItemIsEnabled));

	QWidget* min = ui->thresholdTableWidget->cellWidget(row, 2);
	min->setEnabled(checked);

	QWidget* max = ui->thresholdTableWidget->cellWidget(row, 3);
	max->setEnabled(checked);
}
