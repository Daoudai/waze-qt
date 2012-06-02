#ifndef WAZEIMAGEPROVIDER_H
#define WAZEIMAGEPROVIDER_H

#include <QObject>

class WazeImageProvider : public QObject
{
    Q_OBJECT
public:
    explicit WazeImageProvider(QObject *parent = 0);

    Q_INVOKABLE QString getImage(QString imageName);
    
signals:
    
public slots:

private:
    QString _imagesPath;
};

#endif // WAZEIMAGEPROVIDER_H
