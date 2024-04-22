#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/queue.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/traffic-control-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/v4ping-helper.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include <unordered_set> 

#include <fstream> 
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono> 
#include <thread> 


using namespace ns3;
NS_LOG_COMPONENT_DEFINE("networkSimulation");

/**
 * Structure to store configuration parameters read from a text file.
 *
 * This structure holds various configuration parameters like traffic type, 
 * data rates, packet sizes, drop thresholds, and more, which are used in the ns-3 simulation.
 */
struct ConfigParameters {
  std::string traffic_type;
  std::string csma_data_rate;
  int csma_delay;
  uint32_t nclient, nserver;
  double client_lambda;
  std::string client_data_rate;
  int packet_size; 
  double sojourn1_drop_thresholdi, sojourn1_drop_thresholdii, sojourn1_drop_thresholdiii, sojourn1_packet_interval;
  double sojourn2_drop_thresholdi, sojourn2_drop_thresholdii, sojourn2_drop_thresholdiii, sojourn2_packet_interval;
  double sojourn3_drop_thresholdi, sojourn3_drop_thresholdii, sojourn3_drop_thresholdiii, sojourn3_packet_interval;
  double sojourn1_drop_duration, sojourn1_drop_rate, sojourn2_drop_duration, sojourn2_drop_rate, sojourn3_drop_duration, sojourn3_drop_rate, sojourn4_drop_duration, sojourn4_drop_rate;
  double buffer1_drop_thresholdi, buffer1_drop_thresholdii, buffer1_drop_thresholdiii, buffer1_packet_interval;
  double buffer2_drop_thresholdi, buffer2_drop_thresholdii, buffer2_drop_thresholdiii, buffer2_packet_interval;
  double buffer3_drop_thresholdi, buffer3_drop_thresholdii, buffer3_drop_thresholdiii, buffer3_packet_interval;
  double buffer1_drop_duration, buffer1_drop_rate, buffer2_drop_duration, buffer2_drop_rate, buffer3_drop_duration, buffer3_drop_rate, buffer4_drop_duration, buffer4_drop_rate;
  double sojourn1_drop_threshold, sojourn2_drop_threshold, sojourn3_drop_threshold, sojourn4_drop_threshold, buffer1_drop_threshold, buffer2_drop_threshold, buffer3_drop_threshold, buffer4_drop_threshold;

};

/**
 * Loads configuration parameters from a text file into a vector of ConfigParameters structures.
 *
 * This function reads a text file where each line contains key-value pairs separated by an equals sign. 
 * It then populates a vector of ConfigParameters structures. Each set of key-value pairs 
 * from the file fills one ConfigParameters structure.
 * 
 * @param filename Name of the text file containing the configuration parameters.
 * @param paramsSets Vector to store the read configuration parameters sets.
 *
 * @return Boolean indicating the success or failure of the operation.
 *
 */
