cmake_minimum_required(VERSION 3.20)
project(newxim CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(deps/systemc)
add_subdirectory(deps/yaml-cpp)

include_directories("src/")
set(
  SRCS
  src/Data/Flit.cpp
  src/Main.cpp
  src/Configuration/Graph/CirculantGraph.cpp
  src/Configuration/Graph/Graph.cpp
  src/Configuration/Graph/MeshGraph.cpp
  src/Configuration/Graph/TorusGraph.cpp
  src/Configuration/Graph/TreeGraph.cpp
  src/Configuration/RoutingTable.cpp
  src/Configuration/Factory.cpp
  src/Configuration/Configuration.cpp
  src/Configuration/TrafficManagers/HotspotTrafficManager.cpp
  src/Configuration/TrafficManagers/RandomTrafficManager.cpp
  src/Configuration/TrafficManagers/TableTrafficManager.cpp
  src/Hardware/Processor.cpp
  src/Hardware/ProcessorQueue.cpp
  src/Hardware/Router.cpp
  src/Hardware/Buffer.cpp
  src/Hardware/Network.cpp
  src/Hardware/SimulationTimer.cpp
  src/Hardware/Relay.cpp
  src/Hardware/ReservationTable.cpp
  src/Metrics/ProgressBar.cpp
  src/Metrics/Stats.cpp
  src/Metrics/GlobalStats.cpp
  src/Metrics/FlitTracer.cpp
)

add_executable(newxim ${SRCS})
target_link_libraries(
  newxim
  SystemC::systemc
  yaml-cpp::yaml-cpp
)
