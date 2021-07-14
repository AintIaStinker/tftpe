// Need to fix Upload button for any Errors
// Put in a catch to prevent blank file from trying to be sent


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QToolBar>

#include "tftp.h"
#include "tftpmanager.h"
#include "config.h"

#include <QThread>
#include <QQueue>
#include <QFileInfo>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnUpload->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnSelectFiles_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                "Select one or more files to open",
                QDir::homePath(),
                "All Files (*.*)");

    if(files.length() > 0)
    {
        ui->tblFiles->setRowCount(files.length());

        int count = 0;
        for(const QString &i : files){
            QTableWidgetItem *filePath = new QTableWidgetItem(i);
            QFileInfo fi(i);
            QTableWidgetItem *fileName = new QTableWidgetItem(fi.fileName());
            ui->tblFiles->setItem(count, 0, filePath);
            ui->tblFiles->setItem(count, 1, fileName);
            count++;
            filePath->setFlags(filePath->flags() & ~Qt::ItemIsEditable);
        }
         ui->btnUpload->setEnabled(true);
    }
}


void MainWindow::on_btnCancel_clicked()
{
    if(transferInProgress)
    {
        emit stop();
        transferInProgress = false;
        ui->btnUpload->setEnabled(true);
    }
    else
    {
        return;
    }
}

bool MainWindow::queueFileList()
{
    fileList.clear();

    for(int i = 0; i < ui->tblFiles->rowCount(); i++)
    {
        for(int j = 0; j < ui->tblFiles->columnCount(); j++)
        {
            if(ui->tblFiles->item(i, j) != 0)   // Fix me
            {
                fileList.resize(ui->tblFiles->rowCount());
                fileList[i].append(ui->tblFiles->item(i, j)->text());
            }

        }
    }

    if(fileList.length() >= 1)
        return true;
    else
        return false;
}

void MainWindow::on_btnUpload_clicked()
{
    if(!queueFileList()) return;

    else
    {
        queueFileList();    //Generate list of files to process
        startProcess();     //Begin new thread & transfer file(s)
    }
    ui->btnUpload->setDisabled(true);
}

void MainWindow::startProcess()
{
    thread = new QThread;
    tftp = new class tftp();

    tftp->setIpAddress(ui->leHost->text());
    tftp->setIpPort(ui->lePort->text().toUInt());
    tftp->setFileToSend(fileList[0][1]);
    tftp->setBlockSize(ui->cmbBlockSize->currentText().toInt());
    tftp->setFilePath(fileList[0][0]);

    ui->progressBar->setMinimum(0);
    ui->progressBar->reset();


    connect(thread, &QThread::started, tftp, &tftp::start, Qt::QueuedConnection);
    connect(this, &MainWindow::quitThread, thread, &QThread::quit, Qt::QueuedConnection);
    connect(tftp, &tftp::completed, this, &MainWindow::completed, Qt::QueuedConnection);
    connect(this, &MainWindow::stop, tftp, &tftp::stop, Qt::QueuedConnection);
    connect(tftp, &tftp::setEnabled, ui->btnUpload, &QPushButton::setEnabled, Qt::QueuedConnection);
    connect(tftp, &tftp::setMaximum, ui->progressBar, &QProgressBar::setMaximum, Qt::QueuedConnection);
    connect(tftp, &tftp::setValue, ui->progressBar, &QProgressBar::setValue, Qt::QueuedConnection);
    connect(this, &MainWindow::reset, ui->progressBar, &QProgressBar::reset, Qt::QueuedConnection);
    connect(tftp, &tftp::sendStatus, ui->txtProgress, &QPlainTextEdit::appendPlainText, Qt::QueuedConnection);
    connect(tftp, &tftp::sendStatus, ui->lblStatus, &QLabel::setText, Qt::QueuedConnection);

    tftp->moveToThread(thread);

    thread->start();
    transferInProgress = true;
}

void MainWindow::save()
{
    QFile file("something.txt");
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    ds << ui->leHost->text();
    ds << ui->lePort->text();
    ds << ui->cmbBlockSize->currentData();


}

void MainWindow::load()
{
    QFile file("something.txt");
    file.open(QIODevice::ReadOnly);
    QDataStream ds(&file);
    QString host;
    QString port;



    ds >> host >> port;
    ui->leHost->setText(host);
    ui->lePort->setText(port);
    qDebug() << "HOst " << host;
}

void MainWindow::completed()
{
    if(!transferInProgress)
    {
        emit tftp->sendStatus("Transaction cancelled...");
        delete tftp;
        thread->quit();
        thread->wait();
        return;
    }
    if (fileList.length() >= 2)
    {
        fileList.removeFirst();
        emit tftp->sendStatus("Completed file " + tftp->getFileToSend() + "...");
        delete tftp;
        emit quitThread();
        startProcess();
        return;
    }
    else
    {
        emit tftp->sendStatus("Completed file " + tftp->getFileToSend() + "...");
        delete tftp;
        emit quitThread();
    }
    ui->btnUpload->setEnabled(true);
}


void MainWindow::on_btnDeleteRow_clicked()
{
    ui->tblFiles->removeRow(ui->tblFiles->currentRow());
}

void MainWindow::on_btnCredits_clicked()
{
    credits *c = new credits(0);
    c->exec();
}


void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}


void MainWindow::on_actionSave_Config_triggered()
{
    save();
}


void MainWindow::on_actionLoad_Config_triggered()
{
    load();
}

