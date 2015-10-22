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
#include <QRect>
#include <QImage>
#include <QPainter>
#include <QSharedPointer>
#include "qtdisplayevent.h"
#include "qtdisplayresizeevent.h"
#include "qtdisplaywindow.h"

#define SCALE_FACTOR 4.0f
#define SELVIEW_POS_INACTIVE -100.0
#define LOCAL_BORDER_SIZE 2

typedef struct Yuv2RgbCtx{
	uint8_t *rgb;
	size_t rgblen;
	MSVideoSize dsize;
	MSVideoSize ssize;
	MSScalerContext *sws;
}Yuv2RgbCtx;

struct QtDisplay {
public:
	QSharedPointer<QtDisplayWindow> window;
	QImage mainImage;
	QImage localImage;
	MSVideoSize wsize; /*the initial requested window size*/
	MSVideoSize vsize; /*the video size received for main input*/
	MSVideoSize lsize; /*the video size received for local display */
	Yuv2RgbCtx mainview;
	Yuv2RgbCtx locview;
	int sv_corner;
	float sv_scalefactor;
	float sv_posx,sv_posy;
	int background_color[3];
	bool_t need_repaint;
	bool_t autofit;
	bool_t mirroring;
	bool_t own_window;
	bool_t auto_window;
};


static void yuv2rgb_init(Yuv2RgbCtx *ctx){
	ctx->rgb=NULL;
	ctx->rgblen=0;
	ctx->dsize.width=0;
	ctx->dsize.height=0;
	ctx->ssize.width=0;
	ctx->ssize.height=0;
	ctx->sws=NULL;
}

static void yuv2rgb_uninit(Yuv2RgbCtx *ctx){
	if (ctx->rgb){
		ms_free(ctx->rgb);
		ctx->rgb=NULL;
		ctx->rgblen=0;
	}
	if (ctx->sws){
		ms_scaler_context_free(ctx->sws);
		ctx->sws=NULL;
	}
	ctx->dsize.width=0;
	ctx->dsize.height=0;
	ctx->ssize.width=0;
	ctx->ssize.height=0;
}

static void yuv2rgb_prepare(Yuv2RgbCtx *ctx, MSVideoSize src, MSVideoSize dst){
	if (ctx->sws!=NULL) yuv2rgb_uninit(ctx);
	ctx->sws=ms_scaler_create_context(src.width,src.height,MS_YUV420P,
			dst.width,dst.height, MS_RGB24_REV,
			MS_SCALER_METHOD_BILINEAR);
	ctx->dsize=dst;
	ctx->ssize=src;
	ctx->rgblen=dst.width*dst.height*3;
	ctx->rgb=(uint8_t*)ms_malloc0(ctx->rgblen+dst.width);
}


/*
 this function resizes the original pictures to the destination size and converts to rgb.
 It takes care of reallocating a new SwsContext and rgb buffer if the source/destination sizes have
 changed.
*/
static void yuv2rgb_process(Yuv2RgbCtx *ctx, MSPicture *src, MSVideoSize dstsize, bool_t mirroring){
	MSVideoSize srcsize;

	srcsize.width=src->w;
	srcsize.height=src->h;
	if (!ms_video_size_equal(dstsize,ctx->dsize) || !ms_video_size_equal(srcsize,ctx->ssize)){
		yuv2rgb_prepare(ctx,srcsize,dstsize);
	}
	{
		int rgb_stride=-dstsize.width*3;
		uint8_t *p;

		p=ctx->rgb+(dstsize.width*3*(dstsize.height-1));
		if (ms_scaler_process(ctx->sws,src->planes,src->strides, &p, &rgb_stride)<0){
			ms_error("Error in 420->rgb ms_scaler_process().");
		}
		if (mirroring) rgb24_mirror(ctx->rgb,dstsize.width,dstsize.height,dstsize.width*3);
	}
}

static void yuv2rgb_draw(Yuv2RgbCtx *ctx, QImage image, int dstx, int dsty){
	if (ctx->rgb){
		QPainter painter(&image);
		QImage srcImage(ctx->rgb, ctx->dsize.width, ctx->dsize.height, QImage::Format_RGB888);

		painter.drawImage(
				QRect(dstx, dsty, image.width(), image.height()),
				srcImage,
				QRect(0, 0, ctx->dsize.width, ctx->dsize.height));
	}
}

