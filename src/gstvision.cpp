#include <gstvision.h>
#include <gst/gst.h>
#include <glib.h>
#include <iostream>
#include <thread>

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
    StringVector result;
    return result;
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

  gboolean Pipeline::Impl::bus_callback(GstBus* bus,
					GstMessage* message,
					gpointer data) {
    
    Pipeline::Impl* pImpl = static_cast<Pipeline::Impl*>(data);
    
    switch (GST_MESSAGE_TYPE (message)) {

    case GST_MESSAGE_EOS:
      std::cerr << "BUS MESSAGE: " << std::endl;
      g_print ("End of stream\n");
      pImpl->stop();
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

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

}
