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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QGraphicsScene>
#include <QDragEnterEvent>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void loadimg(QString file, QLabel *frame, double *zoom, QLabel *zoomlevel, QPushButton *zoomin, QPushButton *zoomout);

    void unloadimg();

    void parseCommandLine();

    bool isOutImgLoaded();

    void upscale();

    QString getModel();

    QString getMode();

    QString getNoiseReductionLevel();

    void listProcessors();

    int getProcessor();

    bool getDisableGPU();

    bool getForceOCL();

    int getJobs();

    int getBlockSize();

    void imagemagickFilters();

    void zoomin(double *zoom, QLabel *pic, QLabel *zoomlabel, QPushButton *zoomin, QPushButton *zoomout);

    void zoomout(double *zoom, QLabel *pic, QLabel *zoomlabel, QPushButton *zoomin, QPushButton *zoomout);

protected:
    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

private slots:
    void on_actionExit_triggered() __attribute__ ((noreturn));

    void on_actionOpen_triggered();

    void on_pushButton_Resize_clicked();

    void on_doubleSpinBox_valueChanged();

    void on_actionUpscale_triggered();

    void on_mode_currentIndexChanged(int index);

    void on_action_View_triggered();

    void on_actionClear_triggered();

    void on_action_Export_triggered();

    void on_actionPaste_input_triggered();

    void on_actionCopy_output_triggered();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_actionSave_as_triggered();

    void on_pushButton_3_clicked();

    void on_format_currentIndexChanged(int index);

    void on_actionAbout_triggered();

    void on_actionAbout_Qt_triggered();

    void on_scaler_activated(int index);

    void on_l_zoomin_clicked();

    void on_l_zoomout_clicked();

    void on_r_zoomin_clicked();

    void on_r_zoomout_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
