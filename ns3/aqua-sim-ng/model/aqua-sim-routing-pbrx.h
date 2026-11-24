/*
 * PBR-family routing for Aqua-Sim NG (single module, several protocols selected by "Mode").
 *
 *   Mode 0  DBR          (Yan et al. 2008) receiver-based, holding time by depth
 *   Mode 1  RE-PBR       (Khasawneh et al. 2018) min route cost, single forwarder
 *   Mode 2  SPRE-PBR     (Khasawneh et al. 2020) route cost + 3 layers + Threshold 4
 *   Mode 3  SPRE-PBR-near  variant preferring shorter links
 *   Mode 4  APCDBRP      (Wang et al. 2023) power control, implicit updates, probe recovery
 *   Mode 6  EEDBR        (Wahid & Kim 2012) sender-based forwarding list ordered by residual energy,
 *                        holding time by residual energy + doubling priority value, overhearing suppression
 *   Mode 7  WDFAD-DBR    (Yu et al. 2016) receiver-based, holding time by the weighted depth difference of
 *                        two hops, Reuleaux-triangle primary area + auxiliary sub-areas suppressed by density,
 *                        neighbour table from overheard packets + NEIGHBOR REQUEST/ACK, distance prediction
 *   Mode 8  HydroCast    (Noh et al. 2016) sender picks a forwarding set by greedy clustering that maximises the
 *                        expected packet advance (EPA) among mutually audible upward neighbours; timer alpha(R - dP),
 *                        ACK suppression, ARQ; local minima keep a recovery path found by 2D void-floor flooding
 *   Mode 9  DVOR         (Guan et al. 2019) sinks flood periodic queries that set hop counts; receivers wait by the
 *                        hop-count difference (Eq. 1) and drop on hearing a copy from a node with hop count <= theirs
 *   Mode 10 QLFR         (Zhou et al. 2020) Q-learning priority list of shallower neighbours (energy + depth reward),
 *                        holding time k(n - 1), V-value piggybacked, list length adapted from the sinks' delivery ratio
 *   Mode 11 HFS-PBR      (Yan & Xiao 2023) sender names a best forwarding node (BFN, Eq. 1) that forwards at once;
 *                        receivers that know the BFN hold by Eq. 3 (AFN); others (SAN) forward with prob. 1/(n+1)
 *                        if their angle to the sink is smaller than the sender's
 *
 * Traffic is generated inside the module (network-wide Poisson, rate Lambda) so all
 * protocols see identical sources; statistics are kept in static counters.
 */
#ifndef AQUA_SIM_ROUTING_PBRX_H
#define AQUA_SIM_ROUTING_PBRX_H

#include "aqua-sim-routing.h"
#include "aqua-sim-address.h"
#include "ns3/header.h"
#include "ns3/tag.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"
#include <map>
#include <set>
#include <deque>
#include <vector>
#include <cstring>
#include <string>

