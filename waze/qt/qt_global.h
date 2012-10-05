#ifndef QT_GLOBAL_H
#define QT_GLOBAL_H

#include <QByteArray>
#include <QVariant>

void roadmap_main_set_qml_context_property(const char* name, QObject* value);

class WazeString {
public:
    explicit WazeString() {

    }

    inline WazeString(const QString str) {
        _str = str.toLocal8Bit();
    }

    inline WazeString(const WazeString& other) {
        if (&other != this)
        {
            _str = other._str;
        }
    }

    inline WazeString &operator=(const WazeString &other)
    {
        if (&other != this)
        {
            _str = other._str;
        }

        return *this;
    }

    inline WazeString &operator=(const QString &str)
    {
        _str = str.toLocal8Bit().data();

        return *this;
    }

    inline bool operator==(const WazeString &other) const
    {
        if (&other == this)
        {
            return true;
        }

        return _str == other._str;
    }

    inline bool operator==(const QString &str) const
    {
        return _str == str.toLocal8Bit();
    }

    inline int length() const {
        return _str.length();
    }

    inline const char *getStr() const {
        return _str.data();
    }

    inline QString getQStr() const {
        return QString(_str);
    }

private:
    QByteArray _str;
};

#endif // QT_GLOBAL_H
