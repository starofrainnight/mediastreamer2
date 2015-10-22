/*
Qt video display filter for mediastreamer2

Copyright (C) 2015 Hong-She Liang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SRC_VIDEOFILTERS_QTDISPLAYWINDOW_H_INCLUDED_31A05A4C77A611E5B7B300E04C68002D
#define SRC_VIDEOFILTERS_QTDISPLAYWINDOW_H_INCLUDED_31A05A4C77A611E5B7B300E04C68002D

#include <QMainWindow>

class QtDisplayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtDisplayWindow(QWidget *parent = 0);
    bool event(QEvent * e);
};

#endif /* SRC_VIDEOFILTERS_QTDISPLAYWINDOW_H_INCLUDED_31A05A4C77A611E5B7B300E04C68002D */