/*
* Draws a background, that is the black rectangles at top, bottom or left right sides of the video display.
* It is normally invoked only when a full redraw is needed (notified by Windows).
*/
static void draw_background(QImage image, MSVideoSize wsize, MSRect mainrect, int color[3]) {
	QColor brush;
	QRect brect;
	QPainter painter(&image);

	brush = QColor(qRgb(color[0],color[1],color[2]));
	if (mainrect.x>0){
		brect.setLeft(0);
		brect.setTop(0);
		brect.setRight(mainrect.x);
		brect.setBottom(wsize.height);
		painter.fillRect(brect, brush);

		brect.setLeft(mainrect.x+mainrect.w);
		brect.setTop(0);
		brect.setRight(wsize.width);
		brect.setBottom(wsize.height);
		painter.fillRect(brect, brush);
	}
	if (mainrect.y>0){
		brect.setLeft(0);
		brect.setTop(0);
		brect.setRight(wsize.width);
		brect.setBottom(mainrect.y);
		painter.fillRect(brect, brush);

		brect.setLeft(0);
		brect.setTop(mainrect.y+mainrect.h);
		brect.setRight(wsize.width);
		brect.setBottom(wsize.height);
		painter.fillRect(brect, brush);
	}
	if (mainrect.w==0 && mainrect.h==0){
		/*no image yet, black everything*/
		brect.setLeft(0);
		brect.setTop(0);
		brect.setRight(wsize.width);
		brect.setBottom(wsize.height);
		painter.fillRect(brect, brush);
	}
}

static void draw_local_view_frame(QImage image, MSVideoSize wsize, MSRect localrect){
	QPainter painter(&image);

	painter.drawRect(QRect(
			QPoint(localrect.x-LOCAL_BORDER_SIZE, localrect.y-LOCAL_BORDER_SIZE),
			QPoint(localrect.x+localrect.w+LOCAL_BORDER_SIZE, localrect.y+localrect.h+LOCAL_BORDER_SIZE)));
}

static int get_vsize(MSFilter *f, void *data){
	QtDisplay *obj=(QtDisplay*)f->data;
	*(MSVideoSize*)data=obj->wsize;
	return 0;
}

static int set_vsize(MSFilter *f, void *data){
	QtDisplay *obj=(QtDisplay*)f->data;
	obj->wsize=*(MSVideoSize*)data;
	return 0;
}

