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

extern "C"
{

#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msvideo.h"

#include "layouts.h"

};

#include <QMainWindow>
#include <QApplication>
#include "qtdisplaywindow.h"

struct QtDisplay {
public:
	QSharedPointer<QtDisplayWindow> window;
};

static void qt_display_init(MSFilter  *f){
	QtDisplay * data = NULL;

	data = new QtDisplay;
	data->window.reset(new QtDisplayWindow());
	data->window->moveToThread(QApplication::instance()->thread());
}

static void qt_display_uninit(MSFilter *f){
	delete (QtDisplay*)f->data;
}

static void qt_display_preprocess(MSFilter *f){
	
}

static void qt_display_process(MSFilter *f){
}

static MSFilterMethod methods[]={
	{	0	,NULL}
};

MSFilterDesc ms_qt_display_desc={
  MS_DRAWDIB_DISPLAY_ID,
  "MSQtDisplay",
  N_("A video display based on Qt api"),
  MS_FILTER_OTHER,
  NULL,
  2,
  0,
  qt_display_init,
  qt_display_preprocess,
  qt_display_process,
  NULL,
  qt_display_uninit,
  methods
};

MS_FILTER_DESC_EXPORT(ms_qt_display_desc)