namespace ns3 {

/* Per-packet transmit range / power, read by AquaSimPhyCmn (power control). */
class PbrxTxTag : public Tag
{
public:
  PbrxTxTag () : m_range (-1), m_power (0) {}
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const { return 16; }
  virtual void Serialize (TagBuffer i) const { i.WriteDouble (m_range); i.WriteDouble (m_power); }
  virtual void Deserialize (TagBuffer i) { m_range = i.ReadDouble (); m_power = i.ReadDouble (); }
  virtual void Print (std::ostream &os) const { os << "range=" << m_range << " power=" << m_power; }
  double m_range;
  double m_power;
};

class PbrxHeader : public Header
{
public:
  enum { DATA = 0, HELLO = 1, ACK = 2, PROBE = 3, PREPLY = 4, HOLE = 5, BEACON = 6, SINKACK = 7, RDISC = 8, RREP = 9 };
  PbrxHeader ();
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint8_t type;
  uint16_t sender;      // node index of the transmitter
  uint16_t src;         // origin (node index)
  uint32_t seq;
  double gen;           // generation time (s)
  double depth;         // sender depth (pressure)
  double energy;        // sender residual energy
  double succ;          // sender forwarding success ratio (APCDBRP)
  double txRange;       // range/power level used for this transmission
  double ival;          // hello: interval until the sender's next hello (neighbour expiry)
  uint8_t ok;           // sender has a route (void flag)
  double vval;          // QLFR: sender V-value
  std::vector<uint16_t> relays;          // forwarding list / intended receiver
  std::vector<std::pair<uint16_t, float>> nbrs;   // HydroCast beacon / recovery route: neighbour ids + distance
};

struct PbrxNbr
{
  double t = -1, depth = 0, dist = 0, energy = 0, succ = 1, ival = 0;
  double v = 0;                             // QLFR V-value
  bool ok = true;
  std::map<uint16_t, float> nbrs;           // 2-hop info (HydroCast)
};

class AquaSimPbrx : public AquaSimRouting
{
public:
  static TypeId GetTypeId (void);
  AquaSimPbrx ();
  virtual ~AquaSimPbrx ();
  virtual bool Recv (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
  virtual int64_t AssignStreams (int64_t stream);

  void Start (uint16_t index, bool isSink);

  // ---- global (static) state shared by all nodes of one run
  static std::vector<AquaSimPbrx*> s_nodes;   // by index
  static uint16_t s_numSensors;
  static uint64_t s_gen, s_deliv, s_tx, s_probe, s_hello, s_dup;
  static double s_delaySum, s_ctrlEnergy;
  static std::set<std::pair<uint16_t, uint32_t>> s_delivered;
  static uint64_t s_txType[16], s_retx, s_dropRetx, s_dropVoid, s_rxOk, s_rxLinkLoss;
  static uint64_t s_winGen, s_winDel, s_holdFwd;
  static uint64_t s_phyRxN, s_phyRxErr; static double s_phyRxT, s_phyTxT;
  static void ResetStats ();
  static void Report ();
  static std::map<std::pair<uint16_t, uint32_t>, std::string> s_last;
  static const char *s_why;
  static uint64_t s_dupAck;

  double ResidualEnergy ();
  bool Dead ();
  double Depth ();
  Vector Pos ();
  bool m_isSink;
  uint16_t m_index;

  // APCDBRP global recovery (sinks share statistics): called by the scenario
  void Flood (bool fromSink);
  void GenData ();
  void Tick ();          // called every 10 s by the scenario (protocol timers)
  int Mode () const { return m_mode; }
  bool NoAck () const { return m_mode == 0 || m_mode == 6 || m_mode == 7 || m_mode == 9 || m_mode == 10 || m_mode == 11; }   // receiver-contention protocols: no ACK / retransmission
  static double s_eedbrTarget;   // EEDBR desired delivery ratio (0 = always suppress)
  void EedbrNotify (double ratio);   // sink: flood the measured delivery ratio

private:
  // configuration
  int m_mode;
  double m_R, m_helloInt, m_retime, m_slot, m_tPkt, m_sigmaD, m_txPowerW, m_e0;
  bool m_acoustic;
  bool m_near = false;   // RE-PBR/SPRE-PBR: link-quality term decreasing with distance (faithful to the triangle metric)
  double m_SL, m_freq, m_rate;
  uint32_t m_dataBytes, m_helloBytes, m_ackBytes, m_ctrlBytes;
  int m_retx;
  double m_psrTh;

  Ptr<UniformRandomVariable> m_uni;
  Ptr<NormalRandomVariable> m_norm;

  std::map<uint16_t, PbrxNbr> m_nit;
  std::set<std::pair<uint16_t, uint32_t>> m_seen;
  std::map<std::pair<uint16_t, uint32_t>, EventId> m_holds;
  std::map<std::pair<uint16_t, uint32_t>, uint16_t> m_holdFrom;
  std::map<std::pair<uint16_t, uint32_t>, PbrxHeader> m_holdPkt;
  std::set<std::pair<uint16_t, uint32_t>> m_custody;   // packets this node took responsibility for
  static uint32_t s_floodId;

  // sender-side stop-and-wait
  struct Pending { PbrxHeader h; int attempt = 0; int rec = 0; std::set<uint16_t> excl; bool active = false; };
  Pending m_pend;
  std::deque<PbrxHeader> m_queue;
  EventId m_ackTimer;
  double m_okCount = 0, m_totCount = 0;     // APCDBRP forwarding success ratio
  bool m_hole = false;
  double m_lastHelloE = 0; bool m_lastOk = false;
  double m_lastData = -1;
  EventId m_helloEvent;

  // helpers
  void Transmit (PbrxHeader h, uint32_t bytes, double range);
  void ScheduleHello (double delay);
  void SendHello ();
  void TrySend ();
  void SendCurrent ();
  void AckTimeout (uint32_t token);
  void Confirmed ();
  void HoldExpire (PbrxHeader h);
  void QueueAck (const PbrxHeader &h);
  void Observe (const PbrxHeader &h, double dist);
  bool HasRoute ();
  void Deliver (const PbrxHeader &h);
  void Monitor ();
  double Psr (double d, double range, double bits = 512);
  bool Fresh (const PbrxNbr &e, double now) const;
  void DropCustody (const PbrxHeader &h);
  double RangeFor (double d);
  double Gain (double range);
  bool ChooseRelays (PbrxHeader &h, double &range);
  std::vector<std::tuple<uint16_t, double, double, double, bool, double>> View ();
  uint16_t Probe ();
  uint32_t m_token = 0;
  // EEDBR
  double m_eedbrMaxHold = 2.0, m_eedbrP0 = 0.2, m_eedbrETh = 0.1, m_eedbrDr = 1.0;
  std::set<std::pair<uint16_t, uint32_t>> m_eedbrKeep;
  // WDFAD-DBR
  double m_wAlpha = 0.5, m_wDelta = 0, m_wP = 0.99, m_speed = 0, m_lastNreq = -1e9;
  int m_wLevels = 4;
  void WdfadNreq ();
  void WdfadLearn (const PbrxHeader &h, double dEst);
  double WdfadNextDepth (int *count = nullptr);
  bool WdfadInForwardArea (double h, double d);
  // HydroCast
  double m_hcGamma = 0.032, m_hcBeta = 0.5, m_hcHello = 30, m_hcPathLife = 300, m_hcHoldMax = 0;
  int m_hcHopLimit = 1, m_hcRetx = 5, m_hcVectors = 20;
  std::vector<uint16_t> m_hcPath; double m_hcPathT = -1e9, m_hcLastDisc = -1e9, m_hcSurfT = -1e9;
  bool m_hcSurf = true; uint32_t m_hcDiscSeq = 0; int m_hcFail = 0;
  double HcBackoff () const { return 60.0 * std::pow (2.0, std::min (m_hcFail, 4)); }
  bool HcHasUp ();
  bool HcPathValid ();
  bool HcSurface ();
  void HcDiscover ();
  void HcOnDisc (const PbrxHeader &h);
  void HcOnRrep (const PbrxHeader &h);
  bool HcChoose (PbrxHeader &h, std::vector<std::tuple<uint16_t, double, double, double, bool, double>> &up);
  // DVOR
  double m_dvN = 1e9, m_dvT = -1e9, m_dvPeriod = 20, m_dvSlot = 0.01; uint32_t m_dvQid = 0; int m_dvCW = 31;
  double DvN ();
  // QLFR
  std::map<uint16_t, double> m_q; double m_qV = 0, m_qAlpha = 0.5, m_qGamma = 0.8, m_qH = 1; int m_qL = 3;
  // HFS-PBR
  double m_hfBeta = 1.0, m_hfAlpha = 0.8;
  double HfAngle ();
  int HfBfn (double *depth = nullptr);
public:
  void DvorQuery (uint32_t qid);
  static double s_qlfrPth;
  static bool s_noRouteBit;       // sensitivity: ignore the framework has-route bit in candidate filtering
  static bool s_noSinkShortcut;   // sensitivity: sinks are ordinary candidates (no nearest-sink shortcut)
};

}  // namespace ns3
#endif
