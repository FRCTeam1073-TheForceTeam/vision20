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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-botvision
 *
 * The vision element does vision processing for the robot.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! vision ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "botvision.h"

GST_DEBUG_CATEGORY_STATIC (bot_vision_debug_category);
#define GST_CAT_DEFAULT bot_vision_debug_category

/* prototypes */


static void bot_vision_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void bot_vision_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void bot_vision_dispose (GObject * object);
static void bot_vision_finalize (GObject * object);

static gboolean bot_vision_start (GstBaseTransform * trans);
static gboolean bot_vision_stop (GstBaseTransform * trans);
static gboolean bot_vision_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn bot_vision_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn bot_vision_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

enum
{
  PROP_0
};

/* pad templates */

/* Add/remove formats you can handle */
#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ NV12, I420, Y444, Y42B, UYVY, RGBA }")

/* Add/remove formats you can handle */
#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ NV12, I420, Y444, Y42B, UYVY, RGBA }")


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (BotVision, bot_vision, GST_TYPE_VIDEO_FILTER,
    GST_DEBUG_CATEGORY_INIT (bot_vision_debug_category, "botvision", 0,
        "debug category for vision element"));

static void
bot_vision_class_init (BotVisionClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class =
      GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "FIXME Long name", "Generic", "Robot vision plugin",
      "FRC1073 <frc1073@frc1073.org>");

  gobject_class->set_property = bot_vision_set_property;
  gobject_class->get_property = bot_vision_get_property;
  gobject_class->dispose = bot_vision_dispose;
  gobject_class->finalize = bot_vision_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (bot_vision_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (bot_vision_stop);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (bot_vision_set_info);
  video_filter_class->transform_frame =
      GST_DEBUG_FUNCPTR (bot_vision_transform_frame);
  video_filter_class->transform_frame_ip =
      GST_DEBUG_FUNCPTR (bot_vision_transform_frame_ip);

}

static void
bot_vision_init (BotVision * vision)
{
}

void
bot_vision_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  BotVision *vision = BOT_VISION (object);

  GST_DEBUG_OBJECT (vision, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
bot_vision_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  BotVision *vision = BOT_VISION (object);

  GST_DEBUG_OBJECT (vision, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
bot_vision_dispose (GObject * object)
{
  BotVision *vision = BOT_VISION (object);

  GST_DEBUG_OBJECT (vision, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (bot_vision_parent_class)->dispose (object);
}

void
bot_vision_finalize (GObject * object)
{
  BotVision *vision = BOT_VISION (object);

  GST_DEBUG_OBJECT (vision, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (bot_vision_parent_class)->finalize (object);
}

static gboolean
bot_vision_start (GstBaseTransform * trans)
{
  BotVision *vision = BOT_VISION (trans);

  GST_DEBUG_OBJECT (vision, "start");

  return TRUE;
}

static gboolean
bot_vision_stop (GstBaseTransform * trans)
{
  BotVision *vision = BOT_VISION (trans);

  GST_DEBUG_OBJECT (vision, "stop");

  return TRUE;
}

static gboolean
bot_vision_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  BotVision *vision = BOT_VISION (filter);

  GST_DEBUG_OBJECT (vision, "set_info");

  return TRUE;
}

/* transform */
static GstFlowReturn
bot_vision_transform_frame (GstVideoFilter * filter, GstVideoFrame * inframe,
    GstVideoFrame * outframe)
{
  BotVision *vision = BOT_VISION (filter);

  GST_DEBUG_OBJECT (vision, "transform_frame");

  return GST_FLOW_OK;
}

static GstFlowReturn
bot_vision_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  BotVision *vision = BOT_VISION (filter);

  GST_DEBUG_OBJECT (vision, "transform_frame_ip");

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "botvision", GST_RANK_NONE,
      BOT_TYPE_VISION);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "bot_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "bot_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://frc1073.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vision,
    "Robot vision plugin",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
