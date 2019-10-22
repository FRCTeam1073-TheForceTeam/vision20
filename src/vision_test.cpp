#include <gstvision.h>
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

// Define signal handler for clean shutdown control.
void on_SIGINT(int signum);
bool done(false);

class TestProcessor: public gv::ImageProcessor {
public:
  
  virtual void process_image(gv::ImageProcessor::ImageBuffer& image){
    std::cerr << "P: " << image.width << ", " << image.height << std::endl;
  }
  
  virtual void render_image(ImageBuffer& image) {
    std::cerr << "R: " << image.width << ", " << image.height << std::endl;
  }
};


/// The pipeline wrapper object that runs the pipeline.
gv::Pipeline::Handle pipeline;

int main(int argc, char* argv[]) {

  std::cerr << "GSTVISION: Basic Test." << std::endl;

  gv::Pipeline::init(&argc, &argv);

  std::string pipeline_def =
    gv::build_nanocam_vision_def(60, 600000, "127.0.0.1", 5801);
  std::cerr << "PIPELINE: " << std::endl << pipeline_def << std::endl;

  pipeline = gv::Pipeline::create(pipeline_def);

  // Register interrupt handler before running pipeline.
  // connect shutdown signal callback for clean shutdown behavior:
  std::signal(SIGINT, on_SIGINT);

  pipeline->register_image_processor("vision",
				     std::make_shared<TestProcessor>());
  
  // Runs pipeline on internal threads...
  pipeline->run();

  // Run main loop doing other things if you want...
  while (not done) {
    std::cerr << "Main thread running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  pipeline->stop();

  return 0;
}

void on_SIGINT(int signum) {
  std::cerr << " Got SIGINT (" << signum << ") Stopping..." << std::endl;
  done = true;
}
