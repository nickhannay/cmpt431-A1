#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>

#define sqr(x) ((x) * (x))
#define DEFAULT_NUMBER_OF_POINTS "12345678"
uint total_points_inside = 0;
std::mutex mu;


uint c_const = (uint)RAND_MAX + (uint)1;
inline double get_random_coordinate(uint *random_seed) {
  return ((double)rand_r(random_seed)) / c_const;
}

void get_points_in_circle(uint n, uint random_seed, uint id) {
  timer serial_timer;
  double time = 0.0;

  serial_timer.start();
  uint circle_count = 0;
  double x_coord, y_coord;
  for (uint i = 0; i < n; i++) {
    x_coord = (2.0 * get_random_coordinate(&random_seed)) - 1.0;
    y_coord = (2.0 * get_random_coordinate(&random_seed)) - 1.0;
    if ((sqr(x_coord) + sqr(y_coord)) <= 1.0)
      circle_count++;
  }
  time = serial_timer.stop();

  mu.lock();
  std::cout << id <<", "<< n <<", " << circle_count <<", " << std::setprecision(TIME_PRECISION) 
            <<time << "\n";
  total_points_inside += circle_count;
  mu.unlock();
  
}

void piCalculation(uint n, uint threads) {
  timer serial_timer;
  double time_taken = 0.0;
  uint random_seed = 1;

  serial_timer.start();
  // -------------------------------------------------------------------
  uint pts_thread = n/threads ; 
  uint remainder = n % threads;

  std::vector<std::thread> thread_vector;
  std::cout << "thread_id, points_generated, circle_points, time_taken\n";
  for (int i =0; i<threads; i++){
    // evenly distribute workload amongst threads
    if ( i< remainder){
      thread_vector.push_back(std::thread 
                             (get_points_in_circle, pts_thread + 1, random_seed+i, i));
    }
    else{
      thread_vector.push_back(std::thread 
                             (get_points_in_circle, pts_thread, random_seed+i, i));
    }
    
  }
  // join threads back with main thread
  for (auto& th : thread_vector){
    th.join();
  }
  
  double pi_value = 4.0 * (double)total_points_inside / (double)n;
  // -------------------------------------------------------------------
  time_taken = serial_timer.stop();

  // Print the overall statistics
  std::cout << "Total points generated : " << n << "\n";
  std::cout << "Total points in circle : " << total_points_inside << "\n";
  std::cout << "Result : " << std::setprecision(VAL_PRECISION) << pi_value
            << "\n";
  std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION)
            << time_taken << "\n";
}

int main(int argc, char *argv[]) {
  // Initialize command line arguments
  cxxopts::Options options("pi_calculation",
                           "Calculate pi using serial and parallel execution");
  options.add_options(
      "custom",
      {
          {"nPoints", "Number of points",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_POINTS)},
          {"nWorkers", "Number of workers",
           cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
      });

  auto cl_options = options.parse(argc, argv);
  uint n_points = cl_options["nPoints"].as<uint>();
  uint n_workers = cl_options["nWorkers"].as<uint>();
  std::cout << std::fixed;
  std::cout << "Number of points : " << n_points << "\n";
  std::cout << "Number of workers : " << n_workers << "\n";

  piCalculation(n_points, n_workers);

  return 0;
}
