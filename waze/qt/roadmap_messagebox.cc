/* roadmap_messagebox.cc - The C to C++ wrapper for the QT RoadMap message box.
 *
 * LICENSE:
 *
 *   (c) Copyright 2003 Latchesar Ionkov
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * SYNOPSYS:
 *
 *   See roadmap_messagebox.h
 */
#include <QMessageBox>
#include <QTimer>

extern "C" {
#include "roadmap_start.h"
#define __ROADMAP_MESSAGEBOX_NO_LANG
#include "roadmap_messagebox.h"
}

class MessageBoxHelper : public QObject
{
    Q_OBJECT
public:
    MessageBoxHelper() {}
    ~MessageBoxHelper() {}
    void setCallback(messagebox_closed callback)
    {
        _callback = callback;
    }
public slots:
    void invoke_callback(int result)
    {
        _callback(result);
    }
private:
    messagebox_closed _callback;
};


void roadmap_messagebox (const char *title, const char *message)
{
    QMessageBox *mb = new QMessageBox( 0);
    mb->setWindowTitle(title);
    mb->setText ( message );
    mb->setWindowModality ( Qt::NonModal );
    mb->show();
    mb->raise();
}

void roadmap_messagebox_custom( const char *title, const char *message,
                int title_font_size, char* title_color, int text_font_size, char* text_color )
{
    QMessageBox *mb = new QMessageBox( 0);
    mb->setWindowTitle(title);
    mb->setText ( message );
    mb->setWindowModality ( Qt::NonModal );
    mb->show();
    mb->raise();
}

void roadmap_messagebox_cb(const char *title, const char *message,
         messagebox_closed on_messagebox_closed)
{
    QMessageBox *mb = new QMessageBox( 0);
    MessageBoxHelper *mbh = new MessageBoxHelper();
    mbh->setCallback(on_messagebox_closed);
    QObject::connect(mb, SIGNAL(finished(int)), mbh, SLOT(invoke_callback(int)));
    QObject::connect(mb, SIGNAL(destroyed()), mbh, SLOT(deleteLater()));
    mb->setWindowTitle(title);
    mb->setText ( message );
    mb->setWindowModality ( Qt::NonModal );
    mb->show();
    mb->raise();
}

void roadmap_messagebox_timeout (const char *title, const char *message, int seconds)
{
    QMessageBox *mb = new QMessageBox( 0);
    QTimer::singleShot(seconds*1000, mb, SLOT(accept()));
    mb->setWindowTitle(title);
    mb->setText ( message );
    mb->setWindowModality ( Qt::NonModal );
    mb->show();
    mb->raise();
}