bool loadParametersFromFile(const std::string & filename, std::vector < ConfigParameters > & paramsSets) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return false;
  }

  std::string line;
  ConfigParameters params;
  while (std::getline(file, line)) {
    if (line.empty()) { //reached the end of a configuration set.
      paramsSets.push_back(params);
      params = ConfigParameters(); // Reset params for next set
      continue;
    }
    std::istringstream is_line(line);
    std::string key;
    if (std::getline(is_line, key, '=')) {
      std::string value;
      if (std::getline(is_line, value)) {
        key.erase(key.find_last_not_of(" \n\r\t") + 1);
        value.erase(0, value.find_first_not_of(" \n\r\t"));

        if (key == "traffic_type") params.traffic_type = value;
        else if (key == "csma_data_rate") params.csma_data_rate = value;
        else if (key == "csma_delay") params.csma_delay = std::stoi(value);
        else if (key == "nclient") params.nclient = std::stoi(value);
        else if (key == "nserver") params.nserver = std::stoi(value);
        else if (key == "client_lambda") params.client_lambda = std::stod(value);
        else if (key == "client_data_rate") params.client_data_rate = value;
        else if (key == "packet_size") params.packet_size = std::stoi(value);
        else if (key == "sojourn1_drop_threshold") params.sojourn1_drop_threshold = std::stod(value);
        else if (key == "sojourn2_drop_threshold") params.sojourn2_drop_threshold = std::stod(value);
        else if (key == "sojourn3_drop_threshold") params.sojourn3_drop_threshold = std::stod(value);
        else if (key == "sojourn4_drop_threshold") params.sojourn4_drop_threshold = std::stod(value);
        else if (key == "buffer1_drop_threshold") params.buffer1_drop_threshold = std::stod(value);
        else if (key == "buffer2_drop_threshold") params.buffer2_drop_threshold = std::stod(value);
        else if (key == "buffer3_drop_threshold") params.buffer3_drop_threshold = std::stod(value);
        else if (key == "buffer4_drop_threshold") params.buffer4_drop_threshold = std::stod(value);
        else if (key == "sojourn1_drop_thresholdi") params.sojourn1_drop_thresholdi = std::stod(value);
        else if (key == "sojourn1_drop_thresholdii") params.sojourn1_drop_thresholdii = std::stod(value);
        else if (key == "sojourn1_drop_thresholdiii") params.sojourn1_drop_thresholdiii = std::stod(value);
        else if (key == "sojourn1_packet_interval") params.sojourn1_packet_interval = std::stod(value);
        else if (key == "sojourn2_drop_thresholdi") params.sojourn2_drop_thresholdi = std::stod(value);
        else if (key == "sojourn2_drop_thresholdii") params.sojourn2_drop_thresholdii = std::stod(value);
        else if (key == "sojourn2_drop_thresholdiii") params.sojourn2_drop_thresholdiii = std::stod(value);
        else if (key == "sojourn2_packet_interval") params.sojourn2_packet_interval = std::stod(value);
        else if (key == "sojourn3_drop_thresholdi") params.sojourn3_drop_thresholdi = std::stod(value);
        else if (key == "sojourn3_drop_thresholdii") params.sojourn3_drop_thresholdii = std::stod(value);
        else if (key == "sojourn3_drop_thresholdiii") params.sojourn3_drop_thresholdiii = std::stod(value);
        else if (key == "sojourn3_packet_interval") params.sojourn3_packet_interval = std::stod(value);
        else if (key == "sojourn1_drop_duration") params.sojourn1_drop_duration = std::stod(value);
        else if (key == "sojourn1_drop_rate") params.sojourn1_drop_rate = std::stod(value);
        else if (key == "sojourn2_drop_duration") params.sojourn2_drop_duration = std::stod(value);
        else if (key == "sojourn2_drop_rate") params.sojourn2_drop_rate = std::stod(value);
        else if (key == "sojourn3_drop_duration") params.sojourn3_drop_duration = std::stod(value);
        else if (key == "sojourn3_drop_rate") params.sojourn3_drop_rate = std::stod(value);
        else if (key == "sojourn4_drop_duration") params.sojourn4_drop_duration = std::stod(value);
        else if (key == "sojourn4_drop_rate") params.sojourn4_drop_rate = std::stod(value);
        else if (key == "buffer1_drop_thresholdi") params.buffer1_drop_thresholdi = std::stod(value);
        else if (key == "buffer1_drop_thresholdii") params.buffer1_drop_thresholdii = std::stod(value);
        else if (key == "buffer1_drop_thresholdiii") params.buffer1_drop_thresholdiii = std::stod(value);
        else if (key == "buffer1_packet_interval") params.buffer1_packet_interval = std::stod(value);
        else if (key == "buffer2_drop_thresholdi") params.buffer2_drop_thresholdi = std::stod(value);
        else if (key == "buffer2_drop_thresholdii") params.buffer2_drop_thresholdii = std::stod(value);
        else if (key == "buffer2_drop_thresholdiii") params.buffer2_drop_thresholdiii = std::stod(value);
        else if (key == "buffer2_packet_interval") params.buffer2_packet_interval = std::stod(value);
        else if (key == "buffer3_drop_thresholdi") params.buffer3_drop_thresholdi = std::stod(value);
        else if (key == "buffer3_drop_thresholdii") params.buffer3_drop_thresholdii = std::stod(value);
        else if (key == "buffer3_drop_thresholdiii") params.buffer3_drop_thresholdiii = std::stod(value);
        else if (key == "buffer3_packet_interval") params.buffer3_packet_interval = std::stod(value);
        else if (key == "buffer1_drop_duration") params.buffer1_drop_duration = std::stod(value);
        else if (key == "buffer1_drop_rate") params.buffer1_drop_rate = std::stod(value);
        else if (key == "buffer2_drop_duration") params.buffer2_drop_duration = std::stod(value);
        else if (key == "buffer2_drop_rate") params.buffer2_drop_rate = std::stod(value);
        else if (key == "buffer3_drop_duration") params.buffer3_drop_duration = std::stod(value);
        else if (key == "buffer3_drop_rate") params.buffer3_drop_rate = std::stod(value);
        else if (key == "buffer4_drop_duration") params.buffer4_drop_duration = std::stod(value);
        else if (key == "buffer4_drop_rate") params.buffer4_drop_rate = std::stod(value);
      }
    }
  }
  paramsSets.push_back(params);

  file.close();
  return true;
}

double simulationTime = 61.0;


/**
 * Collects, calculates, and logs statistics for the network simulation flows.
 *
 * This function retrieves the current flow statistics from the FlowMonitor
 * calculates delay, throughput, PLR and FCT for each flow. 
 * Then, it computes the average values of these metrics for all flows and log the deatils into a file.
 * This function is scheduled to run every 0.1 seconds to continuously collect and log flow statistics during the simulation.
 *
 * @param monitor Pointer to the FlowMonitor object.
 * @param classifier Pointer to the Ipv4FlowClassifier object
 * @param trafficStatsFile Shared pointer to ofstream object for logging the output
 */
std::unordered_set<FlowId> completedFlows;

