#include <CustomFunctions.h>

namespace Filename
{
    QString withoutExt(QString name)
        {
              int pos = name.lastIndexOf('.');
                if(pos == -1)
                      return name;
                name.chop(name.length() - pos);
                return name;
        }
        QString getExt(QString name)
        {
              int pos = name.lastIndexOf('.');
                if(pos == -1)
                     return "";
                name.remove(0, pos);
                return name;
        }
        QString replaceAll(QString source, const QString& target, const QString& newVal)
        {
              int pos = -1;
                while((pos = source.indexOf(target)) != -1)
                      source.replace(pos, target.length(), newVal);
                return source;
        }
        void replaceAllByRef(QString& source, const QString& target, const QString& newVal)
        {
              int pos = -1;
                while((pos = source.indexOf(target)) != -1)
                      source.replace(pos, target.length(), newVal);
        }

        QString withoutFullpath(QString str)
        {
              int pos = str.lastIndexOf('/');
                if(pos == -1)
                      return str;
                str.remove(0, pos + 1);
                return str;

        }

        QString getDirectory(QString str)
        {
            int pos{};
            if((pos = str.lastIndexOf("/")) != -1)
                str.chop(str.length() - pos);
            return str;
        }
}
