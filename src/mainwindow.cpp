#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStringListModel>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    this->setWindowTitle("Simple IDE");
    ui->setupUi(this);

    SetUpDockWidgets();

    ui->textEdit->hide();
    ui->consoleTextEdit->hide();
    ui->LWTargetFiles->hide();
    this->setCentralWidget(textEdit);    
    SetUpFont();
    SetUpShortcuts();
    SetUpProcess();
    SetUpDirectories();    
    connect(ui->BTNExecute, SIGNAL(clicked()), this, SLOT(CompileAndRun()));
    connect(ui->BTNCompile, SIGNAL(clicked()), this, SLOT(Compile()));

    SetUpExecutionMapWidget();
    SetUpHighlighter();
    SetUpCompleter();
    consoleTextEdit->setReadOnly(true);

    //SetUpTempFileExtChange();



}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::DisplayMessage(const QString& text)
{
    static QMessageBox messageBox;
    messageBox.setText(text);
    messageBox.show();
}

void MainWindow::SetUpFont()
{
    QFont font;
    font.setFamily(QStringLiteral("Nyala"));
    font.setPointSize(15);
    font.setPixelSize(15);
    font.setBold(false);
    font.setItalic(false);
    font.setWeight(12);
    const int tabStop = 2;  //2 characters
    QFontMetrics metrics(font);
    textEdit->setTabStopWidth(tabStop * metrics.width(' '));
    textEdit->setFont(font);
}

void MainWindow::SetUpShortcuts()
{    
    /*QShortcut* saveShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
    QShortcut* openShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);        

    QObject::connect(saveShortcut, &QShortcut::activated, this, &MainWindow::on_actionSave_triggered);
    QObject::connect(openShortcut, &QShortcut::activated, this, &MainWindow::on_actionOpen_triggered);
    */
    namespace QT = Qt;
    AddShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), &MainWindow::on_actionOpen_triggered);
    AddShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), &MainWindow::on_actionSave_triggered);
    AddShortcut(QKeySequence(Qt::CTRL + Qt::Key_N), &MainWindow::on_actionNew_triggered);
    AddShortcut(QKeySequence(Qt::CTRL + Qt::Key_R), &MainWindow::CompileAndRun);
    AddShortcut(QKeySequence(Qt::CTRL + Qt::Key_E), &MainWindow::Compile);
    AddShortcut(QKeySequence(Qt::CTRL + Qt::Key_M), &MainWindow::on_actionNew_Temporary_File_triggered);    
    //Shortcuts defined in textEdit
    //[CTRL + D] - duplicate current line
}

void MainWindow::SetUpProcess()
{    
    cmd.start("cmd.exe");
    connect(&cmd, SIGNAL(readyReadStandardOutput()), this, SLOT(OnReadyRead()));
    connect(&cmd, SIGNAL(readyReadStandardError()), this, SLOT(OnReadyReadStandardError()));
}

void MainWindow::SetUpDirectories()
{    
    for(auto ptr : {&completerPath, &templatesPath, &temporaryPath})
        if(!QDir(*ptr).exists())
            QDir().mkdir(*ptr);
}


void MainWindow::AddNewTargetFile(const QString& filename)
{
    if(targetFiles.contains(filename))
        return;
    targetFiles << filename;
}

/*
void MainWindow::DisplayTargetFiles()
{
//    auto listWidget = LWTargetFiles;
    if(listWidget->count() > 0)
        ui->LWTargetFiles->clear();
    listWidget->addItems(targetFiles);
}
*/

void MainWindow::DisplayTargetFiles()
{
    if(LWTargetFiles->count() > 0)
        LWTargetFiles->clear();
    LWTargetFiles->addItems(targetFiles);
}

void MainWindow::SetTargetFile(const QString& text)
{
    QString result = "Simple IDE - ";
    result += text;
    this->setWindowTitle(Filename::withoutFullpath(text));
    targetFile = text;
}

