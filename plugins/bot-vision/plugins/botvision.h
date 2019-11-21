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

#ifndef _BOT_VISION_H_
#define _BOT_VISION_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define BOT_TYPE_VISION   (bot_vision_get_type())
#define BOT_VISION(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),BOT_TYPE_VISION,BotVision))
#define BOT_VISION_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),BOT_TYPE_VISION,BotVisionClass))
#define BOT_IS_VISION(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),BOT_TYPE_VISION))
#define BOT_IS_VISION_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),BOT_TYPE_VISION))

typedef struct _BotVision BotVision;
typedef struct _BotVisionClass BotVisionClass;

struct _BotVision
{
  GstVideoFilter base_vision;

};

struct _BotVisionClass
{
  GstVideoFilterClass base_vision_class;
};

GType bot_vision_get_type (void);

G_END_DECLS

#endif
