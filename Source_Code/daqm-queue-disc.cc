#include "daqm-queue-disc.h"
#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
#include <vector>
#include "ns3/simulator.h"
#include "queue-disc.h"
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("dAQMQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (dAQMQueueDisc);

TypeId
dAQMQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::dAQMQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<dAQMQueueDisc> ()
    .AddAttribute ("DropSJThreshold",
                   "Constant sojourn threshold for dropping packets.",
                   TimeValue (MilliSeconds (2500)),
                   MakeTimeAccessor (&dAQMQueueDisc::m_sj_dropThreshold),
                   MakeTimeChecker ())
    .AddAttribute ("DropSJThresholdi",
                   "Threshold for dropping packets based on 1st deri sojourn time difference.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_sj_dropThresholdi),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DropSJThresholdii",
                   "Threshold for dropping packets based on 2nd deri sojourn time difference.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_sj_dropThresholdii),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DropSJThresholdiii",
                   "Threshold for dropping packets based on 3rd deri sojourn time difference.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_sj_dropThresholdiii),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DropQLThreshold",
                   "Constant sojourn threshold for dropping packets.",
                   DoubleValue (200.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_ql_dropThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DropQLThresholdi",
                   "Threshold for dropping packets based on 1st deri buffer size difference.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_ql_dropThresholdi),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DropQLThresholdii",
                   "Threshold for dropping packets based on 2nd deri buffer size difference.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_ql_dropThresholdii),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DropQLThresholdiii",
                   "Threshold for dropping packets based on 3rd deri buffer size difference.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_ql_dropThresholdiii),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSize",
                   "Maximum size of the queue.",
                   QueueSizeValue (QueueSize ("200p")),
                    MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("DropDuration",
                   "Duration for dropping packets.",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&dAQMQueueDisc::m_dropDuration),
                   MakeTimeChecker ())
    .AddAttribute ("PacketInterval",
                   "Inter arrival time of packets.",
                   TimeValue (MilliSeconds (2)),
                   MakeTimeAccessor (&dAQMQueueDisc::m_packetInterval),
                   MakeTimeChecker ())
    .AddAttribute ("DropRate",
                   "DropRate for dropping packets.",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&dAQMQueueDisc::m_dropRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Target",
                   "Target queue delay in ms after which the drop conditions can be triggered.",
                   TimeValue (MilliSeconds (5)),
                   MakeTimeAccessor (&dAQMQueueDisc::m_target),
                   MakeTimeChecker ())
    ;
  return tid;
}

dAQMQueueDisc::dAQMQueueDisc ()
 : QueueDisc (QueueDiscSizePolicy::MULTIPLE_QUEUES, QueueSizeUnit::PACKETS),
    m_sj_dropThreshold (MilliSeconds (2500)),
    m_sj_dropThresholdi (50.0),
    m_sj_dropThresholdii (50.0),
    m_sj_dropThresholdiii (50.0),
    m_ql_dropThreshold (200.0),
    m_ql_dropThresholdi (50.0),
    m_ql_dropThresholdii (50.0),
    m_ql_dropThresholdiii (50.0),
    m_dropDuration (MilliSeconds (100)),
    m_packetInterval (MilliSeconds (2)),
    m_target (MilliSeconds (5)),
    m_dropPackets (false),
    droppedSJFirst (false),
    droppedSJSecond (false),
    droppedSJThird (false),
    droppedSJConstant (false),
    droppedQLFirst (false),
    droppedQLSecond (false),
    droppedQLThird (false),
    droppedQLConstant (false),
    flag (false),
    m_dropRate (0.5),
    bufferSizes ()

{

   m_rng = CreateObject<UniformRandomVariable> ();
   m_rng->SetAttribute ("Min", DoubleValue (0.0));
   m_rng->SetAttribute ("Max", DoubleValue (1.0));

   if ((m_rng == nullptr))
   {
     NS_LOG_INFO("m_rng is not initialized");
   }
   m_lastSampleTime = std::chrono::steady_clock::now();
   m_currentIndex = 0;
}


dAQMQueueDisc::~dAQMQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

bool dAQMQueueDisc::DoEnqueue(Ptr < QueueDiscItem > item) {

  NS_LOG_FUNCTION(this << item);

  NS_LOG_INFO("Current Queue size before enqueue: " << GetCurrentSize());

  // Check if enqueuing the new packet will exceed the maximum allowed queue size.
  if (GetCurrentSize() + item > GetMaxSize()) {
    NS_LOG_WARN("Queue full -- dropping packet.");
    DropBeforeEnqueue(item, OVERLIMIT_DROP);
    return false;
  }

  // Check if packets are currently in the dropping phase due to exceeding dAQM deri ratio threshold.
  if (m_dropPackets && m_rng -> GetValue() < m_dropRate) {
    NS_LOG_DEBUG("dAQM deri ratio exceeded -- dropping packet with " << m_dropRate * 100 << "% chance.");
    DropBeforeEnqueue(item, UNFORCED_DROP);
    NS_LOG_DEBUG("Dropping packet with ID: " << item -> GetPacket() -> GetUid());
    droppedSJFirst = false;
    droppedSJSecond = false;
    droppedSJThird = false;
    droppedSJConstant = false;
    droppedQLFirst = false;
    droppedQLSecond = false;
    droppedQLThird = false;
    droppedQLConstant = false;
    return false;
  }

  // Attempt to enqueue the packet.
  bool retval = GetInternalQueue(0) -> Enqueue(item);
  Time enqueueTime = Simulator::Now();
  NS_LOG_INFO("Enqueuing packet: " << item << " with ID: " << item -> GetPacket() -> GetUid() << " at time:" << enqueueTime);
  NS_LOG_INFO("Current Queue size after enqueue: " << GetCurrentSize());
  NS_LOG_INFO("Number packets " << GetInternalQueue(0) -> GetNPackets());
  NS_LOG_INFO("Number bytes " << GetInternalQueue(0) -> GetNBytes());

  return retval;

}

Ptr < QueueDiscItem > dAQMQueueDisc::DoDequeue() {

    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Current Queue size before dequeue: " << GetCurrentSize());

    Ptr < QueueDiscItem > item = GetInternalQueue(0) -> Dequeue();

    // If the queue is empty, log it and return null.
    if (!item) {
      NS_LOG_DEBUG("Queue empty");
      return 0;
    }

    // Call DropPacket to determine whether the packet should be dropped.
    bool drop = DropPacket(item);

    // Drop the packet based on DropPacket's return value and the if-else conditions.
    if (drop == false) {
      NS_LOG_DEBUG("Time ratio > threshold -- dropping packets for the next " << m_dropDuration.GetMilliSeconds() << "ms with probability of " << m_dropRate * 100 << "%.");

      // Drop the packet after dequeue.
      DropAfterDequeue(item, TIME_EXCEEDED_DROP);

      // Start dropping packets.
      m_dropPackets = true;
      NS_LOG_DEBUG("Scheduling to Stop Dropping Packets after " << m_dropDuration.GetMilliSeconds() << "ms.");

      // Schedule to stop dropping packets after a duration.
      Simulator::Schedule(m_dropDuration, & dAQMQueueDisc::StopDroppingPackets, this);

      NS_LOG_DEBUG("Dequeuing packet: " << item << "with ID: " << item -> GetPacket() -> GetUid() << " at time:" << Simulator::Now());

      // Dequeue the oldest packet from the internal queue.
      item = GetInternalQueue(0) -> Dequeue();
    }
    NS_LOG_INFO("Current Queue size after dequeue: " << GetCurrentSize());

    return item;
  }

void dAQMQueueDisc::StopDroppingPackets() {
  NS_LOG_FUNCTION(this);
  // Stop dropping packets.
  m_dropPackets = false;
  NS_LOG_INFO("Stop Dropping Packets at: " << Simulator::Now());
}

bool dAQMQueueDisc::DropPacket(Ptr < QueueDiscItem > item) {
  NS_LOG_FUNCTION(this << item);

  // Initialize variables, schedule the update for buffer size.
  uint32_t Npackets = GetInternalQueue(0) -> GetNPackets();
  Time now = Simulator::Now();
  Simulator::Schedule(now + MilliSeconds(15), & dAQMQueueDisc::UpdateCurrentBufferSize, this);

  // The logic for packet dropping based on dAQM algorithm.
  if (now > m_target) {
    if ((Npackets > 5) && (m_dropPackets == false) && (flag == false)) {
      double m_sj_ratio3 = Get_SJ_Third_Deri_Ratio();
      double m_sj_ratio2 = Get_SJ_Second_Deri_Ratio();
      double m_sj_ratio1 = Get_SJ_First_Deri_Ratio();
      Time constant_sj_val = packet1WaitTime;
      double constant_ql_val = 40;
      if ((m_sj_ratio3 > m_sj_dropThresholdiii) || (m_sj_ratio3 < m_sj_dropThresholdiii * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 3rd deri sojourn time ratio: " << m_sj_ratio3);
        flag = true;
        droppedSJThird = true;
        //std::cout << "The drop is triggered for drop with Third Deri: " << std::boolalpha << droppedSJThird << std::endl;
        return false; // Drop the packet
      } else if ((m_ql_ratio3 > m_ql_dropThresholdiii) || (m_ql_ratio3 < m_ql_dropThresholdiii * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 3rd deri buffer size ratio: " << m_ql_ratio3);
        flag = true;
        droppedQLThird = true;
        //std::cout << "The drop is triggered for drop with Third Deri: " << std::boolalpha << droppedQLSecond << std::endl;
        return false; // Drop the packet
      } else if ((m_sj_ratio2 > m_sj_dropThresholdii) || (m_sj_ratio2 < m_sj_dropThresholdii * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 2nd deri sojourn time ratio: " << m_sj_ratio2);
        flag = true;
        droppedSJSecond = true;
        //std::cout << "The drop is triggered for drop with Second Deri: " << std::boolalpha << droppedSJSecond << std::endl;
        return false; // Drop the packet
      } else if ((m_ql_ratio2 > m_ql_dropThresholdii) || (m_ql_ratio2 < m_ql_dropThresholdii * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 2nd deri buffer size ratio: " << m_ql_ratio2);
        flag = true;
        droppedQLSecond = true;
        //std::cout << "The drop is triggered for drop with Second Deri: " << std::boolalpha << droppedQLSecond << std::endl;
        return false; // Drop the packet
      } else if ((m_sj_ratio1 > m_sj_dropThresholdi) || (m_sj_ratio1 < m_sj_dropThresholdi * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 1st deri sojourn time ratio: " << m_sj_ratio1);
        flag = true;
        droppedSJFirst = true;
        //std::cout << "The drop is triggered for drop with First Deri: " << std::boolalpha << droppedSJFirst << std::endl;
        return false; // Drop the packet
      } else if ((m_ql_ratio1 > m_ql_dropThresholdi) || (m_ql_ratio1 < m_ql_dropThresholdi * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 1st deri buffer size ratio: " << m_ql_ratio1);
        flag = true;
        droppedQLFirst = true;
        //std::cout << "The drop is triggered for drop with First Deri: " << std::boolalpha << droppedQLSecond << std::endl;
        return false; // Drop the packet
      } else if ((constant_sj_val > m_sj_dropThreshold) || (constant_sj_val < m_sj_dropThreshold * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive constant sojourn time limit: " << m_sj_dropThreshold);
        flag = true;
        droppedSJConstant = true;
        //std::cout << "The drop is triggered for drop with constant sojourn limit: " << std::boolalpha << droppedSJConstant << std::endl;
        return false; // Drop the packet
      } else if ((constant_ql_val > m_ql_dropThreshold) || (constant_ql_val < m_ql_dropThreshold * (-1))) {
        NS_LOG_DEBUG("Dropping packet due to excessive 1st deri buffer size ratio: " << constant_ql_val);
        flag = true;
        droppedQLConstant = true;
        //std::cout << "The drop is triggered for drop with constant queue limit: " << std::boolalpha << droppedQLConstant << std::endl;
        return false; // Drop the packet
      }
    }
  } else {
   NS_LOG_DEBUG("Less than 5 packets in queue; time is less than target; or in the dropping phase, not going to calculate dAQM deri ratios");
  }

  // Reset flag for next iteration.
  flag = false;
  return true;

}

Ptr <const QueueDiscItem > dAQMQueueDisc::DoPeek() const {
      NS_LOG_FUNCTION(this);

      Ptr <const QueueDiscItem > item = GetInternalQueue(0) -> Peek();

      if (!item) {
        NS_LOG_DEBUG("Queue empty");
        return 0;
      }

      return item;

    }

void dAQMQueueDisc::DoDispose() {
  NS_LOG_FUNCTION(this);

  QueueDisc::DoDispose();
}

/**
 * Calculates the first derivative of sojourn time ratio between two consecutive packets.
 *
 * @return Returns the first derivative of the sojourn time ratio.
 */
double dAQMQueueDisc::Get_SJ_First_Deri_Ratio()

{

  double interPacketTime = static_cast < double > (packet2WaitTime.GetNanoSeconds() / 1000000 - packet1WaitTime.GetNanoSeconds() / 1000000) / static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000);
  //std::cout << "First Deri ratio "<<interPacketTime << std::endl;

  return interPacketTime;
}

/**
 * Calculates the second derivative of sojourn time ratio between four consecutive packets.
 *
 * @return Returns the second derivative of the sojourn time ratio.
 */
double dAQMQueueDisc::Get_SJ_Second_Deri_Ratio()

{

  double a = static_cast < double > ((packet3WaitTime.GetNanoSeconds() / 1000000 - packet1WaitTime.GetNanoSeconds() / 1000000) / (2.0 * static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000)));

  double b = static_cast < double > ((packet4WaitTime.GetNanoSeconds() / 1000000 - packet2WaitTime.GetNanoSeconds() / 1000000) / (2.0 * static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000)));

  //std::cout << "a "<< a << "ms"<< std::endl;
  //std::cout << "b "<< b << "ms"<< std::endl;
  double interPacketTime = (b - a) / static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000);
  //std::cout << "a " << std::setprecision(2) << std::fixed << a << "ms"<< std::endl;
  //std::cout << "b " << std::setprecision(2) << std::fixed << b << "ms"<< std::endl;
  //std::cout << "Second Deri ratio "<<interPacketTime << std::endl;

  return interPacketTime;
}

/**
 * Calculates the third derivative of sojourn time ratio between five consecutive packets.
 *
 * @return Returns the third derivative of the sojourn time ratio.
 */
double dAQMQueueDisc::Get_SJ_Third_Deri_Ratio()

{
  NS_LOG_FUNCTION(this);

  packet1WaitTime = GetPacketWaitTime();
  packet2WaitTime = GetPacketWaitTime();
  packet3WaitTime = GetPacketWaitTime();
  packet4WaitTime = GetPacketWaitTime();
  packet5WaitTime = GetPacketWaitTime();

  //double a = (packet3WaitTime-packet1WaitTime).GetMilliSeconds()/(2* m_packetInterval.GetMilliSeconds());
  double a = static_cast < double > (packet3WaitTime.GetNanoSeconds() / 1000000 - packet1WaitTime.GetNanoSeconds() / 1000000) / (2.0 * static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000));

  //double b = (packet4WaitTime- packet2WaitTime).GetMilliSeconds ()/ (2* m_packetInterval.GetMilliSeconds ());
  double b = static_cast < double > (packet4WaitTime.GetNanoSeconds() / 1000000 - packet2WaitTime.GetNanoSeconds() / 1000000) / (2.0 * static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000));

  //double c = (packet5WaitTime- packet3WaitTime).GetMilliSeconds ()/(2* m_packetInterval.GetMilliSeconds ());
  double c = static_cast < double > (packet5WaitTime.GetNanoSeconds() / 1000000 - packet3WaitTime.GetNanoSeconds() / 1000000) / (2.0 * static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000));

  //std::cout << "a " << std::setprecision(2) << std::fixed << a << "ms"<< std::endl;
  //std::cout << "b " << std::setprecision(2) << std::fixed << b << "ms"<< std::endl;
  //std::cout << "c " << std::setprecision(2) << std::fixed << c << "ms"<< std::endl;

  double interPacketTime1 = (b - a) / static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000);
  double interPacketTime2 = (c - b) / static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000);
  double interPacketTime = (interPacketTime2 - interPacketTime1) / static_cast < double > (m_packetInterval.GetNanoSeconds() / 1000000);
  //std::cout << "interPacketTime1 "<< interPacketTime1 << "ms"<< std::endl;
  //std::cout << "interPacketTime2 "<< interPacketTime2 << "ms"<< std::endl;
  //std::cout << "Third Deri ratio "<<interPacketTime << std::endl;

  return interPacketTime;
}

