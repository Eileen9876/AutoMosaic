#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "facedetection.h"
#include "ImageProc.hpp"
#include <string>
#include <vector>
#include <QFileDialog>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>
#include <iostream>

std::string ToStr(QString qstr);

QString ToQStr(int num);

QString ToQStr(std::string str);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->FileName_Label->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_LoadImage_Button_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "請選擇要使用的圖片",
        "/Desktop/",
        "Images (*.png *.xpm *.jpg)"
        );

    if (fileName.isEmpty()) return;

    ui->FileName_Label->setText(fileName);

    QPixmap pixmap(fileName);

    ui->Image_Label->setPixmap(pixmap.scaled(ui->Image_Label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::on_Mosaic_Button_clicked()
{
    QString fileName = ui->FileName_Label->text();

    if (fileName == "") {
        QMessageBox::information(this, "", "尚未載入圖片");
        return;
    }

    imgproc::Data data(imgproc::DataType::IMAGE, ToStr(fileName));

    imgproc::Client cli("127.0.0.1", 8000);

    imgproc::Result res = cli.TransmitData(data);

    if (res.error != "") {
        QMessageBox::critical(this, "錯誤", ToQStr(res.error));
        return;
    }

    QImage image(fileName);
    for (std::vector<int> subList : res.image_data) {

        //subList(y1, x2, y2, x1)
        //QRect(int left, int top, int width, int height)
        QRect rect(subList[3], subList[0], subList[1]-subList[3], subList[2]-subList[0]);

        image = MosaicImage(image, rect);
    }

    QPixmap pixmap = QPixmap::fromImage(image);

    ui->Image2_Label->setPixmap(pixmap.scaled(ui->Image2_Label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::on_StoreImage_Button_clicked()
{
    QPixmap pixmap = ui->Image2_Label->pixmap();

    if (pixmap.isNull()) {
        QMessageBox::information(this, "", "尚未處理圖片");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "儲存檔案",
        "/Desktop/",
        "Images (*.png *.xpm *.jpg)"
    );

    if (fileName.isEmpty()) return;

    pixmap.save(fileName);
}

std::string ToStr(QString qstr){
    return qstr.toStdString();
}

QString ToQStr(int num){
    return QString::number(num);
}

QString ToQStr(std::string str){
    return QString::fromStdString(str);
}
