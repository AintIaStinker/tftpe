#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include "tftp.h"
#include <QProgressBar>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSelectFiles_clicked();

    void on_btnCancel_clicked();

    void on_btnUpload_clicked();

    void completed();

    void on_btnDeleteRow_clicked();

signals:
    void quitThread();
    void start();
    void stop();
    void reset();

private:
    Ui::MainWindow *ui;
    QVector<QVector<QString>> fileList{};
    QThread *thread;
    tftp *tftp;

    bool transferInProgress = false;
    bool queueFileList();
    void startProcess();
    QProgressBar *progressBar;



};
#endif // MAINWINDOW_H
