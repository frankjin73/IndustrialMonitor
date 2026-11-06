#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include "ui_AlarmSettingDialog.h"
#include "Threshold.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AlarmSettingDialogClass; };
QT_END_NAMESPACE

class AlarmSettingDialog : public QDialog
{
	Q_OBJECT

public:
	AlarmSettingDialog(QWidget* parent = nullptr, const QMap<quint8, Threshold>& ths = {});
	~AlarmSettingDialog();

private:
	void initUi();
	Ui::AlarmSettingDialogClass *ui;
	QMap<quint8, Threshold> m_ths;
};

