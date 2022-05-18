#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QFont>
#include <QVector>
#include <QShortcut>
#include <QKeySequence>
#include <QTextStream>
#include <QProcess>
#include <QMap>
#include <QCompleter>
#include <QDir>
#include <QStringList>
#include <QListWidgetItem>
#include <map>
#include <QKeyEvent>
#include <CustomFunctions.h>
#include <ctextedit.h>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <highlighter.h>
#include <completerdatabase.h>
#include <QAbstractItemModel>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE






class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    
     CTextEdit* textEdit;

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();    
    void OnReadyRead();
    void OnReadyReadStandardError();
    void Compile();
    void CompileAndRun();  
    void on_LWTargetFiles_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);



    void on_actionNew_triggered();
    void on_actionNew_Temporary_File_triggered();

    void on_actionShow_opened_files_triggered();

    void on_actionShow_console_output_triggered();

    void on_actionCompile_triggered();

    void on_actionExecute_triggered();

    void on_actionEdit_execution_map_triggered();

private: //constants
    const QString completerPath = QDir().currentPath() + "/completers";
    const QString templatesPath = QDir().currentPath() + "/templates";
    const QString temporaryPath = QDir().currentPath() + "/temporaryDirectory";    
private:

   // CompleterDatabase completerDatabase;

    //highlighters:
    QMap<QString, Highlighter*> highlighters;
    QMap<QString, QCompleter*> completers;


    QDockWidget* DWTargetFiles;
    QDockWidget* DWConsole;
    QListWidget* LWTargetFiles;
    QTextEdit* consoleTextEdit;

    QWidget windowExecutionMap;
    std::map<QString, QLineEdit*> executionMapQMap;
    QGridLayout* windowExecutionMapLayout;

    //debug
    QDockWidget* DWCursorMonitor;
    QLabel* LBCursorMonitor;
    //...
    Ui::MainWindow *ui;
    QString targetFile;
    QStringList targetFiles;    
    QVector<QShortcut*> shortcuts;

    bool importTemplates = true;
    QVector<QString> extList = {".cpp", ".cs", ".java", ".py"};
    QString tempFileExt = ".cpp";

    QProcess cmd;
    QMap<QString,QString> compileMap{
        {".cpp", "g++ -std=c++17 {file} -o {exe}"},
        {".cs", "\"C:/Program Files/Mono/bin/mcs.bat\" {file}"},
        {".py", "start cmd.exe /c \"{fileDirName}.\\{fileNameNoExt}.py & pause\""}
    };

    std::map<QString, QString> runMap{
      {".cpp", "g++ -std=c++17 {file} -o {exe} && start cmd.exe /c \"{fileDirName}.\\{fileNameNoExt}.exe & echo[ & echo[ & pause\""},
      {".cs", "\"C:/Program Files/Mono/bin/mcs.bat\" {file} && start cmd.exe /c \"{fileDirName}.\\{fileNameNoExt}.exe & echo[ & echo[ & pause\""},
      {".py", "start cmd.exe /c \"{fileDirName}.\\{fileNameNoExt}.py & pause\""},
      {".java", "javac {file} && java {fileNoExt}"}
    };
    /*
     *    std::map<QString, QString> runMap{
      {".cpp", "g++ -std=c++17 {file} -o {exe} && start {exe}"},
      {".cs", "C:/Program Files/Mono/bin/mcs.bat {file} && start {exe}"},
      {".py", "python {file}"},
      {".java", "javac {file} && java {fileNoExt}"}
    };
     */

    QCompleter completer;

    void DisplayMessage(const QString&);
    void SetUpFont();
    void SetUpShortcuts();
    void SetUpProcess();    
    void SetUpDirectories();
    void SetUpDockWidgets();
    void AddNewTargetFile(const QString& str);
    void DisplayTargetFiles();
    void SetTargetFile(const QString&);
    void AddShortcut(QKeySequence&&, void(MainWindow::*func)());
    void AddShortcut(QKeySequence&&, void(*func)());
    void SetUpExecutionMapWidget();
    void SetUpHighlighter();
    void HighlightCurrentFile();
    void AutoCompleteCurrentFile();
    void SetUpCompleter();

    void SetUpTempFileExtChange();
    void closeEvent(QCloseEvent*) override;
    QAbstractItemModel* modelFromFile(const QString&);
    void createMenu();


    //new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
};

#define OPEN_QFILE(file, flags ,str) if(!file.open(flags))\
{\
    DisplayMessage(str);\
    return;\
}

class KeyEnterReceiver : public QObject
{
    Q_OBJECT
public:
    KeyEnterReceiver(MainWindow*);
protected:
    MainWindow* parent = nullptr;
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif // MAINWINDOW_H
