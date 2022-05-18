#ifndef COMPLETERDATABASE_H
#define COMPLETERDATABASE_H
#include <QMap>
#include <QVector>
#include <QString>
#include <QDir>
#include <QDirIterator>

class CompleterDatabase
{
private:
    bool refreshList(QString, QString);
    bool fillList(QString);
    QMap<QString, QStringList> completerList;

    const QString completerPath = QDir().currentPath() + "/completers";
public:
    CompleterDatabase();
    QStringList* operator[](const QString&);
    enum GetFromFileMode
    {
        DontReplace, Overwrite
    };
    void GetFromFile(const QString&, GetFromFileMode);
    void SetUp();

    void DisplayMessage(const QString&);

};


#endif // COMPLETERDATABASE_H
