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

#ifndef SRC_VIDEOFILTERS_QTDISPLAYRESIZEEVENT_H_INCLUDED_D098EAF877BD11E59EC100E04C68002D
#define SRC_VIDEOFILTERS_QTDISPLAYRESIZEEVENT_H_INCLUDED_D098EAF877BD11E59EC100E04C68002D

#include <QEvent>
#include <QSize>

class QtDisplayResizeEvent : public QEvent
{
public:
	QtDisplayResizeEvent(Type type, const QSize &size);

	QSize size();

	/**
	 * Display event id;
	 */
	static int DisplayResize;

private:
	QSize m_size;
};

#endif /* SRC_VIDEOFILTERS_QTDISPLAYRESIZEEVENT_H_INCLUDED_D098EAF877BD11E59EC100E04C68002D */
