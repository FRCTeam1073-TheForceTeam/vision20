#include <gstvision.h>
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

// Define signal handler for clean shutdown control.
void on_SIGINT(int signum);
bool done(false);

/// Build the pipeline string we want to send:
std::string build_pipeline_def();


/// The pipeline wrapper object that runs the pipeline.
gv::Pipeline::Handle pipeline;

int main(int argc, char* argv[]) {

  std::cerr << "GSTVISION: Basic Test." << std::endl;

  gv::Pipeline::init(&argc, &argv);

  std::string pipeline_def =
    gv::build_nanocam_compression_def(60, 600000, "127.0.0.1", 5801);
  std::cerr << "PIPELINE: " << std::endl << pipeline_def << std::endl;
  pipeline = gv::Pipeline::create(pipeline_def);

  // List the elements:
  gv::StringVector elements = pipeline->list_elements();

  std::cerr << "ELEMENTS: ";
  for (const auto& element: elements) {
    std::cerr << element << ", ";
  }
  std::cerr << std::endl;
  
  // Register interrupt handler before running pipeline.
  // connect shutdown signal callback for clean shutdown behavior:
  std::signal(SIGINT, on_SIGINT);

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

std::string build_pipeline_def() {
  std::string pipeline_def;

  pipeline_def = "videotestsrc ! autovideosink";

  return pipeline_def;
}

void on_SIGINT(int signum) {
  std::cerr << " Got SIGINT (" << signum << ") Stopping..." << std::endl;
  done = true;
}
