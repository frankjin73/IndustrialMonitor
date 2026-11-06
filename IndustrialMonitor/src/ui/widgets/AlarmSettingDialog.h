#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include "ui_AlarmSettingDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AlarmSettingDialogClass; };
QT_END_NAMESPACE

class AlarmSettingDialog : public QDialog
{
	Q_OBJECT

public:
	AlarmSettingDialog(QWidget *parent = nullptr);
	~AlarmSettingDialog();

private:
	void initUi();
	Ui::AlarmSettingDialogClass *ui;
};

