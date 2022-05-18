#include "completerdatabase.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
CompleterDatabase::CompleterDatabase()
{
    SetUp();
}

QStringList* CompleterDatabase::operator[](const QString& ext){
    auto it = completerList.find(ext);
    if(it == completerList.end())
        return nullptr;
    else
        return &*it;
}

void CompleterDatabase::GetFromFile(const QString& filename, GetFromFileMode mode = GetFromFileMode::Overwrite)
{
    QFile file(filename);

    int pos{};
    if((pos = filename.indexOf(".")) == -1)
    {
        DisplayMessage(QStringLiteral("%1\nDoes not have an extension").arg(filename));
        return;
    }
    QString ext = filename.mid(0, pos);
    auto it = completerList.find(ext);
    if(it != completerList.end() && mode == GetFromFileMode::DontReplace)
        return;

    if(!file.exists())
    {
        DisplayMessage(QStringLiteral("Completer tried to get list from non-existent file: %1").arg(filename));
        return;
    }
    if(!file.open(QIODevice::ReadOnly))
    {
        DisplayMessage(QStringLiteral("Completer failed to open %1").arg(filename));
        return;
    }


    if(it != completerList.end())
        it->clear();

    QTextStream stream(&file);
    for(QString temp; stream.readLineInto(&temp);)
        completerList[ext] << temp.trimmed();
    file.close();
}

void CompleterDatabase::SetUp()
{

    QDirIterator it(completerPath);
    while(it.hasNext())
        GetFromFile(it.next());
}

void CompleterDatabase::DisplayMessage(const QString& msg)
{
    static QMessageBox messageBox;
    messageBox.setText(msg);
    messageBox.show();
}