void MainWindow::AddShortcut(QKeySequence&& sequence, void(MainWindow::*func)())
{
    shortcuts.push_back(new QShortcut(std::move(sequence), this));
    QObject::connect(shortcuts.back(), &QShortcut::activated, this, func);
}

void MainWindow::AddShortcut(QKeySequence&& sequence, void (*func)())
{
    shortcuts.push_back(new QShortcut(std::move(sequence), this));
    QObject::connect(shortcuts.back(), &QShortcut::activated, this, func);
}




//SLOTS:------------------------
void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Get programming file", "", "Programming Files (*.cpp *.cs *.py)");
    if(filename.length() == 0)
        return;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        DisplayMessage(QStringLiteral("Failed to open \"%1\" for reading.").arg(filename));
        return;
    }

    SetTargetFile(filename);


    if(!targetFiles.contains(filename))
        targetFiles << filename;

    DisplayTargetFiles();
    textEdit->setPlainText(file.readAll());
    file.close();    
    HighlightCurrentFile();
    AutoCompleteCurrentFile();

    textEdit->isPythonFile = Filename::getExt(targetFile) == ".py";
}

void MainWindow::on_actionSave_triggered()
{
    if(targetFile.length() == 0)
        return;
    QFile file(targetFile);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        DisplayMessage(QStringLiteral("Failed to open \"%1\" for writing.").arg(targetFile));
        return;
    }
    QTextStream stream(&file);
    stream << textEdit->toPlainText();
    file.close();
}

void MainWindow::OnReadyRead()
{    
    consoleTextEdit->append(cmd.readAllStandardOutput());
}

void MainWindow::OnReadyReadStandardError()
{
    consoleTextEdit->append(cmd.readAllStandardError());
}

void MainWindow::Compile()
{
    if(targetFile.trimmed().length() == 0)
    {
        DisplayMessage("There is no target file.");
        return;
    }
    on_actionSave_triggered();

    QString fileNoExt = Filename::withoutExt(targetFile); //<<

    QString targetFileExecutable = fileNoExt + ".exe";
    QString ext = Filename::getExt(targetFile);
    QString command = compileMap[ext];
    if(command.trimmed().length() == 0)
    {
        DisplayMessage("There is no command set for \"" + ext + "\" files.");
        return;
    }
    QString fileName = Filename::withoutFullpath(targetFile);
    QString fileNameNoExt = Filename::withoutExt(fileName);
    Filename::replaceAllByRef(command, "{file}",targetFile);
    Filename::replaceAllByRef(command, "{exe}",targetFileExecutable);
    Filename::replaceAllByRef(command, "{fileName}", fileName);
    Filename::replaceAllByRef(command, "{fileNoExt}", fileNoExt);
    Filename::replaceAllByRef(command, "{fileNameNoExt}", fileNameNoExt);
    consoleTextEdit->clear();
    cmd.write(command.toLatin1() + '\n');
}

void MainWindow::CompileAndRun()
{
    if(targetFile.trimmed().length() == 0)
    {
        DisplayMessage("There is no target file.");
        return;
    }
    on_actionSave_triggered();
    QString fileNoExt = Filename::withoutExt(targetFile); // <<
    QString targetFileExecutable = fileNoExt + ".exe";
    QString ext = Filename::getExt(targetFile);
    QString command = runMap[ext];
    if(command.trimmed().length() == 0)
    {
        DisplayMessage("There is no command set for \"" + ext + "\" files.");
        return;
    }
    QString fileName = Filename::withoutFullpath(targetFile);
    QString fileNameNoExt = Filename::withoutExt(fileName);
    Filename::replaceAllByRef(command, "{file}",targetFile);
    Filename::replaceAllByRef(command, "{exe}",targetFileExecutable);
    Filename::replaceAllByRef(command, "{fileName}", fileName);
    Filename::replaceAllByRef(command, "{fileNoExt}", fileNoExt);
    Filename::replaceAllByRef(command, "{fileNameNoExt}", fileNameNoExt);
    QString fileDirName = Filename::getDirectory(targetFile);
    Filename::replaceAllByRef(command, "{fileDirName}", fileDirName);
    consoleTextEdit->clear();
    cmd.write(command.toLatin1() + '\n');
}



