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

#ifndef INCLUDE_MEDIASTREAMER2_QTDISPLAYEVENT_H_INCLUDED_FCF355F077B511E58F2B00E04C68002D
#define INCLUDE_MEDIASTREAMER2_QTDISPLAYEVENT_H_INCLUDED_FCF355F077B511E58F2B00E04C68002D

#include <QEvent>
#include <QImage>

class QtDisplayEvent : public QEvent
{
public:
	QtDisplayEvent(Type type, QImage image);

	QImage image();

	/**
	 * Display event id;
	 */
	static int Display;

private:
	QImage m_image;
};

#endif /* INCLUDE_MEDIASTREAMER2_QTDISPLAYEVENT_H_INCLUDED_FCF355F077B511E58F2B00E04C68002D */
