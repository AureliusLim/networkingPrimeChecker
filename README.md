# Distributed Prime Checker
This project has 3 processes which are: master server, slave process, and client process. The client sends a request to initiate the counting of prime numbers until the specified limit. After the master server receives the request, the task is equally distributed between the master server and slave process effectively increasing the performance. 

## Contributors
- Caoile, Sean
- Lim, Aurelius
- Tan, Gavin
- Yongco, Denzel

  ## Features
  -**Multithreaded Prime Checker**: Leverages multiple threads for the prime checker to perform calculations in parallel
  -**Distributed Computing**: Leverages a slave process to lessen load and perform the calculation in parallel with the slave
  -**Performance Measurement**: Calculates and displays the time taken to calculate the count of prime numbers.

  ## Requirement
  -A C++ Compiler (e.g., G++, Clang)
  -C++11 Standard Library
