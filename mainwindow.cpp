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
#include <QPluginLoader>
#include <QScroller>
#include <QDragEnterEvent>
#include <QTextStream>

QString extension=".png", tmpimg="/tmp/waifu2x-qtgui", tmpclipboard="/tmp/waifu2x-qtgui-paste.png";
QStringList log, imagemagick_filters;
double zoom_l=1, zoom_r=1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // not implemented: check if waifu2x-converter-cpp is installed
    ui->widget_Quality->setVisible(0);
    ui->imagemagick->setVisible(0);
    ui->statusBar->showMessage("Ready.", 5000);
    QScrollerProperties scroller_properties = QScroller::scroller(ui->scrollArea_l)->scrollerProperties();
    QVariant no_overshoot = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff);
    scroller_properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, no_overshoot);
    scroller_properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, no_overshoot);
    QScroller::scroller(ui->scrollArea_l)->setScrollerProperties(scroller_properties);
    QScroller::grabGesture(ui->scrollArea_l, QScroller::LeftMouseButtonGesture);
    QScroller::scroller(ui->scrollArea_r)->setScrollerProperties(scroller_properties);
    QScroller::grabGesture(ui->scrollArea_r, QScroller::LeftMouseButtonGesture);
    listProcessors();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadimg(QString file, QLabel *frame, double *zoom, QLabel *zoomlevel, QPushButton *zoomin, QPushButton *zoomout)
{
    frame->setScaledContents(0);
    frame->setPixmap(QPixmap::fromImage(QImage(file)));
    frame->setFixedSize(frame->pixmap()->size());
    *zoom=1.0;
    zoomlevel->setText("1x");
    zoomin->setEnabled(1);
    zoomout->setEnabled(1);
    on_doubleSpinBox_valueChanged();
    ui->actionUpscale->setEnabled(1);
    ui->pushButton_Resize->setEnabled(1);
}

void MainWindow::unloadimg()
{
    ui->pic_r->clear();
    ui->pic_r->setScaledContents(0);
    ui->pic_r->setFixedSize(0, 0);
    zoom_r=1.0;
    ui->zoom_r->setText("1x");
    ui->r_zoomin->setEnabled(1);
    ui->r_zoomout->setEnabled(1);
}

bool MainWindow::isOutImgLoaded()
{
    return !(ui->pic_r->pixmap()==0);
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
    unloadimg();
    loadimg(file_open, ui->pic_l, &zoom_l, ui->zoom_l, ui->l_zoomin, ui->l_zoomout);
}

