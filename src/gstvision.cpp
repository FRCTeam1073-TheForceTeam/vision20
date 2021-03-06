#include <gstvision.h>
#include <gst/gst.h>
#include <glib.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>

namespace gv {

  

  // Internal implementation class.
  class Pipeline::Impl {
  public:
    friend class Pipeline;
    
    Impl();
    ~Impl();

    void build(const std::string& pipeline_description);
    void run();
    void stop();

    static gboolean bus_callback(GstBus* bus,
				 GstMessage* message,
				 gpointer data);

    StringVector list_elements();

    GstElement* find_element(const std::string& name);

  protected:

    // Internal thread function:
    void thread_function();

    std::thread  _pipeline_thread;
    GstElement*  _pipeline;
    GMainLoop*   _main_loop;
    guint        _bus_watch_id;
  };
  

  void Pipeline::init(int* argc, char** argv[]) {
    guint major, minor, micro, nano;
    gst_init(argc, argv);
    gst_version(&major, &minor, &micro, &nano);
    std::cerr << "GSTREAMER: " << major << "." << minor << "."
	      << micro << "." << nano << std::endl;
  }
  
  Pipeline::Handle Pipeline::create(const std::string& pipeline_description) {
    Pipeline::Handle pipeline(new Pipeline());
    if (pipeline) {
      pipeline->build(pipeline_description);
    }
    return pipeline;
  }


  StringVector Pipeline::list_elements() const {
    return _impl->list_elements();
  }

  void Pipeline::run() {
    _impl->run();
  }

  void Pipeline::stop() {
    _impl->stop();
  }

  bool Pipeline::running() {
    /// TODO: test for this.
    return true;
  }


  Pipeline::Pipeline() : _impl(new Pipeline::Impl) {
    
  }

  void Pipeline::build(const std::string& pipeline_description) {
    _impl->build(pipeline_description);
  }



  Pipeline::Impl::Impl() {
    _pipeline = nullptr;
    _main_loop = g_main_loop_new(NULL, FALSE);
  }
  
  Pipeline::Impl::~Impl() {
    if (nullptr != _pipeline) {
      gst_object_unref(_pipeline);
    }
    g_source_remove(_bus_watch_id);
    g_main_loop_unref(_main_loop);
  }

  StringVector Pipeline::Impl::list_elements() {
    StringVector result;
    GstBin* pBin = GST_BIN(_pipeline);
    GstIterator* iterator = gst_bin_iterate_elements(pBin);
    if (nullptr != iterator) {

      GValue element = G_VALUE_INIT;
      while(gst_iterator_next(iterator, &element) == GST_ITERATOR_OK) {

	GstElement* pElement = GST_ELEMENT(g_value_get_object(&element));

	char *name = gst_element_get_name(pElement);
	if (nullptr != name) {
	  result.push_back(std::string(name));
	}
	g_free(name);

	g_value_unset(&element);
      }

      gst_iterator_free(iterator);
    }
    return result;
  }

  void Pipeline::Impl::build(const std::string& pipeline_description) {
    GstParseContext *pcontext = gst_parse_context_new();
    GError* error = nullptr;
    GstParseFlags parse_flags = GST_PARSE_FLAG_FATAL_ERRORS;
    _pipeline = gst_parse_launch_full(pipeline_description.c_str(),
				      pcontext,
				      parse_flags,
				      &error);

    if (nullptr == _pipeline) {
      gst_parse_context_free(pcontext);
      throw std::runtime_error(error->message);
    }

    gst_parse_context_free(pcontext);

    // Grab bus setup:
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
    _bus_watch_id = gst_bus_add_watch(bus, bus_callback, this);
    gst_object_unref(bus);
  }
  
  void Pipeline::Impl::run() {
    if (nullptr != _pipeline and nullptr != _main_loop) {
      _pipeline_thread = std::thread([this](){this->thread_function();});
    }
  }

   
  void Pipeline::Impl::stop() {
    g_main_loop_quit(_main_loop);
    if (_pipeline_thread.joinable()) {
      _pipeline_thread.join();
    }
  }

  void Pipeline::Impl::thread_function() {
    gst_element_set_state(_pipeline, GST_STATE_PLAYING);
    g_main_loop_run(_main_loop);
    gst_element_set_state(_pipeline, GST_STATE_NULL);
  }

  GstElement* Pipeline::Impl::find_element(const std::string& name) {
    GstBin* pBin = GST_BIN(_pipeline);
    return gst_bin_get_by_name(pBin, name.c_str());
  }

