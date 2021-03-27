#ifndef LETTERFORMWINDOW_H
#define LETTERFORMWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QDebug>

namespace Ui {
class letterFormWindow;
}

class letterFormWindow : public QWidget
{
    Q_OBJECT

public:
    explicit letterFormWindow(QWidget *parent = 0);
    ~letterFormWindow();

private slots:

    void on_buttonBox_accepted();

    void on_fontComboBox_currentFontChanged(const QFont &f);

    void on_spinBox_valueChanged(int arg1);

    void ChangeFont(void);

signals:
    void sendFont(QFont);   //用来传递数据的信号

private:
    Ui::letterFormWindow *letterUi;

    QFont tempFont; //缓存字体

};

#endif // LETTERFORMWINDOW_H
