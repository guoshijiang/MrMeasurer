#include "ttimeout.h"
#include "ui_ttimeout.h"

ttimeout::ttimeout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ttimeout)
{
    ui->setupUi(this);
}

ttimeout::~ttimeout()
{
    delete ui;
}