void MainWindow::on_LWTargetFiles_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    QFile file(current->text());
    if(!file.open(QIODevice::ReadOnly))
    {
        DisplayMessage(QStringLiteral("Failed to open \"%1\" for reading.").arg(current->text()));
        LWTargetFiles->setCurrentItem(previous);
        return;
    }
    textEdit->setPlainText(file.readAll());
    SetTargetFile(current->text());
    file.close();
}




void MainWindow::on_actionNew_triggered()
{
    QFileDialog* fileDialog = new QFileDialog;
    fileDialog->setLabelText(QFileDialog::Accept, "Create");
    fileDialog->setLabelText(QFileDialog::Reject, "Nevermind");
    fileDialog->setFileMode(QFileDialog::AnyFile);
    QString newFile = fileDialog->getSaveFileName();
    delete fileDialog;

    if(newFile.trimmed().length() == 0)
        return;


    QFile file(newFile);
    if(file.exists())
    {
        DisplayMessage("It already exists.");
        return;
    }

    QString ext = Filename::getExt(newFile);
    if(ext.trimmed().length() == 0)
    {
        DisplayMessage("Error. The file had no extension.");
        return;
    }

    if(importTemplates && extList.contains(ext))
    {
        QString templateFilePath = templatesPath + "/template" + ext;
        QFile sourceFile(templateFilePath);
        file.open(QIODevice::ReadWrite);
        sourceFile.open(QIODevice::ReadOnly);
        QTextStream stream(&file);
        stream << sourceFile.readAll();
        file.close();
        if(!file.open(QIODevice::ReadOnly))
        {
            DisplayMessage("Failed to open file after creation.");
            return;
        }
        textEdit->setPlainText(file.readAll());
        file.close();
    }
    else
    {
        if(!file.open(QIODevice::ReadWrite))
        {
            DisplayMessage("Failed to create the file.");
            return;
        }
        textEdit->setPlainText(file.readAll());
        file.close();
    }
    if(!targetFiles.contains(newFile))
        targetFiles << newFile;
    DisplayTargetFiles();
    SetTargetFile(newFile);
}

void MainWindow::on_actionNew_Temporary_File_triggered()
{
    QString temporaryPathFile = temporaryPath + "/temp" + tempFileExt;
    QString templatesPathFile = templatesPath + "/template" + tempFileExt;

    QFile sourceFile(templatesPathFile); 
    QFile file(temporaryPathFile);
    OPEN_QFILE(sourceFile, QIODevice::ReadOnly, "Failed to open for template file.\n")
    OPEN_QFILE(file, QIODevice::WriteOnly | QIODevice::Truncate, "Failed to open for template file.\n")
    file.write(sourceFile.readAll());
    file.close();
    file.open(QIODevice::ReadOnly);
    textEdit->setPlainText(file.readAll());
    sourceFile.close();
    file.close();

    if(!targetFiles.contains(temporaryPathFile))
        targetFiles << temporaryPathFile;
    DisplayTargetFiles();
    SetTargetFile(temporaryPathFile);
    HighlightCurrentFile();
    AutoCompleteCurrentFile();
    textEdit->isPythonFile = tempFileExt == ".py";
}
void MainWindow::SetUpDockWidgets()
{
    textEdit = new CTextEdit(this);
    KeyEnterReceiver* ker = new KeyEnterReceiver(this);
    textEdit->installEventFilter(ker);
    textEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    consoleTextEdit = new QTextEdit(this);
    LWTargetFiles = new QListWidget(this);
    DWTargetFiles = new QDockWidget(this);
    DWTargetFiles->setFloating(false);
    DWTargetFiles->setWidget(LWTargetFiles);    

    DWConsole = new QDockWidget(this);
    DWConsole->setFloating(false);
    DWConsole->setWidget(consoleTextEdit);    
    this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, DWTargetFiles);
    this->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, DWConsole);


    //debug
    QDockWidget* DWTempFileWidget = new QDockWidget(this);
    this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, DWTempFileWidget);

    QComboBox* CBXTempFileExt = new QComboBox(this);
    for(const auto& str : extList)
        CBXTempFileExt->addItem(str);

    connect(CBXTempFileExt, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
    [=](const QString& text)
    {
        tempFileExt = text;
    });
    DWTempFileWidget->setWidget(CBXTempFileExt);
}


