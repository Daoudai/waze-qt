#ifndef QT_GLOBAL_H
#define QT_GLOBAL_H

#include <QByteArray>

class WazeString {
public:
    explicit WazeString() {

    }

    WazeString(const QString str) {
        _str = str.toLocal8Bit();
    }

    WazeString(const WazeString& other) {
        if (&other != this)
        {
            _str = other._str;
        }
    }

    WazeString &operator=(const WazeString &other)
    {
        if (&other != this)
        {
            _str = other._str;
        }

        return *this;
    }

    WazeString &operator=(const QString &str)
    {
        _str = str.toLocal8Bit().data();

        return *this;
    }

    bool operator==(const WazeString &other) const
    {
        if (&other == this)
        {
            return true;
        }

        return _str == other._str;
    }

    bool operator==(const QString &str) const
    {
        return _str == str.toLocal8Bit();
    }

    int length() const {
        return _str.length();
    }

    const char *getStr() const {
        return _str.data();
    }

private:
    QByteArray _str;
};

#endif // QT_GLOBAL_H
