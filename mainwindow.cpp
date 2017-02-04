#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <stdlib.h>
#include <QGraphicsView>
#include <QMessageBox>
#include <QStringRef>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 *this function called when open image button is pressed
 * file Dialogue openend to open file allows folder or files of certain extension(png ,bmp,jpg)
 * a message appears for saving the current image if its state was changed
 */

void MainWindow::on_action_Open_triggered()
{
    if(accessed)
    {

        save();

    }
    init();
    QString fn = QFileDialog::getOpenFileName(this,tr("Open File"),"",tr("JPEG (*.jpg);;PNG (*.png);;BMP(*.bmp)" ));

    if (!fn.isEmpty()){

        QImageReader reader(fn);
        reader.setAutoTransform(true);
        QImage image = reader.read();
        image_member img;
        img.currnet_image = QPixmap::fromImage(image) ;
        img.original_image =  img.currnet_image;
        current_image = img.currnet_image;
        img.angle = 0;
        current_size = current_image.size();
        if (!image.isNull()){
            undo_stack.push(img);
            ui->picture->setPixmap(current_image);
            ui->picture->setAlignment(Qt::AlignTop);
        }
    }
}




/*
 * this function called upon undo
 * if no more undo can be done , the image stays as it is
 */
void MainWindow::on_action_Undo_triggered()
{
    if(!check())return;
    init();
    if(!undo_stack.isEmpty())
    {

        if(undo_stack.size() == 1)
        {
            ui->picture->setPixmap(undo_stack.top().original_image);
        }else
        {
            redo_stack.push(undo_stack.pop());
            current_image = undo_stack.top().currnet_image;
            ui->picture->setPixmap(current_image);
        }
    }
}



/*
 * this function called upon redo
 * if no more redo can be done a message
 * appear to tell the user that no more
 * redo can be done
 */

void MainWindow::on_action_Redo_triggered()
{
    if(!check())return;
    init();
    if(!redo_stack.isEmpty())
    {
        undo_stack.push(redo_stack.pop());
        current_image = undo_stack.top().currnet_image;
        ui->picture->setPixmap(current_image);
    }else
    {
        QMessageBox::warning(this,"Invalid redo","No more Redo can be done !");
    }

}

/*
 * this function called when clicking save button
 * open file chooser
 * file extension can be chosen
 */
void MainWindow::on_action_Save_triggered()
{
    if(!check())return;
    init();
    QString fn = QFileDialog::getSaveFileName(

                this,

                tr("Save File"),

                "",

                tr("JPEG (*.jpg);;PNG (*.png) ;;BMP (*.bmp)" )

                );

    QFile file(fn);

    QString temp = fn.right(3);
    file.open(QIODevice::WriteOnly);
    if(!QString::compare(temp, "bmp", Qt::CaseInsensitive))
        current_image.save(&file,"BMP");
    else if(!QString::compare(temp, "JPG", Qt::CaseInsensitive))
        current_image.save(&file,"JPG");
    else if (!QString::compare(temp, "png", Qt::CaseInsensitive))
        current_image.save(&file,"PNG");
    else{
        current_image.save(&file,"JPG");
    }
    if(fn != NULL){
        accessed = false;
        redo_stack.clear();
        undo_stack.clear();
    }
}

/*
 * this function called when clicking select button
 * to select an area for crop or zoom
 */
void MainWindow::on_action_Select_triggered()
{
    if(!check())return;
    selection_on = true ;
}


/*
 * this function called when mouse pressed
 */

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(selection_on){

        origin = event->pos();
        general_origin = event->globalPos();
        if (!rubberBand)
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        rubberBand->setGeometry(QRect(origin, QSize()));
        rubberBand->show();
    }
}

/*
 * function this calling will moving mouse with press
 * keeptrack of the topleft point and set height & width of the rubberband
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    top_left_point = ui->picture->mapFromGlobal(QPoint(std::min(general_origin.x(),event->globalPos().x()),
                                                      std::min(general_origin.y(),event->globalPos().y())));
    h_select = std::abs(general_origin.y()-event->globalPos().y());
    w_select = std::abs(general_origin.x()-event->globalPos().x());

    if(selection_on)
        rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
}

/*
 * function this calling will moving mouse is Released
 * get the rubberband area and selected image
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(selection_on){
        QPoint a = mapToGlobal(origin);
        QPoint b = event->globalPos();
        a = ui->picture->mapFromGlobal(a);
        b = ui->picture->mapFromGlobal(b);
        QPixmap OriginalPix(*ui->picture->pixmap());
        QRect myRect(a ,b);
        selected_Image = OriginalPix.copy(myRect);
    }
}

/*
 *this function is called upon croping selected area.
 */
void MainWindow::on_action_Crop_triggered()
{
    if(!check())return;
    init();
    accessed = true;
    redo_stack.clear();
    if(selection_on && !selected_Image.isNull()){
        rubberBand->hide();
        ui->picture->setPixmap(selected_Image);
        ui->picture->repaint();
        current_image = selected_Image;
        image_member img;
        img.original_image = current_image;
        img.currnet_image = current_image;
        img.angle = 0;
        undo_stack.push(img);
        selection_on = false;
    }
}

/*
 * this function rotate the image to reight by userInput degree
 * view the rotated image
 */
void MainWindow::on_action_Rotate_Right_triggered()
{
    if(!check())return;
    init();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter Angle"),
                                         tr("Angle:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty()){
        if(!undo_stack.empty())
        {
            angle = undo_stack.top().angle;
        }
        else{
            angle = 0;
        }
        angle += text.toInt();
        rotate(angle);
    }
}



