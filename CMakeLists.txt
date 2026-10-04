cmake_minimum_required(VERSION 3.16)

project(Nova LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(Nova
  main.cpp
  des.md
  core/AxonInitialSegment.h
  core/RandomUtils.h
  core/NeuronParameters.h
  core/NeuronType.h
  core/Synapse.h
  core/SynapsePopulation.h
  core/DendriteBranch.h
  core/DendriteCluster.h
  core/Soma.h
  core/Neuron.h
  core/Network.h
)

add_executable(test_ais
    test/test_ais.cpp
    core/AxonInitialSegment.h
    core/RandomUtils.h
    core/NeuronParameters.h
    core/NeuronType.h
    core/Synapse.h

)

add_executable(test_synapse
    test/test_synapse.cpp
    core/Synapse.h
    core/RandomUtils.h
    core/SynapsePopulation.h
)

add_executable(test_synapse_population
    test/test_synapse_population.cpp
    core/Synapse.h
    core/SynapsePopulation.h
    core/RandomUtils.h
)

add_executable(test_dendrite_branch
    test/test_dendrite_branch.cpp
    core/DendriteBranch.h
)

add_executable(test_dendrite_cluster
    test/test_dendrite_cluster.cpp
    core/DendriteCluster.h
    core/DendriteBranch.h
    core/NeuronType.h
)

add_executable(test_soma
    test/test_soma.cpp
    core/Soma.h
    core/NeuronType.h
    core/RandomUtils.h
)

add_executable(test_soma_ais
    test/test_soma_ais.cpp
    core/Soma.h
    core/AxonInitialSegment.h
    core/NeuronType.h
    core/RandomUtils.h
)

add_executable(test_neuron
    test/test_neuron.cpp
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/RandomUtils.h
)

add_executable(test_network
    test/test_network.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/RandomUtils.h
)


add_executable(test_synapse_current
    test/test_synapse_current.cpp
    core/Synapse.h
    core/SynapsePopulation.h
    core/RandomUtils.h
)

add_executable(test_stdp
    test/test_stdp.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/RandomUtils.h
)
add_executable(test_associative_learning
    test/test_associative_learning.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/RandomUtils.h
)
target_link_libraries(Nova Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS Nova
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
