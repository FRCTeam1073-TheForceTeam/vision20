
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace gv {

  using StringVector = std::vector<std::string>;


  /**\ brief The ImageProcessor defines an abstract class interface
   * for processing and rendering images. This class is designed
   * to work in conjunction with a named appsrc and appsink in your
   * gstreamer pipeline.
   *
   */
  class ImageProcessor {
  public:
    using Handle = std::shared_ptr<ImageProcessor>;

    class ImageBuffer {
    public:

      ImageBuffer();
      
      size_t         width;
      size_t         height;
      unsigned char *data;
      size_t         data_size;
    };

    ImageProcessor() = default;
    virtual ~ImageProcessor() = default;

    virtual void process_image(ImageBuffer& image) = 0;
    virtual void render_image(ImageBuffer& image) = 0;
    
  protected:

  };


  /** \brief The Pipeline class wraps a gstreamer-1.0 pipeline created
   * from the gstreamer-1.0 string definition and manages that
   * pipeline for you. It isolates you from most of the gstreamer
   * details but allows you to attach callbacks to named pipeline
   * elements and perorm other operations on named pipeline elements.
   *
   */
  class Pipeline {
  public:
    using Handle = std::shared_ptr<Pipeline>;

    /// Gstreamer init function.
    static void init(int* argc, char** argv[]);
    

    /// Create function sets up the Pipeline object.
    static Handle create(const std::string& pipeline_description);


    /** \brief List the names of elements in this pipeline.
     */
    StringVector list_elements() const;

    /// Start the pipeline running and don't return until its done.
    void run();

    /// Stop the pipeline, will wait for internal threads, etc.
    void stop();

    /// Is the pipeline running.
    bool running();

    /// Set a string property on a named element.
    void set_property(const std::string& element, const std::string& property,
		      const std::string& value);

    /// Set an int property on a named element.
    void set_property(const std::string& element, const std::string& property,
		      int value);

    /// Set a ratio property on a named element.
    void set_property(const std::string& element, const std::string& property,
		      int numerator, int denominator=1);


    void register_image_processor(const std::string& name,
				  ImageProcessor::Handle processor);

    /// Default destructor.
    virtual ~Pipeline() = default;

    class Impl;
    std::unique_ptr<Impl> _impl;
    
  private:
    Pipeline();

    /** \brief Build the pipeline from the given description.
     */
    void build(const std::string& pipeline_description);

  };

  /** \brief Builds standard nano camera compression pipeline definition.
   *
   */
  std::string build_nanocam_compression_def(int fps, int bps,
					    const std::string& to_host,
					    int to_port);
  
  /** \brief Builds standard web camera compression pipeline definition.
   *
   */
  std::string build_webcam_compression_def(const std::string& cam_dev,
					   int fps, int bps,
					   const std::string& to_host,
					   int to_port);

  std::string build_nanocam_vision_def(int fps, int bps,
				       const std::string& to_host,
				       int to_port);
  
}