static int get_native_window_id(MSFilter *f, void *data){
	QtDisplay *obj=(QtDisplay*)f->data;
	if(obj->auto_window) {
		*(long*)data=(long)obj->window.data();
	} else {
		*(unsigned long*)data=MS_FILTER_VIDEO_NONE;
	}
	return 0;
}

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
	QtDisplay *obj=(QtDisplay*)f->data;
	QRect rect;
	MSVideoSize wsize; /* the window size*/
	MSVideoSize vsize;
	MSVideoSize lsize; /*local preview size*/
	QImage * mainImagePtr = NULL;
	QImage * localImagePtr = NULL;
	MSRect mainrect;
	MSRect localrect;
	MSPicture mainpic;
	MSPicture localpic;
	mblk_t *main_im=NULL;
	mblk_t *local_im=NULL;
	bool_t repainted=FALSE;
	int corner=obj->sv_corner;
	float scalefactor=obj->sv_scalefactor;

	if (!obj->window) {
		goto end;
	}

	rect = obj->window->geometry();
	if (rect.isEmpty() || rect.width()<=32 || rect.height()<=32) {
		goto end;
	}

	mainImagePtr = &obj->mainImage;
	localImagePtr = &obj->localImage;

	wsize.width=rect.width();
	wsize.height=rect.height();
	if (!ms_video_size_equal(wsize, obj->wsize))
		obj->need_repaint=TRUE;
	obj->wsize=wsize;

	/*get most recent message and draw it*/
	if (corner!=-1 && f->inputs[1]!=NULL && (local_im=ms_queue_peek_last(f->inputs[1]))!=NULL) {
		if (ms_yuv_buf_init_from_mblk(&localpic,local_im)==0){
			obj->lsize.width=localpic.w;
			obj->lsize.height=localpic.h;
		}
	}

	if (f->inputs[0]!=NULL && (main_im=ms_queue_peek_last(f->inputs[0]))!=NULL) {
		if (ms_yuv_buf_init_from_mblk(&mainpic,main_im)==0){
			if (obj->vsize.width!=mainpic.w || obj->vsize.height!=mainpic.h){
				ms_message("Detected video resolution changed to %ix%i",mainpic.w,mainpic.h);
				if (obj->autofit && (mainpic.w>wsize.width || mainpic.h>wsize.height) ){
					QtDisplayResizeEvent event(
							(QEvent::Type)QtDisplayResizeEvent::DisplayResize,
							QSize(mainpic.w, mainpic.h));

					QApplication::sendEvent(obj->window.data(), &event);
					obj->mainImage = QImage(mainpic.w, mainpic.h, QImage::Format_RGB888);
				}
				//in all case repaint the background.
				obj->need_repaint=TRUE;
			}
			obj->vsize.width=mainpic.w;
			obj->vsize.height=mainpic.h;
		}
	}

	if (main_im!=NULL || local_im!=NULL || obj->need_repaint){
		ms_layout_compute(wsize,obj->vsize,obj->lsize,corner,scalefactor,&mainrect,&localrect);
		vsize.width=mainrect.w;
		vsize.height=mainrect.h;
		lsize.width=localrect.w;
		lsize.height=localrect.h;

		if (local_im!=NULL)
			yuv2rgb_process(&obj->locview,&localpic,lsize,!mblk_get_precious_flag(local_im));

		if (main_im!=NULL)
			yuv2rgb_process(&obj->mainview,&mainpic,vsize,obj->mirroring && !mblk_get_precious_flag(main_im));

		/*handle the case where local view is disabled*/
		if (corner==-1 && obj->locview.rgb!=NULL){
			yuv2rgb_uninit(&obj->locview);
		}
		if (obj->locview.rgb==NULL){
			 /*One layer: we can draw directly on the displayed surface*/
			localImagePtr = mainImagePtr;
			if (obj->need_repaint) {
				draw_background(*localImagePtr, wsize, mainrect, obj->background_color);
			}
		}else{
			/* in this case we need to stack several layers*/
			/*Create a second DC and bitmap to draw to a buffer that will be blitted to screen
			once all drawing is finished. This avoids some blinking while composing the image*/
			draw_background(*localImagePtr, wsize, mainrect, obj->background_color);
		}

		if (obj->need_repaint){
			repainted=TRUE;
			obj->need_repaint=FALSE;
		}
		if (main_im!=NULL || obj->locview.rgb!=NULL) {
			yuv2rgb_draw(&obj->mainview, *localImagePtr, mainrect.x,mainrect.y);
		}
		if (obj->locview.rgb!=NULL) {
			draw_local_view_frame(*localImagePtr,wsize,localrect);
			yuv2rgb_draw(&obj->locview, *localImagePtr,localrect.x,localrect.y);
		}
		if (mainImagePtr!=localImagePtr){
			if (main_im==NULL && !repainted){
				/* Blitting local rect only */
				QPainter painter(mainImagePtr);

				painter.drawImage(
						QRect(localrect.x-LOCAL_BORDER_SIZE, localrect.y-LOCAL_BORDER_SIZE, localrect.w+LOCAL_BORDER_SIZE,localrect.h+LOCAL_BORDER_SIZE),
						*localImagePtr);
			}else{
				/*Blitting the entire window */
				QPainter painter(mainImagePtr);

				painter.drawImage(
						QRect(0, 0, wsize.width, wsize.height),
						*localImagePtr);
			}
		}
		/*else using direct blitting to screen*/

		if (obj->mainview.rgb) {
			QtDisplayEvent event((QEvent::Type)QtDisplayEvent::Display, *mainImagePtr);
			QApplication::sendEvent(obj->window.data(), &event);
		}
	}

	end:

	if (f->inputs[0]!=NULL)
		ms_queue_flush(f->inputs[0]);
	if (f->inputs[1]!=NULL)
		ms_queue_flush(f->inputs[1]);
}

static MSFilterMethod methods[]={
	{	MS_FILTER_GET_VIDEO_SIZE			, get_vsize	},
	{	MS_FILTER_SET_VIDEO_SIZE			, set_vsize	},
	{	MS_VIDEO_DISPLAY_GET_NATIVE_WINDOW_ID, get_native_window_id },
	{	0	,NULL}
};

MSFilterDesc ms_qt_display_desc={
  MS_QT_DISPLAY_ID,
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
