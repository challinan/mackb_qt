#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLCDNumber>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_message_1_clicked();

    void on_message_2_clicked();

    void on_message_3_clicked();

    void on_message_4_clicked();

    void on_speedProgressBar_valueChanged(int value);

    void on_spinBoxSpeed_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    int speed;
};

#endif // MAINWINDOW_H
