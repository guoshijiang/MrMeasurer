#include "timerdialog.h"
#include "ui_timerdialog.h"

TimerDialog::TimerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimerDialog)
{
    ui->setupUi(this);
}

TimerDialog::~TimerDialog()
{
    delete ui;
}