  gboolean Pipeline::Impl::bus_callback(GstBus* bus,
					GstMessage* message,
					gpointer data) {
    
    Pipeline::Impl* pImpl = static_cast<Pipeline::Impl*>(data);

    gchar  *debug = nullptr;
    GError *error = nullptr;

    switch (GST_MESSAGE_TYPE (message)) {


    case GST_MESSAGE_WARNING:
      gst_message_parse_warning(message, &error, &debug);
      g_free (debug);
      std::cerr << "BUS WARNING: " << error->message << std::endl;
      g_error_free (error);
      break;
    case GST_MESSAGE_INFO:
      gst_message_parse_info(message, &error, &debug);
      g_free (debug);
      std::cerr << "BUS INFO: " << error->message << std::endl;
      g_error_free (error);
      break;
      
    case GST_MESSAGE_PROPERTY_NOTIFY:
      std::cerr << "BUS PROPERTY CHANGE:" << std::endl;
      break;

    case GST_MESSAGE_EOS:
      std::cerr << "BUS MESSAGE: " << std::endl;
      g_print ("End of stream\n");
      pImpl->stop();
      break;

    case GST_MESSAGE_ERROR: {
      gst_message_parse_error (message, &error, &debug);
      g_free (debug);
      
      std::cerr << "BUS MESSAGE: " << std::endl;
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      pImpl->stop();
      break;
    }
    default:
      break;
    }

    return TRUE;
  }


  std::string build_nanocam_compression_def(int fps, int bps,
					    const std::string& to_host,
					    int to_port) {

    std::ostringstream def;
    def << "nvarguscamerasrc name=camera do-timestamp=true ! ";
    def << "video/x-raw(memory:NVMM),format=(string)NV12,width=(int)1280,height=(int)720,framerate=" << fps << "/1 ! ";
    def << "nvvidconv name=converter flip-method=0 ! ";
    def << "video/x-raw(memory:NVMM),width=(int)640,height=(int)360,format=(string)NV12 ! ";
    def << "omxh264enc name=encoder control-rate=2 bitrate=" << bps << " profile=1 preset-level=1 ! ";
    def << "video/x-h264,framerate=" << fps << "/1,stream-format=(string)byte-stream ! ";
    def << "h264parse name=parser ! rtph264pay name=payloader config-interval=1 ! udpsink name=udpsink host=" << to_host << " port=" << to_port;

    return def.str();
  }

  std::string build_webcam_compression_def(const std::string& cam_dev,
					   int fps, int bps,
					   const std::string& to_host,
					   int to_port) {
    std::ostringstream def;
    def << "v4l2src name=camera device=" << cam_dev << " ! ";
    def << "video/x-raw,format=(string)YUY2,width=(int)320,height=(int)240,framerate=" << fps << "/1 ! ";
    def << "nvvidconv name=converter flip-method=0 ! ";
    def << "video/x-raw(memory:NVMM),width=(int)320,height=(int)240,format=(string)NV12 ! ";
    def << "omxh264enc name=encoder control-rate=2 bitrate=" << bps << " profile=1 preset-level=1 ! ";
    def << "video/x-h264,framerate=" << fps << "/1, stream-format=(string)byte-stream ! ";
    def << "h264parse name=parser ! rtph264pay name=payloader config-interval=1 ! udpsink name=udpsink host=" << to_host << " port=" << to_port;

    return def.str();
  }

    std::string build_nanocam_vision_def(int fps, int bps,
					 const std::string& to_host,
					 int to_port) {

    std::ostringstream def;
    def << "nvarguscamerasrc name=camera do-timestamp=true ! ";
    def << "video/x-raw(memory:NVMM),format=(string)NV12,width=(int)1280,height=(int)720,framerate=" << fps << "/1 ! ";
    def << "nvvidconv name=converter flip-method=0 ! ";
    def << "video/x-raw(memory:NVMM),width=(int)640,height=(int)360,format=(string)NV12 ! ";
    def << "nvtee name=tee ! queue name=compression_queue !";
    def << "omxh264enc name=encoder control-rate=2 bitrate=" << bps << " profile=1 preset-level=1 ! ";
    def << "video/x-h264,framerate=" << fps << "/1,stream-format=(string)byte-stream ! ";
    def << "h264parse name=parser ! rtph264pay name=payloader config-interval=1 ! udpsink name=udpsink host=" << to_host << " port=" << to_port;
    def << " tee. ! queue name=vision_queue ! appsink name=vision_sink";

    return def.str();
  }


}
