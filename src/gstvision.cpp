#include <gstvision.h>
#include <gst/gst.h>
#include <gst/app/app.h>
#include <glib.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <map>

namespace gv {


  ImageProcessor::ImageBuffer::ImageBuffer() :
    width(0),
    height(0),
    data(0),
    data_size(0) {
  }

  // Internal implementation class.
  class Pipeline::Impl {
  public:
    friend class Pipeline;

    struct ProcessorRegistration {
      ImageProcessor::Handle processor;
      GstAppSink* app_sink;
      GstAppSrc* app_source;
    };

    using ProcessorMap = std::map<std::string, ProcessorRegistration>;
    
    
    Impl();
    ~Impl();

    StringVector list_elements();
    
    void build(const std::string& pipeline_description);
    
    void run();
    
    void stop();

    void set_property(const std::string& element, const std::string& property,
		      const std::string& value);

    void set_property(const std::string& element, const std::string& property,
		      int value);

    void set_property(const std::string& element, const std::string& property,
		      int numerator, int denominator);

    void register_image_processor(const std::string& name,
    				  ImageProcessor::Handle processor);

    // Callback from pipeline bus events:
    static gboolean bus_callback(GstBus* bus,
				 GstMessage* message,
				 gpointer data);


    // Callback from appsink on samples:
    static GstFlowReturn new_sample(GstAppSink* sink, gpointer data);
    static GstFlowReturn new_preroll(GstAppSink* sink, gpointer data);



    GstElement* find_element(const std::string& name);

  protected:

    // Internal thread function:
    void thread_function();

    std::thread  _pipeline_thread;
    GstElement*  _pipeline;
    GMainLoop*   _main_loop;
    guint        _bus_watch_id;
    ProcessorMap _image_processors;
  };

  class CallbackInfo {
  public:
    CallbackInfo(const std::string& name_, Pipeline::Impl* pipe_):
      name(name_)
      ,pipeline(pipe_) {};

        
    std::string     name;
    Pipeline::Impl *pipeline;
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

  void Pipeline::set_property(const std::string& element,
			      const std::string& property,
			      const std::string& value) {
    _impl->set_property(element, property, value);
  }

  void Pipeline::set_property(const std::string& element,
			      const std::string& property,
			      int value) {
    _impl->set_property(element, property, value);
  }

  void Pipeline::set_property(const std::string& element,
			      const std::string& property,
			      int numerator, int denominator) {
    _impl->set_property(element, property, numerator, denominator);
  }

  Pipeline::Pipeline() : _impl(new Pipeline::Impl) {
    
  }

  void Pipeline::build(const std::string& pipeline_description) {
    _impl->build(pipeline_description);
  }


  void Pipeline::register_image_processor(const std::string& name,
					  ImageProcessor::Handle processor) {
    _impl->register_image_processor(name, processor);
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


  void Pipeline::Impl::set_property(const std::string& element,
				    const std::string& property,
				    const std::string& value) {
    GstElement* el = find_element(element);
    if (nullptr != el) {
      g_object_set(G_OBJECT(el),property.c_str(), value.c_str(), nullptr);
    } else {
      throw std::runtime_error("Element not found.");
    }
  }

  void Pipeline::Impl::set_property(const std::string& element,
				    const std::string& property,
				    int value) {
    GstElement* el = find_element(element);
    if (nullptr != el) {
      g_object_set(G_OBJECT(el),property.c_str(), value, nullptr);
    } else {
      throw std::runtime_error("Element not found.");
    }
  }

  void Pipeline::Impl::set_property(const std::string& element,
				    const std::string& property,
				    int numerator, int denominator) {
    // TODO: Unimplemented.
    throw std::logic_error("Unimplemented method.");
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


  GstFlowReturn Pipeline::Impl::new_preroll(GstAppSink* appsink,
					    gpointer data) {
    return GST_FLOW_OK;
  }

  /// Route callback to C++ client:
  GstFlowReturn Pipeline::Impl::new_sample(GstAppSink* appsink,
					   gpointer data) {
    // Get caps and frame
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstCaps *caps = gst_sample_get_caps(sample);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const int width =
      g_value_get_int(gst_structure_get_value(structure, "width"));
    const int height =
      g_value_get_int(gst_structure_get_value(structure, "height"));


    CallbackInfo* cb_info = static_cast<CallbackInfo*>(data);
    

    // Show caps on first frame
    //    if(!framecount) {
    //        g_print("caps: %s\n", gst_caps_to_string(caps));
    //    }
    //    framecount++;

    // Get frame data
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    gv::ImageProcessor::ImageBuffer image_buffer;

    image_buffer.width = width;
    image_buffer.height = height;
    image_buffer.data_size = map.size;
    image_buffer.data = map.data;

    auto processor = cb_info->pipeline->_image_processors.find(cb_info->name);

    if (processor != cb_info->pipeline->_image_processors.end()) {
      processor->second.processor->process_image(image_buffer);
    }
    
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
  }
  


  void Pipeline::Impl::register_image_processor(const std::string& name,
						ImageProcessor::Handle processor) {
    ProcessorRegistration pr;
    pr.processor = processor;
    pr.app_sink = (GstAppSink*) find_element(name+"_sink");
    pr.app_source = (GstAppSrc*) find_element(name+"_source");
    _image_processors[name] = pr;

    // Now set up callbacks for this registration:
    if (pr.app_sink != nullptr) {
      gst_app_sink_set_emit_signals(pr.app_sink, TRUE);
      gst_app_sink_set_drop(pr.app_sink, true);
      gst_app_sink_set_max_buffers(pr.app_sink, 1);
      GstAppSinkCallbacks callbacks = {nullptr, Pipeline::Impl::new_preroll,
				       Pipeline::Impl::new_sample};
      CallbackInfo* pCallbackInfo = new CallbackInfo(name, this);
      gst_app_sink_set_callbacks(pr.app_sink, & callbacks,
				 pCallbackInfo, nullptr);
    }
    
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