void CollectData(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier, std::shared_ptr<std::ofstream> trafficStatsFile) {
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    double totalDelay = 0.0;
    double totalPLR = 0.0;
    double totalThroughput = 0.0;
    double totalFct = 0.0;
    int numFlows = 0;


    for (const auto &iter: stats) {
        FlowId id = iter.first;
    
        if (iter.second.txPackets == iter.second.rxPackets) {
        		completedFlows.insert(id);
        }

        // Ignore flows with no received packets
        if (iter.second.rxPackets == 0) continue;
				double delay = iter.second.delaySum.GetSeconds() / iter.second.rxPackets * 1000;
    		double packetLossRatio = static_cast < double > (iter.second.lostPackets) / iter.second.txPackets;
        // Calculate individual throughput
        double throughput = iter.second.rxBytes * 8.0 / (iter.second.timeLastRxPacket - iter.second.timeFirstTxPacket).GetSeconds() / 1000;
        // Calculate individual FCT
        double fct = (iter.second.timeLastRxPacket - iter.second.timeFirstTxPacket).GetSeconds();
				
				totalDelay += delay;
				totalPLR += packetLossRatio;
        totalThroughput += throughput;
        totalFct += fct;
        numFlows++;
    }

    // Calculate average throughput and FCT
    double avgDelay = (numFlows > 0) ? totalDelay / numFlows : 0;
    double avgPLR = (numFlows > 0) ? totalPLR / numFlows : 0;
    double avgThroughput = (numFlows > 0) ? totalThroughput / numFlows : 0;
    double avgFct = (numFlows > 0) ? totalFct / numFlows : 0;
    if (numFlows != 0){
        /*std::cout << "Number of Flows: " << numFlows << std::endl;
        std::cout << "avgDelay: " << avgDelay << std::endl;
        std::cout << "avgPLR: " << avgPLR << std::endl;
        std::cout << "avgThroughput: " << avgThroughput << std::endl;
        std::cout << "avgFct: " << avgFct << std::endl;
        std::cout << "\n " << std::endl;*/
        // Log average metrics
        //1:Simulation time, 2: delay, 3: plr, 4:th, 5: fct
        *trafficStatsFile << Simulator::Now().GetSeconds() << "\t" << avgDelay << "\t" << avgPLR << "\t" << avgThroughput << "\t" << avgFct << "\t" << std::endl;
         trafficStatsFile->flush();
	}
   
    Simulator::Schedule(Seconds(0.1), &CollectData, monitor, classifier, trafficStatsFile);
}


std::map < uint64_t, ns3::Time > packet_enqueue_times;
std::map < uint64_t, std::pair < double, double >> packet_info;
std::ofstream sojournFile;
std::map < uint32_t, Time > enqueueTimes;
std::ofstream bufferFile;

/**
 * Logs queue size changes.
 *
 * Called whenever the queue size changes. Logs the current simulation time and the new queue size to a file.
 *
 * @param oldVal Previous value of the queue size.
 * @param newVal New value of the queue size.
 */
void TraceQueueSize(uint32_t oldVal, uint32_t newVal) {
  double now = Simulator::Now().GetSeconds();
  //std::cout << "TraceQueueSize called at time " << now << " with value " << newVal<< std::endl;
  bufferFile << now << "\t" << newVal << std::endl;
}

/**
 * Records the time when a packet is enqueued.
 *
 * Logs the enqueue time for the packet to a map, using the packet's ID as the key.
 *
 * @param item Pointer to the QueueDiscItem being enqueued.
 */
void EnqueueTrace(Ptr < const QueueDiscItem > item) {
  Time enqueueTime = Simulator::Now();
  enqueueTimes[item -> GetPacket() -> GetUid()] = enqueueTime;
}

/**
 * Records the time when a packet is dequeued and calculates its sojourn time.
 *
 * Logs the dequeue time and calculates the sojourn time for the packet, then logs it to a file.
 *
 * @param item Pointer to the QueueDiscItem being dequeued.
 */
void DequeueTrace(Ptr < const QueueDiscItem > item) {
  Time dequeueTime = Simulator::Now();
  uint32_t packetId = item -> GetPacket() -> GetUid();
  if (enqueueTimes.count(packetId) > 0) {
    // Calculate the sojourn time
    double sojournTime = (dequeueTime - enqueueTimes[packetId]).GetMilliSeconds();
    //NS_LOG_UNCOND("Packet Dequeued. Packet ID: " << packetId << ", Sojourn time: " << sojournTime << "ms");
    // Write the simulation time and the sojourn time to the file
    sojournFile << dequeueTime.GetSeconds() << "\t" << sojournTime << std::endl;
    enqueueTimes.erase(packetId); // Optionally, remove the packet ID from the map
  } 
}

/**
 * Records the time when a packet is dropped.
 *
 * Logs the drop time for the packet.
 *
 * @param item Pointer to the QueueDiscItem being dropped.
 */
void DropTrace(Ptr < const QueueDiscItem > item) {
  Time dropTime = Simulator::Now();
  //std::cout << "Drop time: " << dropTime << std::endl;
}


std::ofstream trafficInFile;
std::map < Ptr < Node > , uint64_t > totalTxBytes;
void ConnectSocketTracer(Ptr < Application > app, Ptr < Node > node);