void MainWindow::upscale()
{
    QStringList args;
    QFile::remove(tmpimg+extension);
    QProcess proc;
    if (ui->scaler->currentIndex())
    {
        args<<ui->lineEdit->text()<<"-filter"<<ui->imagemagick->currentText()<<"-resize"<<QString::number(ui->doubleSpinBox->value()*100)+"%"<<tmpimg+extension;
        if (ui->widget_Compression->isVisible())
            args<<"-define png:compression-level="+QString::number(ui->compression->value()); // PNG
        if (ui->widget_Quality->isVisible())
            args<<"-quality"<<QString::number(ui->quality->value());
        proc.setProgram("convert");
    }
    else
    {
        args<<"-i"<<ui->lineEdit->text()<<"-o"<<tmpimg+extension;
        // model not implemented
        args<<"-p"<<ui->processor->currentText().at(0);
        if (ui->widget_Compression->isVisible())
            args<<"-c"<<QString::number(ui->compression->value());
        if (ui->widget_Quality->isVisible())
            args<<"-q"<<QString::number(ui->quality->value());
        args<<"-m"<<getMode(); // mode
        args<<"--scale-ratio"<<QString::number(ui->doubleSpinBox->value()); // scale
        args<<"--noise-level"<<getNoiseReductionLevel(); // noise reduction level
        if (getJobs()>0) args<<"-j"<<QString::number(getJobs()); // jobs
        if (getBlockSize()>0) args<<"--block-size"<<QString::number(getBlockSize()); // block size

        proc.setProgram("waifu2x-converter-cpp");
    }
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished(-1);
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

void MainWindow::listProcessors()
{
    QProcess process;
    QStringList processors;
    process.setProgram("waifu2x-converter-cpp");
    process.setArguments({"-l"});
    process.start();
    process.waitForFinished(10000);
    processors<<QString(process.readAllStandardOutput()).remove(0, 3).split("\n   ");
    ui->processor->addItems(processors);
}

int MainWindow::getProcessor()
{
    return(ui->processor->currentIndex());
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
    ui->pushButton->setEnabled(0);
    ui->pushButton_2->setEnabled(0);
    ui->pushButton_3->setEnabled(0);
    ui->pushButton_4->setEnabled(0);
    ui->lineEdit->setEnabled(0);
    ui->l_zoomin->setEnabled(0);
    ui->l_zoomout->setEnabled(0);
    ui->r_zoomin->setEnabled(0);
    ui->r_zoomout->setEnabled(0);
    MainWindow::setAcceptDrops(0);
    statusBar()->showMessage("Resizing...");
    MainWindow::repaint();

    if (!(ui->doubleSpinBox->value() > 0.0))
    {
        statusBar()->showMessage("Invalid scale ratio.");
        return;
    }

    upscale();
    if (QFile::exists(tmpimg+extension))
        loadimg(tmpimg+extension, ui->pic_r, &zoom_r, ui->zoom_r, ui->r_zoomin, ui->r_zoomout);
    else
        QMessageBox::critical(this, "Log", log.join("\n"));

    ui->frame_2->setEnabled(1);
    ui->mainToolBar->setEnabled(1);
    ui->menuBar->setEnabled(1);
    ui->pushButton->setEnabled(1);
    ui->pushButton_2->setEnabled(1);
    ui->pushButton_3->setEnabled(1);
    ui->pushButton_4->setEnabled(1);
    ui->lineEdit->setEnabled(1);
    ui->l_zoomin->setEnabled(1);
    ui->l_zoomout->setEnabled(1);
    ui->r_zoomin->setEnabled(1);
    ui->r_zoomout->setEnabled(1);
    MainWindow::setAcceptDrops(1);
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
    QString file_save=QFileDialog::getSaveFileName(this, "Save log as...", "", "*.txt");
    if (file_save.isEmpty())
        return;

    if(file_save.endsWith(".txt")==false)
        file_save.append(".txt");

    QFile logfile(file_save);
    logfile.remove();
    if (logfile.open(QIODevice::ReadWrite)==0)
        QMessageBox::warning(this, "", "Could not save the log.");
    else
    {
        QTextStream stream(&logfile);
        stream<<log.join("\n");
        ui->statusBar->showMessage("Log saved.", 5000);
    }

    logfile.close();
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
            loadimg(tmpclipboard, ui->pic_l, &zoom_l, ui->zoom_l, ui->l_zoomin, ui->l_zoomout);
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
    if (!isOutImgLoaded() && QFile::exists(tmpimg+extension))
    {
        int savelast=QMessageBox::question(this, "", "No image loaded.\nWould you like to save the last temporary file in the currently selected format? ("+extension+")");
        if (savelast==QMessageBox::No)
            return;
    }
    else if (!isOutImgLoaded() || QFile::exists(tmpimg+extension)==false)
    {
        QMessageBox::warning(this, "", "Nothing to save.");
        return;
    }
    else if (QFile::exists(tmpimg+extension)==false)
    {
        QMessageBox::warning(this, "", "The temporary file is missing, cannot save.");
        return;
    }

    QString file_save=QFileDialog::getSaveFileName(this, "Save image...", "", "*"+extension);
    if (file_save.isEmpty())
        return;

    if(file_save.endsWith(extension)==false)
        file_save.append(extension);

    if (QFile::copy(tmpimg+extension, file_save))
        ui->statusBar->showMessage("File saved.", 5000);
    else
        QMessageBox::warning(this, "", "Could not save the image.");
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString filename=event->mimeData()->urls().at(0).toLocalFile();
    ui->lineEdit->setText(filename);
    unloadimg();
    loadimg(filename, ui->pic_l, &zoom_l, ui->zoom_l, ui->l_zoomin, ui->l_zoomout);
}

void MainWindow::zoomin(double *zoom, QLabel *pic, QLabel *zoomlabel, QPushButton *zoomin, QPushButton *zoomout)
{
    if (*zoom==4.0 || pic->pixmap()==0)
        return;

    if (*zoom>=1.0)
        *zoom=*zoom+1;
    else
        *zoom=*zoom+0.25;

    if (*zoom==1.0)
        pic->setScaledContents(0);
    else
        pic->setScaledContents(1);

    pic->setFixedSize(pic->pixmap()->size()*(*zoom));
    zoomlabel->setText(QString::number(*zoom)+"x");

    if (*zoom==4.0)
        zoomin->setEnabled(0);

    zoomout->setEnabled(1);
}

void MainWindow::zoomout(double *zoom, QLabel *pic, QLabel *zoomlabel, QPushButton *zoomin, QPushButton *zoomout)
{
    if (*zoom==0.25 || pic->pixmap()==0)
        return;

    if (*zoom>=2.0)
        *zoom=*zoom-1;
    else
        *zoom=*zoom-0.25;

    if (*zoom==1.0)
        pic->setScaledContents(0);
    else
        pic->setScaledContents(1);

    pic->setFixedSize(pic->pixmap()->size()*(*zoom));
    zoomlabel->setText(QString::number(*zoom)+"x");

    if (*zoom==0.25)
        zoomout->setEnabled(0);

    zoomin->setEnabled(1);
}

void MainWindow::on_l_zoomin_clicked()
{
    zoomin(&zoom_l, ui->pic_l, ui->zoom_l, ui->l_zoomin, ui->l_zoomout);
}

void MainWindow::on_l_zoomout_clicked()
{
    zoomout(&zoom_l, ui->pic_l, ui->zoom_l, ui->l_zoomin, ui->l_zoomout);
}

void MainWindow::on_r_zoomin_clicked()
{
    zoomin(&zoom_r, ui->pic_r, ui->zoom_r, ui->r_zoomin, ui->r_zoomout);
}

void MainWindow::on_r_zoomout_clicked()
{
    zoomout(&zoom_r, ui->pic_r, ui->zoom_r, ui->r_zoomin, ui->r_zoomout);
}
