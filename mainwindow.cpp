#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLCDNumber>
#include <QProgressBar>

#define DEFAULT_CW_SPEED 22

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    printf("This is the main window constructor\n");
    fflush(stdout);
    ui->setupUi(this);

    /* We're alive, do any setup required */
    this->speed = DEFAULT_CW_SPEED;
    int i = this->ui->lcdSpeed->digitCount();
    printf("digit count = %d\n", i);
    fflush(stdout);
    this->ui->lcdSpeed->display(speed);

    /* Setup initial progress bar */
    this->ui->speedProgressBar->setRange(5,65);
    this->ui->speedProgressBar->setValue(speed);

    /* Initialize the spinbox for initial speed */
    this->ui->spinBoxSpeed->setValue(speed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_message_1_clicked()
{
    // This handler is called when message 1 button is clicked
    printf("Message 1 button clicked\n");
    fflush(stdout);
}

void MainWindow::on_message_2_clicked()
{
    // This handler is called when message 2 button is clicked
    printf("Message 2 button clicked\n");
    fflush(stdout);
}

void MainWindow::on_message_3_clicked()
{
    // This handler is called when message 3 button is clicked
    printf("Message 3 button clicked\n");
    fflush(stdout);
}

void MainWindow::on_message_4_clicked()
{
    // This handler is called when message 4 button is clicked
    printf("Message 4 button clicked\n");
    fflush(stdout);
}


void MainWindow::on_speedProgressBar_valueChanged(int value)
{
    /* TODO: Send speed value to WinKeyer */
    printf("progress bar value changed %d\n", value);
    fflush(stdout);
}

void MainWindow::on_spinBoxSpeed_valueChanged(int arg1)
{
    this->ui->speedProgressBar->setValue(arg1);
}