/**
 * Callback function for tracking transmitted bytes.
 *
 * Increments the total transmitted bytes for the given node.
 *
 * @param node The node that transmitted the packet.
 * @param packet The packet that was transmitted.
 */
void TxCallback(Ptr < Node > node, Ptr < const Packet > packet) {
  totalTxBytes[node] += packet -> GetSize();
  //std::cout << "TxCallback is called for node " << node->GetId() << ". Total bytes sent: " << totalTxBytes[node] << std::endl;
}

/**
 * Connects the TxCallback function to the Tx trace source of the socket.
 *
 * @param app The application associated with the socket.
 * @param node The node where the socket is located.
 */
void ConnectSocketTracer(Ptr < Application > app, Ptr < Node > node) {
  Ptr < BulkSendApplication > bulkSendApp = app -> GetObject < BulkSendApplication > ();
  if (bulkSendApp) {
    Ptr < Socket > socket = bulkSendApp -> GetSocket();
    if (socket) {
      socket -> TraceConnectWithoutContext("Tx", MakeBoundCallback( & TxCallback, node));
    }
  }
}

/**
 * Calculates and logs the traffic rate (in kbps).
 *
 * @param node The node for which the traffic rate will be calculated.
 * @param trafficInFile Output file for traffic rate data.
 * @param startTime The simulation start time.
 * @param queueDiscType Type of Queue Disc being used.
 */
void CalculateTraffic(Ptr < Node > node, std::ofstream & trafficInFile, double startTime, std::string queueDiscType) {
  double simTime = Simulator::Now().GetSeconds();
  double traffic = totalTxBytes[node] * 8.0 / (simTime - startTime) / 1000.0; // kbps
  trafficInFile << simTime << "\t" << traffic << std::endl;
}

Time lastArrivalTime;
static Time total_interArrivalTime = Seconds(0);
static uint32_t total_packets = 0;
//std::ofstream interarrTimeFile("inter_arrival_time_Fifo" + traffic_type ".dat");
std::ofstream interarrTimeFile;

/**
 * Callback function for tracking received packets.
 *
 * Calculates and logs the inter-arrival time of packets.
 *
 * @param packet The received packet.
 * @param from The address from which the packet was received.
 */
std::vector<double> rxTimestamps;
void
RxCallback(Ptr <const Packet > packet, const Address & from) {
  Time now = Simulator::Now();
  Time interArrivalTime = now - lastArrivalTime;
  //std::cout << "Inter-arrival time: " << interArrivalTime.GetSeconds () << " seconds" << std::endl;
  //Convert from seconds to ms and then to an integer, so we can get the mode of the integers
  int interArrivalTimeMs = static_cast < int > (std::round(interArrivalTime.GetMilliSeconds()));
  interarrTimeFile << now << "\t" << interArrivalTimeMs << std::endl;
  total_interArrivalTime += interArrivalTime;
  total_packets++;
  lastArrivalTime = now;
  double timestamp = Simulator::Now().GetSeconds();
  rxTimestamps.push_back(timestamp);
}


/**
 * Executes the network simulation with varying queue disciplines and configurations.
 *
 * This function sets up a network of client and server nodes, configures the channel, 
 * and installs the specified queue disciplines. 
 * 
 * @param queueDiscType  Specifies the type of the queue discipline to employ.
 * @param config         A ConfigParameters object containing simulation configuration parameters.
 *
 */
