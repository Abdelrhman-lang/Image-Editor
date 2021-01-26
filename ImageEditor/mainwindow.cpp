#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvas.h"
#include <QFile>
#include <QFileDialog>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QImageWriter>
#include <QCloseEvent>

// MainWindow Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create the Canvas widget and make it
    // the central widget
    canvas = new Canvas;
    setCentralWidget(canvas);

    // Create actions and menus
    createActions();
    createMenus();

    // Set the title
    setWindowTitle(tr("Image Process"));

    // Size the app
    resize(500,500);

}

MainWindow::~MainWindow()
{
    delete ui;
}

// User tried to close the app
void MainWindow::closeEvent(QCloseEvent *event)
{
    // If they try to close maybeSave() returns true
    // if no changes have been made and the app closes
    if(maybeSave()){
        event->accept();
    }else {
        // If there have been changes ignore the event
        event->ignore();
    }
}

// Check if the current image has been changed and then
// open a dialog to open a file
void MainWindow::open()
{
    // Check if changes have been made since last save
    // maybeSave() returns true if no changes have been made
    if(maybeSave()){

        // Get the file to open from a dialog
        // tr sets the window title to Open File
        // QDir opens the current dirctory
        QString fileName = QFileDialog::getOpenFileName(this,
                tr("Open File"), QDir::currentPath());

        // If we have a file name load the image and place
        // it in the scribbleArea
        if(!fileName.isEmpty()){
            canvas->openImage(fileName);
        }
    }
}

// Called when the user clicks Save As in the menu
void MainWindow::save()
{
    // A QAction represents the action of the user clicking
    QAction *action = qobject_cast<QAction *>(sender());

    // Stores the array of bytes of the users data
    QByteArray fileFormat = action->data().toByteArray();

    // Pass it to be saved
    saveFile(fileFormat);
}

// Opens a dialog to change the pen color
void MainWindow::penColor()
{
    // Store the chosen color from the dialog
    QColor newColor = QColorDialog::getColor(canvas->penColor());

    // If a valid color set it
    if(newColor.isValid())
    {
        canvas->setPenColor(newColor);
    }
}

// Opens a dialog that allows the user to change the pen width
void MainWindow::penWidth()
{
    // Stores button value
    bool ok;

    // tr("Scribble") is the title
    // the next tr is the text to display
    // Get the current pen width
    // Define the min, max, step and ok button
    int newWidth = QInputDialog::getInt(this, tr("Scribble"),
                                        tr("Select Pen Width : "),
                                        canvas->penWidth(),
                                        1, 50, 1, &ok);
    // Change the pen width
    if(ok)
    {
        canvas->setPenWidth(newWidth);
    }
}

// Open an about dialog
void MainWindow::about()
{
    // Window title and text to display
    QMessageBox::about(this, tr("About"),
                       tr("<p>The <b>Scribble</b> example is awesome</p>"));
}

// Define menu actions that call functions
void MainWindow::createActions()
{
    // Create the action tied to the menu
    openAct = new QAction(tr("&Open"), this);

    // Define the associated shortcut key
    openAct->setShortcuts(QKeySequence::Open);

    // Tie the action to MainWindow::open()
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    // Get a list of the supported file formats
    // QImageWriter is used to write images to files
    foreach(QByteArray format, QImageWriter::supportedImageFormats())  // returns supported image formats by QImageWriter
    {
        QString text = tr("%1...").arg(QString(format).toUpper());  // PNG,JPEG,.. and save it on our Qstring (text)

        // Create an action for each file format
        QAction *action = new QAction(text, this);
        action->setData(format);    // set action for each of the file formats

        // When clicked call MainWindow::save()
        connect(action, SIGNAL(triggered()), this, SLOT(save()));
        saveAsActs.append(action);  // Attach each file format option menu item to Save As
    }

    // Create print action and tie to MainWindow::print()
    printAct = new QAction(tr("&Print..."), this);
    connect(printAct, SIGNAL(triggered()), canvas, SLOT(print()));

    // Create exit action and tie to MainWindow::close()
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    // Create pen color action and tie to MainWindow::penColor()
    penColorAct = new QAction(tr("&Pen Color..."), this);
    connect(penColorAct, SIGNAL(triggered()), this, SLOT(penColor()));

    // Create pen width action and tie to MainWindow::penWidth()
    penWidthAct = new QAction(tr("Pen &Width..."), this);
    connect(penWidthAct, SIGNAL(triggered()), this, SLOT(penWidth()));

    // Create clear screen action and tie to MainWindow::clearImage()
    clearScreenAct = new QAction(tr("&Clear Screen"), this);
    clearScreenAct->setShortcut(tr("Ctrl+L"));
    connect(clearScreenAct, SIGNAL(triggered()),
            canvas, SLOT(clearImage()));

    // Create about action and tie to MainWindow::about()
    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    // Create about Qt action and tie to MainWindow::aboutQt()
    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

// Create the menubar
void MainWindow::createMenus()
{
    // Create Save As option and the list of file types
    saveAsMenu = new QMenu(tr("&Save As"), this);
    foreach(QAction *action, saveAsActs)
        saveAsMenu->addAction(action);

    // Attach all actions to File
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addMenu(saveAsMenu);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // Attach all actions to Options
    optionMenu = new QMenu(tr("&Options"), this);
    optionMenu->addAction(penColorAct);
    optionMenu->addAction(penWidthAct);
    optionMenu->addSeparator();
    optionMenu->addAction(clearScreenAct);

    // Attach all actions to Help
    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    // Add menu items to the menubar
    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(optionMenu);
    menuBar()->addMenu(helpMenu);
}

// if the user try to leave the application without saving
bool MainWindow::maybeSave()
{
   // Check for changes since last save
    if(canvas->isModified()){
        QMessageBox::StandardButton ret;

        // Image Editor is the title
        // Add text and the buttons
        ret = QMessageBox::warning(this, tr("Image Editor"),
                                   tr("The Image has been modified. \n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

   // If save button clicked call for file to be saved
     if(ret == QMessageBox::Save){
         return saveFile("png");

       // If cancel do nothing
     } else if(ret == QMessageBox::Cancel){
         return false;
     }

    }
    return true;
}

bool MainWindow::saveFile(const QByteArray &fileFormat)
{
    // Define path, name and default file type
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    // Get selected file from dialog box
    // Add the proper file formats and extensions
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    initialPath,
                                                    tr("%1 Files (*.%2);;All Files (*)")
                                                    .arg(QString::fromLatin1(fileFormat.toUpper()))
                                                    .arg(QString::fromLatin1(fileFormat)));
    // If no file do nothing
    if(fileName.isEmpty()){
        return false;
    } else {
        // Call for the file to be saved
        // .constData() --> Returns a pointer to the data stored in the byte array.
        // This function is mostly useful to pass a byte array to a function that accepts a const char *.
        return canvas->saveImage(fileName, fileFormat.constData());
    }
}