void MainWindow::on_actionShow_opened_files_triggered()
{
    if(DWTargetFiles->isHidden())
        DWTargetFiles->show();
}

void MainWindow::on_actionShow_console_output_triggered()
{
    if(DWConsole->isHidden())
        DWConsole->show();
}

void MainWindow::on_actionCompile_triggered()
{
    Compile();
}

void MainWindow::on_actionExecute_triggered()
{
    CompileAndRun();
}


bool KeyEnterReceiver::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) ) {
            if(obj == parent->textEdit){
                ((CTextEdit*)obj)->handleEnterKey();
            }
        } else {
            return QObject::eventFilter(obj, event);
        }
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
    return false;
}
KeyEnterReceiver::KeyEnterReceiver(MainWindow* parent)
{
    this->parent = parent;
}

void MainWindow::SetUpExecutionMapWidget()
{
    windowExecutionMapLayout = new QGridLayout;
    windowExecutionMap.setLayout(windowExecutionMapLayout);
    windowExecutionMap.setWindowTitle("Simple IDE - execution map");
    int i = 0;
    for(auto& pair : runMap)
    {
        QLabel* label = new QLabel(this);
        label->setText(pair.first);
        windowExecutionMapLayout->addWidget(label,i,0);
        QLineEdit* lineEdit = new QLineEdit(this);
        lineEdit->setText(pair.second);
        windowExecutionMapLayout->addWidget(lineEdit,i++,1);
        executionMapQMap[pair.first] = lineEdit;
    }
    QPushButton* submit = new QPushButton(this);
    submit->setText("Submit");
    QObject::connect(submit, &QPushButton::clicked,
    [this]
    {
        for(auto& pair : executionMapQMap)
            runMap[pair.first] = pair.second->text();
    });

    QPushButton* help = new QPushButton(this);
    help->setText("Help");

    QObject::connect(help, &QPushButton::clicked,
    [this]
    {
       static auto msg =
       "Guide to using the execution mapper:\n"
       "Let's pretend the absolute path to the file is: C:\\Owner\\Documents\\file.txt\n"
       "--------------\n"
       "{file} - the absolute path to the currently targetted file:\n"
       "C:\\Owner\\Documents\\file.txt\n\n"
       "--------------\n"
       "{fileNoExt} - the absolute path to the currently targetted file with the ext removed:\n"
       "C:\\Owner\\Documents\\file\n\n"
       "--------------\n"
       "{fileName} - just the file name:\n"
       "file.txt\n\n"
       "--------------\n"
       "{exe} - the absolute path with the extension replaced with .exe:\n"
       "C:\\Owner\\Documents\\file.exe\n\n"
       "--------------\n"
       "{fileNameNoExt} - just the file name, and with the extension replaced with .exe"
       "file.exen\n";
       DisplayMessage(msg);
    });
    windowExecutionMapLayout->addWidget(submit, i,0);
    windowExecutionMapLayout->addWidget(new QLabel(this), i,1);
    windowExecutionMapLayout->addWidget(help,i,2);

    const auto& WEMGeo = windowExecutionMap.geometry();
    const int thisWidth = this->geometry().width();

    windowExecutionMap.setGeometry(WEMGeo.x(), WEMGeo.y(), thisWidth, WEMGeo.height());
    windowExecutionMap.show();
}

