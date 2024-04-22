#ifndef DAQM_QUEUE_DISC_H
#define DAQM_QUEUE_DISC_H
#include "ns3/queue.h"
#include "ns3/traffic-control-module.h"
#include "ns3/queue-disc.h"
#include <vector>
#include <deque>
#include <chrono>

namespace ns3 {

  /**
   * Custom queue discipline (dAQM) for managing packet drops based on 
   * inter-packet arrival time difference, and higher-order derivatives
   * of sojourn time, and buffer size.
   */
  class dAQMQueueDisc: public QueueDisc {
    public: static TypeId GetTypeId(void);

    // Default constructor.
    dAQMQueueDisc();
    // Destructor.
    virtual~dAQMQueueDisc();

    /**
     * Enqueues a packet into the internal queue.
     *
     * This function tries to enqueue a packet into the internal queue,
     * while performing checks to determine if the packet should be dropped.
     * It also logs the state of the queue before and after the enqueue operation.
     * 
     * @param item The packet to be enqueued.
     * @return true if the packet is enqueued successfully; false otherwise.
     */
    virtual bool DoEnqueue(Ptr < QueueDiscItem > item);

    /**
     * Dequeues a packet from the internal queue.
     *
     * This function dequeues a packet from the internal queue and performs checks
     * to decide whether to drop the packet based on the if-else conditions. 
     * It also logs the state of the queue before and after the dequeue operation.
     * 
     * @return A pointer to the dequeued packet, or nullptr if the queue is empty.
     */

    virtual Ptr < QueueDiscItem > DoDequeue(void);

    virtual Ptr <
    const QueueDiscItem > DoPeek(void) const;

    /**
     * Retrieves the waiting time for the oldest packet in the queue.
     *
     * This function computes the waiting time of a packet in the queue based on its enqueue timestamp 
     * and the current simulation time. If the queue is empty, a zero duration is returned. 
     * Otherwise, it dequeues the packet, calculates the waiting time, and returns it.
     * 
     * @return Returns the waiting time of the packet. If the queue is empty, returns a zero duration.
     */
    Time GetPacketWaitTime() const;

    virtual void DoDispose(void);

    /**
     * Stops the packet dropping phase.
     *
     * This function terminates the packet dropping phase
     * by setting the internal flag, m_dropPackets, to false. 
     * It also logs the time at which the dropping phase stops.
     */
    virtual void StopDroppingPackets();

    /**
     * Calculates the first, second, and third derivative ratios for the buffer size.
     *
     * This function calculates the first, second, and third derivative ratios of the buffer size based on past measurements.
     * It stores these calculated ratios in member variables (m_ql_ratio1, m_ql_ratio2, m_ql_ratio3) for later use in packet dropping decisions.
     * 
     */
    virtual void Get_QL_Deri_Ratio();

    /**
     * Updates the current buffer size about every 10 milliseconds.
     *
     * This function updates the current buffer size at regular intervals (about every 10ms). 
     * It updates the bufferSizes array at the current index, cycles the index within the range [0, 4], 
     * and then calculates the derivative ratios if the index cycles back to 0.
     *
     */
    virtual void UpdateCurrentBufferSize();

    /**
     * Implements the packet dropping logic for the dAQM algorithm.
     *
     * This function serves as the core of the dAQM algorithm for queue management.
     * It decides whether a packet should be dropped based on various conditions,
     * including the first, second, and third derivatives of sojourn time and buffer size.
     * 
     * @param item Pointer to the packet that is being considered for dropping.
     * @return Returns true if the packet should not be dropped; false otherwise.
     */
    bool DropPacket(Ptr < QueueDiscItem > item);

    static constexpr
    const char * TIME_EXCEEDED_DROP = "Time exceeded drop"; //Sojourn time ratio above target.
    static constexpr
    const char * OVERLIMIT_DROP = "Overlimit drop"; //Overlimit dropped packet.
    static constexpr
    const char * UNFORCED_DROP = "Unforced drop";

    private:

    Time m_sj_dropThreshold; //Constant sojourn time threshold for droppig packets.
    double m_sj_dropThresholdi; // Sojourn time 1st deri ratio threshold for dropping packets.
    double m_sj_dropThresholdii; // Sojourn time 2nd deri ratio threshold for dropping packets.
    double m_sj_dropThresholdiii; // Sojourn time 3rd deri ratio threshold for dropping packets.
    double m_ql_dropThreshold; //Constant buffer size threshold for dropping packets.
    double m_ql_dropThresholdi; // Buffer size 1st deri ratio threshold for dropping packets.
    double m_ql_dropThresholdii; // Buffer size 2nd deri ratio threshold for dropping packets.
    double m_ql_dropThresholdiii; // Buffer size 3rd deri ratio threshold for dropping packets.

    Time m_dropDuration; // Duration for dropping packets (ms).
    Time m_packetInterval; // Inter arrival time of packets (ms).
    Time m_target;
    std::chrono::steady_clock::time_point m_lastSampleTime;

    /**
     * Calculates the first derivative of sojourn time ratio between two consecutive packets.
     * 
     * @return Returns the first derivative of the sojourn time ratio.
     */
    double Get_SJ_First_Deri_Ratio();

    /**
     * Calculates the second derivative of sojourn time ratio between four consecutive packets.
     * 
     * @return Returns the second derivative of the sojourn time ratio.
     */
    double Get_SJ_Second_Deri_Ratio();

    /**
     * Calculates the third derivative of sojourn time ratio between five consecutive packets.
     * 
     * @return Returns the third derivative of the sojourn time ratio.
     */
    double Get_SJ_Third_Deri_Ratio();

    virtual bool CheckConfig(void);
    virtual void InitializeParams(void);
    bool m_dropPackets;
    Ptr < UniformRandomVariable > m_rng;
    bool droppedSJFirst;
    bool droppedSJSecond;
    bool droppedSJThird;
    bool droppedSJConstant;
    bool droppedQLFirst;
    bool droppedQLSecond;
    bool droppedQLThird;
    bool droppedQLConstant;
    bool flag;
    Time packet1WaitTime;
    Time packet2WaitTime;
    Time packet3WaitTime;
    Time packet4WaitTime;
    Time packet5WaitTime;
    double m_dropRate;
    std::array < int,
    5 > bufferSizes; // Array to store the buffer sizes; updated about every 10ms.
    uint32_t m_currentNumPackets;
    int m_currentIndex;
    double m_ql_ratio1;
    double m_ql_ratio2;
    double m_ql_ratio3;
  };

} // namespace ns3

#endif /* TRAFFIC_CONTROL_H */
