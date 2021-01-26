#include <QtWidgets>
#if defined(QT_PRINTSUPPORT_LIB)       // Checking if printsupport exist in the project in(.pro)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrinter>
#include <QPrintDialog>
#endif
#endif

#include "canvas.h"

Canvas::Canvas(QWidget *parent) : QWidget(parent)   // this is our constructor
{
    // Roots the widget to the top left even if resized
    setAttribute(Qt::WA_StaticContents);

    // Set defaults for the monitored variables
    modified = false;
    scribbling = false;
    myPenWidth = 1;
    myPenColor = Qt::blue;
}

// Used to load the image and place it in the widget
bool Canvas::openImage(const QString &fileName)
{
    // Holds the image
    QImage loadedImage;

    // If the image wasn't loaded leave this function
    if(!loadedImage.load(fileName)){
        return false;
    }
    QSize newSize = loadedImage.size().expandedTo(size());
    resizeImage(&loadedImage, newSize);
    image = loadedImage;
    modified =false;  // because we just put it on screen
    update();
    return true;  //means it was successful
}

// Save the current image
bool Canvas::saveImage(const QString &fileName, const char *fileFormat)
{

    // Created to hold the image
    QImage visibleImage = image;
    resizeImage(&visibleImage, size());

    if(visibleImage.save(fileName, fileFormat)){
        modified = false;  // because it is saved
        return true;
    } else {
        return false;  // because we were not successfull
    }
}

// Used to change the pen color
void Canvas::setPenColor(const QColor &newColor)
{
    myPenColor = newColor;
}

// Used to change the pen width
void Canvas::setPenWidth(int newWidth)
{
    myPenWidth = newWidth;
}

// Color the image area with white
void Canvas::clearImage()
{
    image.fill(qRgb(255,255,255));   //White color
    modified = true;
    update();
}

// If a mouse button is pressed check if it was the
// left button and if so store the current position
// Set that we are currently drawing
void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        lastPoint = event->pos();   // our current mouse position
        scribbling = true;
    }
}

// When the mouse moves if the left button is clicked
// we call the drawline function which draws a line
// from the last position to the current
void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() & Qt::LeftButton) && scribbling){
        drawLineTo(event->pos());
    }
}

// If the button is released we set variables to stop drawing
void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && scribbling){
        drawLineTo(event->pos());
        scribbling = false;
    }
}

// QPainter provides functions to draw on the widget
// The QPaintEvent is sent to widgets that need to
// update themselves
void Canvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Returns the rectangle that needs to be updated
    QRect dirtyRect = event->rect();
    // Draws the rectangle where the image needs to be updated
    painter.drawImage(dirtyRect, image, dirtyRect);
}

// Resize the image to slightly larger then the main window
// to cut down on the need to resize the image
void Canvas::resizeEvent(QResizeEvent *event)
{
    // check if the scribble area width or height is bigger than image width or height
    if(width() > image.width() || height() > image.height())
    {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));  // return which ever is greater width or height and assign it to the image
        update();
    }
    QWidget::resizeEvent(event);
}

void Canvas::drawLineTo(const QPoint &endPoint)
{
    // Used to draw on the widget
    QPainter painter(&image);

    // Set the current settings for the pen                  //rounded on edges // rounded at any position it would join
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // Draw a line from the last registered point to the current
    painter.drawLine(lastPoint, endPoint);

    // Set that the image hasn't been saved
    modified = true;

    int rad = (myPenWidth /2) + 2;  //add alittle bit of filler around cuz we're gonna draw with rectangle

    // Call to update the rectangular space where we drew
    update(QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));  //adjusted will make it larger than it was

    // Update the last position where we left off drawing
    lastPoint = endPoint;
}

// When the app is resized create a new image using
// the changes made to the image
void Canvas::resizeImage(QImage *image, const QSize &newSize)
{
    // Check if we need to redraw the image
    if(image->size() == newSize){
        return;     // we don't need to do anything
    }

    // Create a new image to display and fill it with white
    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255,255,255));

    // Draw the image
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;  // update our image we're storing for later use
}

// Print the image
void Canvas::print()
{
// Check for print dialog availability
#if QT_CONFIG(printdialog)

    // Can be used to print
    QPrinter printer(QPrinter::HighResolution);

    // Open printer dialog and print if asked
    QPrintDialog printDialog(&printer, this);
    if(printDialog.exec() == QDialog::Accepted){
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = image.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(image.rect());
        painter.drawImage(0, 0, image);
    }
#endif // QT_CONFIG(printdialog)
}
