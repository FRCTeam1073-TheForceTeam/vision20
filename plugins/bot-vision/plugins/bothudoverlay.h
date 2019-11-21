/* GStreamer
 * Copyright (C) 2019 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _BOT_HUDOVERLAY_H_
#define _BOT_HUDOVERLAY_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define BOT_TYPE_HUDOVERLAY   (bot_hudoverlay_get_type())
#define BOT_HUDOVERLAY(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),BOT_TYPE_HUDOVERLAY,BotHudoverlay))
#define BOT_HUDOVERLAY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),BOT_TYPE_HUDOVERLAY,BotHudoverlayClass))
#define BOT_IS_HUDOVERLAY(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),BOT_TYPE_HUDOVERLAY))
#define BOT_IS_HUDOVERLAY_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),BOT_TYPE_HUDOVERLAY))

typedef struct _BotHudoverlay BotHudoverlay;
typedef struct _BotHudoverlayClass BotHudoverlayClass;

struct _BotHudoverlay
{
  GstVideoFilter base_hudoverlay;

};

struct _BotHudoverlayClass
{
  GstVideoFilterClass base_hudoverlay_class;
};

GType bot_hudoverlay_get_type (void);

G_END_DECLS

#endif