void RunSimulation(const std::string & queueDiscType,
  const ConfigParameters & config) {
  std::string traffic_type = config.traffic_type;
  //LogComponentEnable ("dAQMQueueDisc", LOG_LEVEL_ALL);
  LogComponentEnable ("dAQMQueueDisc", LOG_LEVEL_DEBUG);
  // Define the number of client and server nodes
  uint32_t nClients = config.nclient;
  uint32_t nServers = config.nserver;

  // Create client and server nodes
  NodeContainer clients;
  clients.Create(nClients);
  NodeContainer servers;
  servers.Create(nServers);

  NodeContainer csmaNodes = NodeContainer(clients, servers);
  
  // For TCP NewReno
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
  Config::SetDefault ("ns3::TcpSocket::DelAckTimeout", TimeValue (Seconds (0)));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (65535));
  Config::SetDefault ("ns3::TcpSocketBase::LimitedTransmit", BooleanValue (false));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1446));
  Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (true));
  
  // Configure CSMA channel attributes
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(config.csma_data_rate)));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(config.csma_delay)));

  // Create a TrafficControlHelper object for setting up queue disciplines
  TrafficControlHelper trControl;

  // Set up the root queue discipline for nodes based on the input type
  if (queueDiscType == "Fifo") {
    trControl.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", QueueSizeValue(QueueSize("2000p")));
  }
  else if (queueDiscType == "Red") {
    trControl.SetRootQueueDisc("ns3::RedQueueDisc");
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(1000));
    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(500));
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
  } else if (queueDiscType == "CoDel") {
    trControl.SetRootQueueDisc("ns3::CoDelQueueDisc");
    Config::SetDefault("ns3::CoDelQueueDisc::MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, (2000))));
  } else if (queueDiscType == "FqCoDel") {
    trControl.SetRootQueueDisc("ns3::FqCoDelQueueDisc");
    Config::SetDefault("ns3::FqCoDelQueueDisc::MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, (2000))));
  }
  else if (queueDiscType == "Cobalt") {
    trControl.SetRootQueueDisc("ns3::CobaltQueueDisc");
    Config::SetDefault("ns3::CobaltQueueDisc::MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, (2000))));
    Config::SetDefault("ns3::CobaltQueueDisc::Pdrop", DoubleValue(0));
    Config::SetDefault("ns3::CobaltQueueDisc::Increment", DoubleValue(0.08));
    Config::SetDefault("ns3::CobaltQueueDisc::Decrement", DoubleValue(0.04));
  } else if (queueDiscType == "Pie") {
    trControl.SetRootQueueDisc("ns3::PieQueueDisc");
    Config::SetDefault("ns3::PieQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::PieQueueDisc::DequeueThreshold", UintegerValue(20));
    Config::SetDefault("ns3::PieQueueDisc::Tupdate", TimeValue(MilliSeconds(15)));
  } else if (queueDiscType == "dAQM1") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThreshold", TimeValue(MilliSeconds(config.sojourn4_drop_threshold)));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn4_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn4_drop_rate));
  } else if (queueDiscType == "dAQM2") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdi", DoubleValue(config.sojourn1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn1_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn1_drop_rate));
  } else if (queueDiscType == "dAQM3") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdii", DoubleValue(config.sojourn2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn2_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn2_drop_rate));
  } else if (queueDiscType == "dAQM4") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdiii", DoubleValue(config.sojourn3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn3_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn3_drop_rate));
  } else if (queueDiscType == "dAQM5") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThreshold", DoubleValue(config.buffer4_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.buffer4_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.buffer4_drop_rate));
  } else if (queueDiscType == "dAQM6") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdi", DoubleValue(config.buffer1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.buffer1_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.buffer1_drop_rate));
  } else if (queueDiscType == "dAQM7") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdii", DoubleValue(config.buffer2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.buffer2_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.buffer2_drop_rate));
  } else if (queueDiscType == "dAQM8") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdiii", DoubleValue(config.buffer3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.buffer3_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.buffer3_drop_rate));
  } else if (queueDiscType == "dAQM9") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThreshold", TimeValue(MilliSeconds(config.sojourn4_drop_threshold)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdi", DoubleValue(config.sojourn1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdii", DoubleValue(config.sojourn2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdiii", DoubleValue(config.sojourn3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThreshold", DoubleValue(config.buffer4_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdi", DoubleValue(config.buffer1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdii", DoubleValue(config.buffer2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdiii", DoubleValue(config.buffer3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn1_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn1_drop_rate));
  } else if (queueDiscType == "dAQM10") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThreshold", TimeValue(MilliSeconds(config.sojourn4_drop_threshold)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdi", DoubleValue(config.sojourn1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdii", DoubleValue(config.sojourn2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdiii", DoubleValue(config.sojourn3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThreshold", DoubleValue(config.buffer4_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdi", DoubleValue(config.buffer1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdii", DoubleValue(config.buffer2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdiii", DoubleValue(config.buffer3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn1_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn1_drop_rate));
  } else if (queueDiscType == "dAQM11") {
    trControl.SetRootQueueDisc("ns3::dAQMQueueDisc");
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThreshold", TimeValue(MilliSeconds(config.sojourn4_drop_threshold)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdi", DoubleValue(config.sojourn1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdii", DoubleValue(config.sojourn2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropSJThresholdiii", DoubleValue(config.sojourn3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThreshold", DoubleValue(config.buffer4_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdi", DoubleValue(config.buffer1_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdii", DoubleValue(config.buffer2_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::DropQLThresholdiii", DoubleValue(config.buffer3_drop_threshold));
    Config::SetDefault("ns3::dAQMQueueDisc::MaxSize", ns3::QueueSizeValue(ns3::QueueSize("2000p")));
    Config::SetDefault("ns3::dAQMQueueDisc::DropDuration", TimeValue(MilliSeconds(config.sojourn1_drop_duration)));
    Config::SetDefault("ns3::dAQMQueueDisc::DropRate", DoubleValue(config.sojourn1_drop_rate));
  } else {
    NS_LOG_ERROR("Invalid queue disc type: " << queueDiscType);
    return;
  }

  // Install CSMA devices and channels to nodes
  NetDeviceContainer csmaDevices;

  csmaDevices = csma.Install(csmaNodes);
  // Install Internet stack on nodes
  InternetStackHelper stack;
  stack.Install(csmaNodes);

  // Install queue disciplines on the csma devices
  QueueDiscContainer qdiscs = trControl.Install(csmaDevices);

  // Connect tracing sources for all queue disciplines
  for (uint32_t i = 0; i < qdiscs.GetN(); ++i) {
    Ptr < QueueDisc > qdisc = qdiscs.Get(i);
    if (queueDiscType == "Fifo") {
      Ptr < FifoQueueDisc > fifoQueue = DynamicCast < FifoQueueDisc > (qdisc);
      if (fifoQueue) {
        fifoQueue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        fifoQueue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        fifoQueue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        fifoQueue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    }
    else if (queueDiscType == "Red") {
      Ptr < RedQueueDisc > rediiiQueue = DynamicCast < RedQueueDisc > (qdisc);
      if (rediiiQueue) {
        rediiiQueue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        rediiiQueue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        rediiiQueue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        rediiiQueue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "CoDel") {
      Ptr < CoDelQueueDisc > codelQueue = DynamicCast < CoDelQueueDisc > (qdisc);
      if (codelQueue) {
        codelQueue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        codelQueue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        codelQueue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        codelQueue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "FqCoDel") {
      Ptr < FqCoDelQueueDisc > fqcodelQueue = DynamicCast < FqCoDelQueueDisc > (qdisc);
      if (fqcodelQueue) {
        fqcodelQueue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        fqcodelQueue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        fqcodelQueue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        fqcodelQueue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    }
    else if (queueDiscType == "Cobalt") {
      Ptr < CobaltQueueDisc > cobaltiiQueue = DynamicCast < CobaltQueueDisc > (qdisc);
      if (cobaltiiQueue) {
        cobaltiiQueue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        cobaltiiQueue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        cobaltiiQueue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        cobaltiiQueue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "Pie") {
      Ptr < PieQueueDisc > pieQueue = DynamicCast < PieQueueDisc > (qdisc);
      if (pieQueue) {
        pieQueue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        pieQueue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        pieQueue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        pieQueue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM1") {
      Ptr < dAQMQueueDisc > dAQM1Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM1Queue) {
        dAQM1Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM1Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM1Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM1Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM2") {
      Ptr < dAQMQueueDisc > dAQM2Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM2Queue) {
        dAQM2Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM2Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM2Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM2Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM3") {
      Ptr < dAQMQueueDisc > dAQM3Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM3Queue) {
        dAQM3Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM3Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM3Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM3Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM4") {
      Ptr < dAQMQueueDisc > dAQM4Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM4Queue) {
        dAQM4Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM4Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM4Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM4Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM5") {
      Ptr < dAQMQueueDisc > dAQM5Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM5Queue) {
        dAQM5Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM5Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM5Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM5Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM6") {
      Ptr < dAQMQueueDisc > dAQM6Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM6Queue) {
        dAQM6Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM6Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM6Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM6Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM7") {
      Ptr < dAQMQueueDisc > dAQM7Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM7Queue) {
        dAQM7Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM7Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM7Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM7Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM8") {
      Ptr < dAQMQueueDisc > dAQM8Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM8Queue) {
        dAQM8Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM8Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM8Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM8Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM9") {
      Ptr < dAQMQueueDisc > dAQM9Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM9Queue) {
        dAQM9Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM9Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM9Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM9Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM10") {
      Ptr < dAQMQueueDisc > dAQM10Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM10Queue) {
        dAQM10Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM10Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM10Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM10Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    } else if (queueDiscType == "dAQM11") {
      Ptr < dAQMQueueDisc > dAQM11Queue = DynamicCast < dAQMQueueDisc > (qdisc);
      if (dAQM11Queue) {
        dAQM11Queue -> TraceConnectWithoutContext("Enqueue", MakeCallback( & EnqueueTrace));
        dAQM11Queue -> TraceConnectWithoutContext("Dequeue", MakeCallback( & DequeueTrace));
        dAQM11Queue -> TraceConnectWithoutContext("Drop", MakeCallback( & DropTrace));
        dAQM11Queue -> TraceConnectWithoutContext("PacketsInQueue", MakeCallback( & TraceQueueSize));
      }
    }
  }

  // Mobility settings for nodes
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
    "MinX", DoubleValue(10.0),
    "MinY", DoubleValue(10.0),
    "DeltaX", DoubleValue(5.0),
    "DeltaY", DoubleValue(5.0),
    "GridWidth", UintegerValue(1),
    "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  for (uint32_t i = 0; i < nClients; ++i) {
    mobility.Install(csmaNodes.Get(i));
  }
  MobilityHelper mobility1;
  mobility1.SetPositionAllocator("ns3::GridPositionAllocator",
    "MinX", DoubleValue(50.0),
    "MinY", DoubleValue(50.0),
    "DeltaX", DoubleValue(100.0),
    "DeltaY", DoubleValue(100.0),
    "GridWidth", UintegerValue(1),
    "LayoutType", StringValue("RowFirst"));
  mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  for (uint32_t i = nClients; i < nServers + nClients; ++i) {
    mobility1.Install(csmaNodes.Get(i));
  }

  // Assign IP addresses to the interfaces
  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign(csmaDevices);


  uint32_t basePort = 8000; // A starting port number

  interarrTimeFile.open("inter_arrival_time_Fifo" + traffic_type + ".dat");

  //Set up Servers
  for (uint32_t i = 0; i < nServers; ++i) {
    uint16_t port = basePort + i;
    Address serverAddress(InetSocketAddress(csmaInterfaces.GetAddress(nClients + i), port));
    PacketSinkHelper sink("ns3::TcpSocketFactory", serverAddress);
    //PacketSinkHelper sink("ns3::UdpSocketFactory", serverAddress);
    ApplicationContainer sinkApp = sink.Install(servers.Get(i));
    // Connect the trace source
    for (uint32_t j = 0; j < sinkApp.GetN(); ++j) {
      Ptr < PacketSink > packetSink = DynamicCast < PacketSink > (sinkApp.Get(j));
      packetSink -> TraceConnectWithoutContext("Rx", MakeCallback( & RxCallback));
    }
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(simulationTime));
  }

  
  std::ofstream trafficInFile("trafficIn-" + queueDiscType + traffic_type + ".dat");

  // Open file for logging incoming traffic
  if (!trafficInFile.is_open()) {
    std::cerr << "Error: could not open output file." << std::endl;
    return;
  }

 
  //double lambda = config.client_lambda; //Arrival rate of the Poisson process
  //double mean = 1.0 / lambda; //Mean inter-arrival time that follows a Poisson process
  
  //Set up Clients
  //This block includes a combination of various traffic models. Uncomment the appropriate model as needed.
  for (uint32_t i = 0; i < nClients; ++i) {
    int serverIndex = nClients + (i % nServers);
    int port = basePort + (i % nServers);

    // OnOff Configuration for either CBR or VBR
    /*OnOffHelper onoffclient ("ns3::TcpSocketFactory", InetSocketAddress (csmaInterfaces.GetAddress (serverIndex), port));
    onoffclient.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    //onoffclient.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.5]"));//For VBR
    onoffclient.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]")); //For CBR
    onoffclient.SetAttribute ("PacketSize", UintegerValue (config.packet_size));
    onoffclient.SetAttribute ("DataRate", StringValue (config.client_data_rate)); 
  
    ApplicationContainer onoffApps = onoffclient.Install(clients.Get(i));
    onoffApps.Start (Seconds (0.0 + i ));
    onoffApps.Stop (Seconds (simulationTime));*/

    // Poisson Traffic Configuration
    // Uncomment the lambda and mean variables while using this traffic model.
    /*OnOffHelper poissonclient ("ns3::TcpSocketFactory", InetSocketAddress (csmaInterfaces.GetAddress (serverIndex), port));
    ns3::Ptr < ExponentialRandomVariable > poisson = CreateObject < ExponentialRandomVariable > ();
    poisson -> SetAttribute("Mean", DoubleValue(mean)); //scale parameter
    //std::cout << "Mean value: " << mean << std::endl;
 
    double startTime = poisson -> GetValue();
    poissonclient.SetAttribute("MaxBytes", UintegerValue(0)); //no limit to the amount of data to be sent
    poissonclient.SetAttribute ("DataRate", StringValue (config.client_data_rate)); 
    poissonclient.SetAttribute("PacketSize", UintegerValue(config.packet_size));
    std::stringstream on;
    on << "ns3::ExponentialRandomVariable[Mean=" << mean << "]";
    poissonclient.SetAttribute("OnTime", StringValue(on.str()));

    std::stringstream off;
    off << "ns3::ExponentialRandomVariable[Mean=" << mean << "]";
    poissonclient.SetAttribute("OffTime", StringValue(off.str()));

    ApplicationContainer poiApps = poissonclient.Install(clients.Get(i));

    poiApps.Start(Seconds(startTime));
    poiApps.Stop(Seconds(simulationTime));*/

    // Pareto Traffic Configuration 
    /*OnOffHelper paretolient ("ns3::TcpSocketFactory", InetSocketAddress (csmaInterfaces.GetAddress (serverIndex), port));
    ns3::Ptr<ParetoRandomVariable> pareto = CreateObject<ParetoRandomVariable> ();
    pareto->SetAttribute ("Scale", DoubleValue (1.5)); //scale parameter
    pareto->SetAttribute ("Shape", DoubleValue (2.5)); //sahape parameter
		
    paretolient.SetAttribute ("DataRate", StringValue (config.client_data_rate)); 
    paretolient.SetAttribute ("PacketSize", UintegerValue(config.packet_size));
    paretolient.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
    paretolient.SetAttribute ("OffTime", PointerValue (pareto));
    ApplicationContainer pareApps = paretolient.Install (clients.Get (i));
    
    pareApps.Start (Seconds (0.0));
    pareApps.Stop (Seconds (simulationTime));*/

    // Weibull Traffic Configuration
    OnOffHelper weibullclient ("ns3::TcpSocketFactory", InetSocketAddress (csmaInterfaces.GetAddress (serverIndex), port));
    //OnOffHelper weibullclient ("ns3::UdpSocketFactory", InetSocketAddress (csmaInterfaces.GetAddress (serverIndex), port));
    ns3::Ptr<WeibullRandomVariable> weibull = CreateObject<WeibullRandomVariable> ();
    weibull->SetAttribute ("Scale", DoubleValue (1)); //scale parameter
    weibull->SetAttribute ("Shape", DoubleValue (1.5)); //shape parameter
		
    weibullclient.SetAttribute ("DataRate", StringValue (config.client_data_rate)); 
    weibullclient.SetAttribute ("PacketSize", UintegerValue(config.packet_size));
    weibullclient.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=3.0]"));
    weibullclient.SetAttribute ("OffTime", PointerValue (weibull));
    ApplicationContainer weiclientApps = weibullclient.Install (clients.Get (i));
    
    weiclientApps.Start (Seconds (0.0));
    weiclientApps.Stop (Seconds (simulationTime));

    totalTxBytes[clients.Get(i)] = 0;
    Ptr < NetDevice > device = clients.Get(i) -> GetDevice(0);
    device -> TraceConnectWithoutContext("PhyTxEnd", MakeBoundCallback( & TxCallback, clients.Get(i)));

    for (double t = 0.0; t < simulationTime; t += 0.1) {
      Simulator::Schedule(Seconds(t), & CalculateTraffic, clients.Get(i), std::ref(trafficInFile), 0.0, queueDiscType);
    }

  }

  // Initialize Animation
  AnimationInterface anim("trAnim.xml");
  anim.SetMaxPktsPerTraceFile(1000000);
  for (uint32_t i = 0; i < nClients; ++i) {
    anim.UpdateNodeDescription(csmaNodes.Get(i), "sender");
    anim.UpdateNodeColor(csmaNodes.Get(i), 255, 0, 0);
  }
  for (uint32_t i = nClients; i < nServers + nClients; ++i) {
    anim.UpdateNodeDescription(csmaNodes.Get(i), "receiver");
    anim.UpdateNodeColor(csmaNodes.Get(i), 0, 255, 0);
  }

  // Initialize and install Flow Monitor  
  Ptr < FlowMonitor > flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  // Stop simulation after predefined simulation time
  Simulator::Stop(Seconds(simulationTime));

  // Write sojourn time to a data file
  sojournFile.open("sojournFile" + queueDiscType + traffic_type + ".dat", std::ios::out | std::ios::trunc);
  bufferFile.open("bufferFile" + queueDiscType + traffic_type + ".dat", std::ios::out);
  for (const auto & pair: packet_info) {
    sojournFile << pair.second.first << " " << pair.second.second << std::endl;
  }

  // Additional Flow Monitoring and Data Collection
  Ptr < FlowMonitor > monitor;
  monitor = flowHelper.InstallAll();
  Ptr < Ipv4FlowClassifier > classifier = DynamicCast < Ipv4FlowClassifier > (flowHelper.GetClassifier());
  std::string fileName = "trafficStatsFile" + queueDiscType + traffic_type + ".dat";
  std::shared_ptr < std::ofstream > trafficStatsFile = std::make_shared < std::ofstream > (fileName.c_str(), std::ios::out);
  CollectData(monitor, classifier, trafficStatsFile);

  // Enable ASCII Tracing
  AsciiTraceHelper ascii;
  //csma.EnableAsciiAll(ascii.CreateFileStream("combi-tcp-trace-" + queueDiscType + traffic_type + ".dat"));

  // Run Simulation
  Simulator::Run();

  // Calculate and output average inter-arrival time if packets were received
  if (total_packets != 0) {
    Time average_interArrivalTime = total_interArrivalTime / total_packets;
    std::cout << "Average inter-arrival time: " << average_interArrivalTime.GetSeconds() << " seconds" << std::endl;
  } else {
    std::cout << "No packets were received." << std::endl;
  }

  flowMonitor -> SerializeToXmlFile("trFlowStats.xml", true, true);

  flowMonitor -> CheckForLostPackets();
  FlowMonitor::FlowStatsContainer stats = flowMonitor -> GetFlowStats();
	
  uint64_t numPackets = rxTimestamps.size();
  double serviceRate = static_cast<double>(numPackets) / simulationTime;
  std::cout << "Total Service rate (mu) = " << serviceRate << " p/s" << std::endl;
  rxTimestamps.clear();

  
  // Close output files
  sojournFile.close();
  bufferFile.close();
  trafficInFile.close();
  interarrTimeFile.close();
  trafficStatsFile -> close();
  Simulator::Destroy();
}

int main(int argc, char * argv[]) {
  // Uncomment the relevant line to select the set of AQM algorithms to be used in the simulation
  //std::vector<std::string> aqmAlgorithms = {"Fifo", "Red", "CoDel", "FqCoDel", "Cobalt", "Pie", "dAQM1", "dAQM2", "dAQM3", "dAQM4", "dAQM5", "dAQM6", "dAQM7", "dAQM8"};
  std::vector<std::string> aqmAlgorithms = { "dAQM9"};
  //std::vector<std::string> aqmAlgorithms = { "dAQM10"};
  //std::vector<std::string> aqmAlgorithms = { "dAQM11"};
  std::vector < ConfigParameters > configSets;

  // Load parameters from a .txt file
  if (!loadParametersFromFile("Configuration_dAQM1.txt", configSets)) {
    std::cerr << "Failed to load parameters from file.\n";
    return 1;
  }
  // Display the number of configurations loaded
  std::cout << "Number of configurations loaded: " << configSets.size() << std::endl;

  // Iterate through each configuration set
  for (const auto & config: configSets) {
    // Run simulations for each AQM algorithm
    for (const auto & aqm: aqmAlgorithms) {
      RunSimulation(aqm, config);
      std::cout << aqm << ": Simulation done." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    std::cout << config.traffic_type << ": Simulation done." << std::endl;
  }
  return 0;
}
