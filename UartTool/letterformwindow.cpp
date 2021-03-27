#include "letterformwindow.h"
#include "ui_letterformwindow.h"

letterFormWindow::letterFormWindow(QWidget *parent) :
    QWidget(parent),
    letterUi(new Ui::letterFormWindow)
{
    letterUi->setupUi(this);

    letterUi->spinBox->setValue(10);
}

letterFormWindow::~letterFormWindow()
{
    delete letterUi;
}

void letterFormWindow::on_buttonBox_accepted()
{
    emit sendFont(tempFont);    //向主界面传递该字体
    this->hide();
}

void letterFormWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
    tempFont.setFamily(f.family());
    ChangeFont();
}

void letterFormWindow::on_spinBox_valueChanged(int arg1)
{
    tempFont.setPointSize(arg1);
    ChangeFont();
}

void letterFormWindow:: ChangeFont(void)
{
    letterUi->label->setFont(tempFont);
}
