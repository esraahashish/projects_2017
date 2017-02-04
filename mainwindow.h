#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QLabel>
#include <QString>
#include <QStringRef>
#include <QFileDialog>
#include <QMouseEvent>
#include <QRect>
#include <QPainter>
#include <qpixmap.h>
#include <qtransform.h>
#include <QSize>
#include <QScrollBar>
#include <QRubberBand>
#include <QStack>
#include <QScrollArea>
#include <QCloseEvent>

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
    void on_action_Open_triggered();

    void on_action_Undo_triggered();

    void on_action_Redo_triggered();

    void on_action_Save_triggered();

    void on_action_Select_triggered();

    void on_action_Crop_triggered();

    void on_action_Rotate_Right_triggered();

    void on_action_Zoom_In_triggered();

    void on_action_Rotate_left_triggered();

    void on_action_Zoom_Out_triggered();

private:
    Ui::MainWindow *ui;
    QPixmap current_image;
    QPoint  general_origin ;
    QPoint  top_left_point ;
    QPixmap selected_Image;
    QSize current_size;
    int h_select;
    int w_select;
    int angle = 0;
    int prev_value_x ;
    int next_value_x;
    int prev_pagestep_x;
    int next_pagestep_x;
    int prev_value_y ;
    int next_value_y;
    int prev_pagestep_y;
    int next_pagestep_y;
    double scaleFactor=1;
    struct image_member{
      int angle;
      QPixmap original_image;
      QPixmap currnet_image;
    };
    QStack <image_member> undo_stack;
    QStack <image_member> redo_stack;
    QRubberBand *rubberBand;

    QPoint  origin ;

    bool selection_on = false ;
    bool accessed = false;
    void mousePressEvent(QMouseEvent *e);
    void closeEvent (QCloseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void rotate(int angle);
    void zoom(float factor , QPixmap image);
    void init();
    bool check();
    void save();
    void zoom_selected_area(float selection_width , float picture_width ,
                            float selection_height , float picture_height);

};

#endif // MAINWINDOW_H