void MainWindow::SetUpHighlighter()
{
    highlighters[".cpp"] = new Highlighter(0,
    {
     QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
     QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
     QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
     QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
     QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
     QStringLiteral("\\bshort\\b"), QStringLiteral("\\bsignals\\b"), QStringLiteral("\\bsigned\\b"),
     QStringLiteral("\\bslots\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bstruct\\b"),
     QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"),
     QStringLiteral("\\bunion\\b"), QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bvirtual\\b"),
     QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bbool\\b"),
     QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bvector\\b"), QStringLiteral("\\bstring\\b")
    });
    highlighters[".cs"] = new Highlighter(0,
    {   QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
        QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bbool\\b"), QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bvar\\b")
    });

    highlighters[".java"] = new Highlighter(0,
    {   QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"),QStringLiteral("\\binline\\b"),
        QStringLiteral("\\bint\\b"), QStringLiteral("\\bInteger\\b"), QStringLiteral("\\bDouble\\b"),
        QStringLiteral("\\blong\\b"), QStringLiteral("\\bLong\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bboolean\\b"), QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bvar\\b"),
        QStringLiteral("\\bSystem\\b"), QStringLiteral("\\bout\\b"), QStringLiteral("\\bScanner\\b"),QStringLiteral("\\bimport\\b")
    });

    highlighters[".py"] = new Highlighter(0,
    {
       QStringLiteral("\\bprint\\b"),QStringLiteral("\\bfloat\\b"),
       QStringLiteral("\\bdef\\b"), QStringLiteral("\\bint\\b"),
        QStringLiteral("\\bstr\\b"), QStringLiteral("\\blist\\b")
    });
}

void MainWindow::HighlightCurrentFile()
{
    auto it = highlighters.begin();
    for(QString ext = Filename::getExt(targetFile); it != highlighters.end(); it++)
        it.value()->setDocument(it.key() == ext ? this->textEdit->document() : 0);
}

void MainWindow::AutoCompleteCurrentFile()
{
    auto it = completers.begin();
    for(QString ext = Filename::getExt(targetFile); it != completers.end(); it++)
    {
        if(it.key() == ext)
        {
            textEdit->setCompleter(it.value());
            return;
        }
    }
}

void MainWindow::SetUpCompleter()
{
    completers[".cpp"] = new QCompleter(this);
    completers[".cs"] = new QCompleter(this);
    completers[".py"] = new QCompleter(this);
    completers[".cpp"]->setModel(modelFromFile(completerPath + "/cpp.txt"));
    completers[".cs"]->setModel(modelFromFile(completerPath + "/cs.txt"));
    completers[".py"]->setModel(modelFromFile(completerPath + "/py.txt"));
    for(auto it = completers.begin(); it != completers.end(); it++)
    {
        it.value()->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
        it.value()->setCaseSensitivity(Qt::CaseInsensitive);
        it.value()->setWrapAround(false);
    }
}

void MainWindow::SetUpTempFileExtChange()
{

}





QAbstractItemModel *MainWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(&completer);

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << QString::fromUtf8(line.trimmed());
    }

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, &completer);
}

void MainWindow::createMenu()
{
    QAction *exitAction = new QAction(tr("Exit"), this);
    //QAction *aboutAct = new QAction(tr("About"), this);
    QAction *aboutQtAct = new QAction(tr("About Qt"), this);

    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    //connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(exitAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("About"));
   // helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::on_actionEdit_execution_map_triggered()
{
    if(windowExecutionMap.isHidden())
        windowExecutionMap.show();
    else
        windowExecutionMap.raise();
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    windowExecutionMap.close();
}
