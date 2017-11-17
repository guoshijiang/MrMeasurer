#ifndef TTIMEOUT_H
#define TTIMEOUT_H

#include <QWidget>

namespace Ui {
class ttimeout;
}

class ttimeout : public QWidget
{
    Q_OBJECT

public:
    explicit ttimeout(QWidget *parent = 0);
    ~ttimeout();

private:
    Ui::ttimeout *ui;
};

#endif // TTIMEOUT_H
