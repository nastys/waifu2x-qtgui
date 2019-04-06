/*
 * Copyright © 2019 nastys
 *
 * This file is part of waifu2x-qtgui.
 * waifu2x-qtgui is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * waifu2x-qtgui is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with waifu2x-qtgui.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QProcess>
#include <QFile>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsPixmapItem>
#include <QPluginLoader>

QString extension=".png", tmpimg="/tmp/waifu2x", tmpclipboard="/tmp/waifu2x-paste.png";
QStringList log, imagemagick_filters;
QGraphicsScene *scenel, *scener;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // not implemented: check if waifu2x-converter-cpp is installed
    ui->widget_Quality->setVisible(0);
    ui->imagemagick->setVisible(0);
    ui->statusBar->showMessage("Ready.", 5000);
    scenel=new QGraphicsScene;
    scener=new QGraphicsScene;
    ui->graphicsView->setScene(scenel);
    ui->graphicsView_2->setScene(scener);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadimg(QString file, QGraphicsScene *scene)
{
    QGraphicsPixmapItem *item=new QGraphicsPixmapItem(QPixmap::fromImage(QImage(file)));
    scene->clear();
    // not implemented: restore size
    scene->addItem(item);
}

bool MainWindow::isOutImgLoaded()
{
    // not implemented
    return 1;
}

void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

void MainWindow::on_actionOpen_triggered()
{
    QString file_open=QFileDialog::getOpenFileName(this, "Open image...");
    if (file_open.isEmpty())
        return;

    ui->lineEdit->setText(file_open);
    loadimg(file_open, scenel);
    on_doubleSpinBox_valueChanged();
}

void MainWindow::upscale()
{
    QStringList args;
    QFile::remove(tmpimg+extension);
    QProcess proc;
    if (ui->scaler->currentIndex())
    {
        args<<ui->lineEdit->text()<<"-filter"<<ui->imagemagick->currentText()<<"-resize"<<QString::number(ui->doubleSpinBox->value()*100)+"%"<<tmpimg+extension;
        // compression/quality not implemented
        proc.setProgram("convert");
    }
    else
    {
        args<<"-i"<<ui->lineEdit->text()<<"-o"<<tmpimg+extension;
        // model not implemented
        // processor not implemented
        // compression/quality not implemented
        args<<"-m"<<getMode(); // mode
        args<<"--scale-ratio"<<QString::number(ui->doubleSpinBox->value()); // scale
        args<<"--noise-level"<<getNoiseReductionLevel(); // noise reduction level
        if (getJobs()>0) args<<"-j"<<QString::number(getJobs()); // jobs
        if (getBlockSize()>0) args<<"--block-size"<<QString::number(getBlockSize()); // block size
        if (getDisableGPU()) args<<"--disable-gpu";
        if (getForceOCL()) args<<"--force-OpenCL";

        proc.setProgram("waifu2x-converter-cpp");
    }
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished();
    log<<"Arguments: "+args.join(" ")+"\n"<<proc.readAll()<<"-----------------------\n";
}

QString MainWindow::getModel()
{
    // not implemented
    return "";
}

QString MainWindow::getMode()
{
    switch (ui->mode->currentIndex())
    {
        case 2:
            return "noise-scale";
        case 1:
            return "noise";
    }
    return "scale";
}

QString MainWindow::getNoiseReductionLevel()
{
    return(QString::number(ui->noiseReduction->value()));
}

QStringList MainWindow::listProcessors()
{
    // not implemented
    return(QStringList({"Default"}));
}

int MainWindow::getProcessor()
{
    return(ui->processor->currentIndex());
}

bool MainWindow::getDisableGPU()
{
    return(ui->disableGPU->isChecked());
}

bool MainWindow::getForceOCL()
{
    return(ui->forceOCL->isChecked());
}

int MainWindow::getJobs()
{
    return(ui->jobs->value());
}

int MainWindow::getBlockSize()
{
    return(ui->blockSize->value());
}

void MainWindow::imagemagickFilters()
{
    if (imagemagick_filters.isEmpty())
    {
        QProcess imagemagick_process;
        imagemagick_process.setProgram("convert");
        imagemagick_process.setArguments({"-list","Filter"});
        imagemagick_process.start();
        imagemagick_process.waitForFinished();
        imagemagick_filters<<QString(imagemagick_process.readAllStandardOutput()).split("\n");

        ui->imagemagick->addItems(imagemagick_filters);
        ui->imagemagick->setCurrentText("Hamming");
    }
}

void MainWindow::on_pushButton_Resize_clicked()
{
    ui->frame_2->setEnabled(0);
    ui->mainToolBar->setEnabled(0);
    ui->menuBar->setEnabled(0);
    statusBar()->showMessage("Resizing...");

    if (!(ui->doubleSpinBox->value() > 0.0))
    {
        statusBar()->showMessage("Invalid scale ratio.");
        return;
    }

    upscale();
    if (QFile::exists(tmpimg+extension))
        loadimg(tmpimg+extension, scener);
    else
        QMessageBox::critical(this, "Log", log.join("\n"));

    ui->frame_2->setEnabled(1);
    ui->mainToolBar->setEnabled(1);
    ui->menuBar->setEnabled(1);
    statusBar()->showMessage("Done.", 5000);
}

void MainWindow::on_doubleSpinBox_valueChanged()
{
    int newW=ui->doubleSpinBox->value()*(QImage(ui->lineEdit->text()).width());
    int newH=ui->doubleSpinBox->value()*(QImage(ui->lineEdit->text()).height());
    ui->res->setText(QString::number(newW)+"x"+QString::number(newH));
}

void MainWindow::on_actionUpscale_triggered()
{
    on_pushButton_Resize_clicked();
}

void MainWindow::on_mode_currentIndexChanged(int index)
{
    switch (index)
    {
        case 2:
            ui->widget_Denoise->setVisible(1);
            ui->frame_Size->setVisible(1);
            break;
        case 1:
            ui->widget_Denoise->setVisible(1);
            ui->frame_Size->setVisible(0);
            break;
        default:
            ui->widget_Denoise->setVisible(0);
            ui->frame_Size->setVisible(1);
    }
}

void MainWindow::on_action_View_triggered()
{
    QMessageBox::information(this, "Log", log.join("\n"));
}

void MainWindow::on_actionClear_triggered()
{
    log.clear();
}

void MainWindow::on_action_Export_triggered()
{
    // not implemented
}

void MainWindow::on_actionPaste_input_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

        if (mimeData->hasImage()) {
            QPixmap newpixmap=qvariant_cast<QPixmap>(mimeData->imageData());
            QFile outfile(tmpclipboard);
            outfile.remove();
            outfile.open(QIODevice::WriteOnly);
            newpixmap.save(&outfile, "PNG");
            outfile.close();

            ui->lineEdit->setText(tmpclipboard);
            loadimg(tmpclipboard, scenel);
        }
        else
        {
            QMessageBox::warning(this, "Warning", "No image data in the clipboard, nothing pasted.");
            return;
        }

    ui->statusBar->showMessage("Pasted from the clipboard.", 5000);
}

void MainWindow::on_actionCopy_output_triggered()
{
    QImage newimage(tmpimg+extension);
    if (!(isOutImgLoaded()) || newimage.isNull())
    {
        QMessageBox::warning(this, "Warning", "No output image, nothing copied.");
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    QMimeData *data = new QMimeData;
    data->setImageData(QVariant(newimage));
    clipboard->setMimeData(data);

    ui->statusBar->showMessage("Copied to the clipboard. WARNING: Clipboard contents may be deleted when you close the application.", 5000);
}

void MainWindow::on_pushButton_clicked()
{
    on_actionPaste_input_triggered();
}

void MainWindow::on_pushButton_2_clicked()
{
    on_actionCopy_output_triggered();
}

void MainWindow::on_pushButton_4_clicked()
{
    on_actionOpen_triggered();
}

void MainWindow::on_actionSave_as_triggered()
{
    // not implemented
}

void MainWindow::on_pushButton_3_clicked()
{
    on_actionSave_as_triggered();
}

void MainWindow::on_format_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0: // PNG
            ui->widget_Quality->setVisible(0);
            ui->widget_Compression->setVisible(1);
            extension=".png";
            break;
        case 1: // WebP
            ui->widget_Compression->setVisible(0);
            ui->widget_Quality->setVisible(1);
            extension=".webp";
            break;
        case 2: // JPG
            ui->widget_Compression->setVisible(0);
            ui->widget_Quality->setVisible(1);
            extension=".jpg";
            break;
        case 3: // BMP
            ui->widget_Quality->setVisible(0);
            ui->widget_Compression->setVisible(0);
            extension=".bmp";
            break;
        case 4: // TIFF
            ui->widget_Quality->setVisible(0);
            ui->widget_Compression->setVisible(0);
            extension=".tiff";
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "waifu2x-qtgui Copyright © 2019 nastys\n\nwaifu2x-qtgui is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nwaifu2x-qtgui is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with waifu2x-qtgui. If not, see <http://www.gnu.org/licenses/>.");
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_scaler_activated(int index)
{
    switch (index)
    {
        case 1:
            // not implemented: check if imagemagick is installed
            imagemagickFilters();
            ui->model->setVisible(0);
            ui->mode->setVisible(0);
            ui->widget_Denoise->setVisible(0);
            ui->imagemagick->setVisible(1);
            ui->toolBox->removeItem(2);
            break;
        case 0:
            // not implemented: check if waifu2x-converter-cpp is installed
            ui->imagemagick->setVisible(0);
            ui->model->setVisible(1);
            ui->mode->setVisible(1);
            ui->widget_Denoise->setVisible(1);
            ui->toolBox->addItem(ui->page_3,"Processor");
    }
}
