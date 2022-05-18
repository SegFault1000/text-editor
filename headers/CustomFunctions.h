#ifndef FILENAME_H
#define FILENAME_H
#include <QString>
namespace Filename
{
    QString withoutExt(QString);
    QString getExt(QString);
    QString replaceAll(QString, const QString&, const QString&);
    void replaceAllByRef(QString&, const QString&, const QString&);
    QString withoutFullpath(QString);
    QString getDirectory(QString);
}

namespace extra
{
    template<class InputIt, class T>
    constexpr InputIt find(InputIt first, InputIt last, const T& value)
    {
        for (; first != last; ++first) {
            if (*first == value) {
                return first;
            }
        }
        return last;
    }
    template<class InputIt, class UnaryPredicate>
    constexpr InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
    {
        for (; first != last; ++first) {
            if (p(*first)) {
                return first;
            }
        }
        return last;
    }
}
#endif // FILENAME_H
