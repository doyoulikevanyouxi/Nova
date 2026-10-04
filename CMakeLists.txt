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
  core/NetworkGenome.h core/Region.h
  core/MetabolicPool.h
  core/ValenceSystem.h
  core/CuriosityInterface.h
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

add_executable(test_oscillation
    test/test_oscillation.cpp
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

add_executable(test_lateral_inhibition
    test/test_lateral_inhibition.cpp
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

add_executable(test_competitive_learning
    test/test_competitive_learning.cpp
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

add_executable(test_multimodal_learning
    test/test_multimodal_learning.cpp
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
add_executable(test_regions
    test/test_regions.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/RandomUtils.h
)


add_executable(test_genome
    test/test_genome.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/RandomUtils.h
)

add_executable(test_signal_propagation
    test/test_signal_propagation.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/RandomUtils.h
)

add_executable(test_modulation
    test/test_modulation.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/RandomUtils.h
)

add_executable(test_metabolism
    test/test_metabolism.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/MetabolicPool.h
    core/RandomUtils.h
)

add_executable(test_valence
    test/test_valence.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/MetabolicPool.h
    core/ValenceSystem.h
    core/RandomUtils.h
)
add_executable(test_curiosity
    test/test_curiosity.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/MetabolicPool.h
    core/ValenceSystem.h
    core/CuriosityInterface.h
    core/RandomUtils.h
)

add_executable(test_body_sense
    test/test_body_sense.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/MetabolicPool.h
    core/ValenceSystem.h
    core/CuriosityInterface.h
    core/RandomUtils.h
)

add_executable(test_action
    test/test_action.cpp
    core/Network.h
    core/Neuron.h
    core/NeuronType.h
    core/DendriteBranch.h
    core/DendriteCluster.h
    core/Soma.h
    core/AxonInitialSegment.h
    core/Synapse.h
    core/SynapsePopulation.h
    core/Region.h
    core/NetworkGenome.h
    core/MetabolicPool.h
    core/ValenceSystem.h
    core/CuriosityInterface.h
    core/RandomUtils.h
)
target_link_libraries(Nova Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS Nova
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