/*
 * this function rotate the image to left by userInput degree
 * view the rotated image
 */
void MainWindow::on_action_Rotate_left_triggered()
{
    if(!check())return;
    init();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter Angle"),
                                         tr("Angle:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty()){
        if(!undo_stack.empty())
        {
            angle = undo_stack.top().angle;
        }
        else{
            angle = 0;
        }
        angle -= text.toInt();
        rotate(angle);
    }
}

/*this function is called to do the
 * rotation of the image with the
 * input angle
 */
void  MainWindow::rotate(int angle)
{
    init();
    accessed = true;
    redo_stack.clear();
    image_member img = undo_stack.top();
    QPixmap pixmap(img.original_image);
    QMatrix rm;
    rm.rotate(angle);
    pixmap = pixmap.transformed(rm);
    img.currnet_image = pixmap;
    img.angle = angle;
    ui->picture->setPixmap(pixmap);
    undo_stack.push(img);
    current_image = pixmap;
}

/*
 * this function called when zoom in button clicked
 * zoom in is done for the whole image of the selected
 * part only. image is rescaled with the new zoom factor
 */
void MainWindow::on_action_Zoom_In_triggered()
{
    //if scaleFactor increase certain value then return to avoid memory full error
    if(scaleFactor >= 3.5 || !check()){
        if(rubberBand != NULL )
            rubberBand->hide();
        return ;
    }

    if(!selection_on ){
        zoom(1.25,current_image);
    }else if (!selected_Image.isNull()){
        float picture_width = ui->scrollArea->width();
        float picture_height =ui->scrollArea->height();
        float selection_width = w_select;
        float selection_height =h_select;
        zoom_selected_area(picture_width , selection_width , picture_height , selection_height );
   }
}

/*
 * this function called when zoom out button clicked
 * zoom out is done for the whole image of the selected
 * part only. image is rescaled with the new zoom factor
 */
void MainWindow::on_action_Zoom_Out_triggered()
{
    if(scaleFactor <= 0.1 || !check() ){
        if(rubberBand != NULL )
            rubberBand->hide();
        return ;
    }
    if(!selection_on ){
        zoom(0.8,current_image);
    }else if (!selected_Image.isNull()){
        float picture_width = ui->scrollArea->width();
        float picture_height =ui->scrollArea->height();
        float selection_width = w_select;
        float selection_height =h_select;
        zoom_selected_area(selection_width , picture_width
                           ,  selection_height ,  picture_height);
    }
}



/*
 * this function called internaly to process
 * the general zoom
 */
void MainWindow::zoom(float factor , QPixmap image){

    scaleFactor *= factor;
    QPixmap pixmap = image;
    int w = (pixmap.width())*scaleFactor;
    int h = (pixmap.height())*scaleFactor;
    ui->picture->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio , Qt::SmoothTransformation));
    adjustScrollBar(ui->scrollArea->horizontalScrollBar(), scaleFactor);
    adjustScrollBar(ui->scrollArea->verticalScrollBar(), scaleFactor);
}

/*
 *this function is called to zoom the
 * selected area with the calculated scale factor
 * with respect to the current image
 * and the selected rubberband
 */
void MainWindow::zoom_selected_area(float selection_width , float picture_width
                                    , float selection_height , float picture_height)
{
    rubberBand->hide();
    selection_on = false;
    float min ;
    (selection_width / picture_width < selection_height/picture_height)?
                min = selection_width / picture_width : min = selection_height/picture_height;
    scaleFactor*= min;

    /*****scalling the image*****/
    QPixmap pixmap(current_image.scaled(current_image.width()*scaleFactor ,
                                        current_image.height()*scaleFactor, Qt::IgnoreAspectRatio , Qt::SmoothTransformation));
    ui->picture->resize(pixmap.size());
    ui->picture->setPixmap(pixmap);

    top_left_point*=min;
    /*enable scroll bars updates*/
    ui->scrollArea->setUpdatesEnabled(true);
    ui->scrollArea->horizontalScrollBar()->setUpdatesEnabled(true);
    ui->scrollArea->verticalScrollBar()->setUpdatesEnabled(true);
    /*set scroll bar's slider position and range*/
    ui->scrollArea->horizontalScrollBar()->setRange(0,ui->picture->width());
    ui->scrollArea->horizontalScrollBar()->setValue(top_left_point.x());
    ui->scrollArea->verticalScrollBar()->setRange(0,ui->picture->height());
    ui->scrollArea->verticalScrollBar()->setValue(top_left_point.y());
}


 /*this method is called to
  * adjust the scrollbar slider position
  */
void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{

    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}


/*
 * set the scale factor to the initial state
 */
void MainWindow::init()
{
    scaleFactor = 1;
}

/*
 * check if there is image loaded or not
 */
bool MainWindow::check()
{
    return !current_image.isNull();
}

/*
 * called when clicking the close button
 * a dialouge is showen to save the current
 * image before closing or not
 */
void MainWindow::closeEvent (QCloseEvent *event)
{
    if(accessed)
        save();
}

/*
 * a dialouge is showen to ask the user
 * tosave the current
 * image before closing or not.
 */
void MainWindow::save()
{
    accessed = false;
    QMessageBox msgBox;
    msgBox.setText("The image has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
        on_action_Save_triggered();
        redo_stack.clear();
        undo_stack.clear();
        break;
    case QMessageBox::Discard:
        redo_stack.clear();
        undo_stack.clear();
        break;
    default:
        return;
    }
}