Time dAQMQueueDisc::GetPacketWaitTime() const {

  if (GetInternalQueue(0) -> IsEmpty()) {
   NS_LOG_INFO("The queue is empty. Cannot dequeue.");
    return Time(); // Return a zero duration if the queue is empty
  }
  Ptr < QueueDiscItem > item = GetInternalQueue(0) -> Dequeue();
 NS_LOG_INFO("Dequeuing packet: " << item << "with ID: " << item -> GetPacket() -> GetUid() << " at time:" << Simulator::Now());
  Time currentTime = Simulator::Now();
  Time enqueueTime = item -> GetTimeStamp(); //gets the enqueue time stamp
  Time waitTime = currentTime - enqueueTime;

  return waitTime;
}

void dAQMQueueDisc::Get_QL_Deri_Ratio() {
  // All buffer sizes have been updated, continue with the rest of the calculation
  int curBufSize1 = bufferSizes[0];
  int curBufSize2 = bufferSizes[1];
  int curBufSize3 = bufferSizes[2];
  int curBufSize4 = bufferSizes[3];
  int curBufSize5 = bufferSizes[4];

  double a = static_cast < double > (curBufSize3 - curBufSize1) / 2.0;
  double b = static_cast < double > (curBufSize4 - curBufSize2) / 2.0;
  double c = static_cast < double > (curBufSize5 - curBufSize3) / 2.0;

  double interBufferSize1 = static_cast < double > (b - a);
  double interBufferSize2 = static_cast < double > (c - b);

  //std::cout << "a " << std::setprecision(2) << std::fixed << a << std::endl;
  //std::cout << "b " << std::setprecision(2) << std::fixed << b << std::endl;
  //std::cout << "c " << std::setprecision(2) << std::fixed << c << std::endl;
  //std::cout << "interBufferSize1 "<< interBufferSize1 << std::endl;
  //std::cout << "interBufferSize2 "<< interBufferSize2 << std::endl;

  double interBufferSize = (interBufferSize2 - interBufferSize1);
  //std::cout << "Third Deri ratio "<<interBufferSize << std::endl;
  m_ql_ratio1 = static_cast < double > (curBufSize2 - curBufSize1);
  m_ql_ratio2 = interBufferSize1;
  m_ql_ratio3 = interBufferSize;

  m_currentIndex = 0;
}

