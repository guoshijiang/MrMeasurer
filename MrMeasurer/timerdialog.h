#ifndef TIMERDIALOG_H
#define TIMERDIALOG_H

#include <QDialog>

namespace Ui {
class TimerDialog;
}

class TimerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimerDialog(QWidget *parent = 0);
    ~TimerDialog();

private:
    Ui::TimerDialog *ui;
};

#endif // TIMERDIALOG_H