void dAQMQueueDisc::UpdateCurrentBufferSize() {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast < std::chrono::milliseconds > (now - m_lastSampleTime);

  if (elapsed.count() >= 10) {

    bufferSizes[m_currentIndex] = GetInternalQueue(0) -> GetNPackets();

    m_currentIndex = (m_currentIndex + 1) % 5;

    if (m_currentIndex == 0) {
      Get_QL_Deri_Ratio();
    }
    m_lastSampleTime = now;
  }

}

bool dAQMQueueDisc::CheckConfig(void) {
  NS_LOG_FUNCTION(this);
  if (GetNQueueDiscClasses() > 0) {
    NS_LOG_ERROR("dAQMQueueDisc cannot have classes");
    return false;
  }

  if (GetNPacketFilters() > 0) {
    NS_LOG_ERROR("dAQMQueueDisc needs no packet filter");
    return false;
  }

  if (GetNInternalQueues() == 0) {
    AddInternalQueue(CreateObjectWithAttributes < DropTailQueue < QueueDiscItem > >
      ("MaxSize", QueueSizeValue(GetMaxSize())));
  }

  if (GetNInternalQueues() != 1) {
    NS_LOG_ERROR("dAQMQueueDisc needs 1 internal queue");
    return false;
  }

  return true;

}
void dAQMQueueDisc::InitializeParams(void) {
  NS_LOG_FUNCTION(this);
}

} // namespace ns3
