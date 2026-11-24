/*
 * PBR-family routing for Aqua-Sim NG — see aqua-sim-routing-pbrx.h
 */
#include "aqua-sim-routing-pbrx.h"
#include "aqua-sim-header.h"
#include "aqua-sim-net-device.h"
#include "aqua-sim-energy-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include <cmath>
#include <algorithm>
#include <tuple>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AquaSimPbrx");
NS_OBJECT_ENSURE_REGISTERED (AquaSimPbrx);
NS_OBJECT_ENSURE_REGISTERED (PbrxHeader);
NS_OBJECT_ENSURE_REGISTERED (PbrxTxTag);

// ------------------------------------------------------------------ tag / header
TypeId PbrxTxTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PbrxTxTag").SetParent<Tag> ().AddConstructor<PbrxTxTag> ();
  return tid;
}
TypeId PbrxTxTag::GetInstanceTypeId (void) const { return GetTypeId (); }

PbrxHeader::PbrxHeader ()
  : type (DATA), sender (0), src (0), seq (0), gen (0), depth (0), energy (0), succ (1),
    txRange (0), ival (0), ok (1), vval (0) {}

TypeId PbrxHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PbrxHeader").SetParent<Header> ().AddConstructor<PbrxHeader> ();
  return tid;
}
TypeId PbrxHeader::GetInstanceTypeId (void) const { return GetTypeId (); }

uint32_t PbrxHeader::GetSerializedSize (void) const
{
  return 1 + 2 + 2 + 4 + 8 * 7 + 1 + 1 + 2 * relays.size () + 2 + 6 * nbrs.size ();
}

void PbrxHeader::Serialize (Buffer::Iterator i) const
{
  auto wd = [&] (double v) { uint64_t u; memcpy (&u, &v, 8); i.WriteHtonU64 (u); };
  i.WriteU8 (type); i.WriteHtonU16 (sender); i.WriteHtonU16 (src); i.WriteHtonU32 (seq);
  wd (gen); wd (depth); wd (energy); wd (succ); wd (txRange); wd (ival); wd (vval);
  i.WriteU8 (ok);
  i.WriteU8 ((uint8_t) relays.size ());
  for (auto r : relays) i.WriteHtonU16 (r);
  i.WriteHtonU16 ((uint16_t) nbrs.size ());
  for (auto &n : nbrs) { i.WriteHtonU16 (n.first); float f = n.second; uint32_t u; memcpy (&u, &f, 4); i.WriteHtonU32 (u); }
}

uint32_t PbrxHeader::Deserialize (Buffer::Iterator i)
{
  auto rd = [&] () { uint64_t u = i.ReadNtohU64 (); double v; memcpy (&v, &u, 8); return v; };
  type = i.ReadU8 (); sender = i.ReadNtohU16 (); src = i.ReadNtohU16 (); seq = i.ReadNtohU32 ();
  gen = rd (); depth = rd (); energy = rd (); succ = rd (); txRange = rd (); ival = rd (); vval = rd ();
  ok = i.ReadU8 ();
  uint8_t n = i.ReadU8 ();
  relays.clear ();
  for (int k = 0; k < n; k++) relays.push_back (i.ReadNtohU16 ());
  uint16_t m = i.ReadNtohU16 ();
  nbrs.clear ();
  for (int k = 0; k < m; k++)
    {
      uint16_t id = i.ReadNtohU16 (); uint32_t u = i.ReadNtohU32 (); float f; memcpy (&f, &u, 4);
      nbrs.push_back ({id, f});
    }
  return GetSerializedSize ();
}

void PbrxHeader::Print (std::ostream &os) const
{
  os << "type=" << (int) type << " sender=" << sender << " src=" << src << " seq=" << seq;
}

// ------------------------------------------------------------------ statics
std::vector<AquaSimPbrx*> AquaSimPbrx::s_nodes;
uint16_t AquaSimPbrx::s_numSensors = 0;
uint64_t AquaSimPbrx::s_gen = 0, AquaSimPbrx::s_deliv = 0, AquaSimPbrx::s_tx = 0,
         AquaSimPbrx::s_probe = 0, AquaSimPbrx::s_hello = 0, AquaSimPbrx::s_dup = 0;
double AquaSimPbrx::s_delaySum = 0, AquaSimPbrx::s_ctrlEnergy = 0;
std::set<std::pair<uint16_t, uint32_t>> AquaSimPbrx::s_delivered;
std::map<std::pair<uint16_t, uint32_t>, std::string> AquaSimPbrx::s_last;
std::map<std::pair<uint16_t, uint32_t>, std::string> s_hist;
#define TRK(h, what) do { if (s_trk) { s_last[std::make_pair ((h).src, (h).seq)] = (what); \
  char b_[96]; snprintf (b_, 96, " %.2f:%u:%s", Simulator::Now ().GetSeconds (), m_index, std::string (what).c_str ()); \
  s_hist[std::make_pair ((h).src, (h).seq)] += b_; } } while (0)
static bool s_trk = getenv ("PBRX_TRK") != nullptr;
uint64_t AquaSimPbrx::s_txType[16] = {0}, AquaSimPbrx::s_retx = 0, AquaSimPbrx::s_dropRetx = 0,
         AquaSimPbrx::s_dropVoid = 0, AquaSimPbrx::s_rxOk = 0, AquaSimPbrx::s_rxLinkLoss = 0;
uint64_t AquaSimPbrx::s_winGen = 0, AquaSimPbrx::s_winDel = 0, AquaSimPbrx::s_holdFwd = 0;
uint64_t AquaSimPbrx::s_phyRxN = 0, AquaSimPbrx::s_phyRxErr = 0; double AquaSimPbrx::s_phyRxT = 0, AquaSimPbrx::s_phyTxT = 0;

void AquaSimPbrx::ResetStats ()
{
  s_nodes.clear (); s_numSensors = 0;
  s_gen = s_deliv = s_tx = s_probe = s_hello = s_dup = 0;
  s_delaySum = s_ctrlEnergy = 0; s_delivered.clear (); s_winGen = s_winDel = 0;
  for (auto &x : s_txType) x = 0;
  s_retx = s_dropRetx = s_dropVoid = s_rxOk = s_rxLinkLoss = 0;
}

TypeId AquaSimPbrx::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AquaSimPbrx")
    .SetParent<AquaSimRouting> ()
    .AddConstructor<AquaSimPbrx> ()
    .AddAttribute ("Mode", "0 DBR, 1 RE-PBR, 2 SPRE-PBR, 3 SPRE-PBR-near, 4 APCDBRP, 6 EEDBR, 7 WDFAD-DBR, 8 HydroCast, 9 DVOR, 10 QLFR, 11 HFS-PBR",
                   IntegerValue (0), MakeIntegerAccessor (&AquaSimPbrx::m_mode), MakeIntegerChecker<int> ())
    .AddAttribute ("Range", "Max range (m)", DoubleValue (250), MakeDoubleAccessor (&AquaSimPbrx::m_R), MakeDoubleChecker<double> ())
    .AddAttribute ("HelloInterval", "Fixed hello interval (s)", DoubleValue (100), MakeDoubleAccessor (&AquaSimPbrx::m_helloInt), MakeDoubleChecker<double> ())
    .AddAttribute ("Retime", "ACK timeout (s)", DoubleValue (1.0), MakeDoubleAccessor (&AquaSimPbrx::m_retime), MakeDoubleChecker<double> ())
    .AddAttribute ("SigmaD", "Ranging error (m)", DoubleValue (5.0), MakeDoubleAccessor (&AquaSimPbrx::m_sigmaD), MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerW", "Full transmit power (W)", DoubleValue (2.0), MakeDoubleAccessor (&AquaSimPbrx::m_txPowerW), MakeDoubleChecker<double> ())
    .AddAttribute ("E0", "Initial energy (J)", DoubleValue (100.0), MakeDoubleAccessor (&AquaSimPbrx::m_e0), MakeDoubleChecker<double> ())
    .AddAttribute ("Acoustic", "true: Thorp/noise PSR per link; false: range (disk) channel",
                   BooleanValue (true), MakeBooleanAccessor (&AquaSimPbrx::m_acoustic), MakeBooleanChecker ())
    .AddAttribute ("SL", "Source level (dB)", DoubleValue (112), MakeDoubleAccessor (&AquaSimPbrx::m_SL), MakeDoubleChecker<double> ())
    .AddAttribute ("FreqKHz", "Carrier (kHz)", DoubleValue (10), MakeDoubleAccessor (&AquaSimPbrx::m_freq), MakeDoubleChecker<double> ())
    .AddAttribute ("Rate", "Bit rate (bps)", DoubleValue (10000), MakeDoubleAccessor (&AquaSimPbrx::m_rate), MakeDoubleChecker<double> ())
    .AddAttribute ("DataBytes", "Data packet size", UintegerValue (64), MakeUintegerAccessor (&AquaSimPbrx::m_dataBytes), MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HelloBytes", "Hello size (bytes)", UintegerValue (64), MakeUintegerAccessor (&AquaSimPbrx::m_helloBytes), MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AckBytes", "ACK size", UintegerValue (8), MakeUintegerAccessor (&AquaSimPbrx::m_ackBytes), MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CtrlBytes", "PROBE/REPLY/BEACON/HOLE size", UintegerValue (64), MakeUintegerAccessor (&AquaSimPbrx::m_ctrlBytes), MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Retx", "Retransmissions", IntegerValue (3), MakeIntegerAccessor (&AquaSimPbrx::m_retx), MakeIntegerChecker<int> ())
    .AddAttribute ("Near", "RE-PBR/SPRE-PBR: link quality decreases with distance (cost term d/dmax instead of 1-d/dmax)", BooleanValue (false), MakeBooleanAccessor (&AquaSimPbrx::m_near), MakeBooleanChecker ())
    .AddAttribute ("EedbrMaxHold", "EEDBR max_holding_time (s)", DoubleValue (2.0), MakeDoubleAccessor (&AquaSimPbrx::m_eedbrMaxHold), MakeDoubleChecker<double> ())
    .AddAttribute ("EedbrP0", "EEDBR priority starting value (s); <= 0: half a contention slot", DoubleValue (0.2), MakeDoubleAccessor (&AquaSimPbrx::m_eedbrP0), MakeDoubleChecker<double> ())
    .AddAttribute ("EedbrETh", "EEDBR residual-energy change (fraction of E0) that triggers a hello", DoubleValue (0.1), MakeDoubleAccessor (&AquaSimPbrx::m_eedbrETh), MakeDoubleChecker<double> ())
    .AddAttribute ("WdfadAlpha", "WDFAD-DBR weighting coefficient alpha (paper: 0.5)", DoubleValue (0.5), MakeDoubleAccessor (&AquaSimPbrx::m_wAlpha), MakeDoubleChecker<double> ())
    .AddAttribute ("WdfadDelta", "WDFAD-DBR delta (m); <= 0: R (paper: delta = R)", DoubleValue (0.0), MakeDoubleAccessor (&AquaSimPbrx::m_wDelta), MakeDoubleChecker<double> ())
    .AddAttribute ("WdfadLevels", "WDFAD-DBR number of density levels / auxiliary sub-areas", IntegerValue (4), MakeIntegerAccessor (&AquaSimPbrx::m_wLevels), MakeIntegerChecker<int> ())
    .AddAttribute ("WdfadP", "WDFAD-DBR required success probability P (Eq. 20)", DoubleValue (0.99), MakeDoubleAccessor (&AquaSimPbrx::m_wP), MakeDoubleChecker<double> ())
    .AddAttribute ("NodeSpeed", "WDFAD-DBR: node speed used by the neighbour prediction (m/s)", DoubleValue (0.0), MakeDoubleAccessor (&AquaSimPbrx::m_speed), MakeDoubleChecker<double> ())
    .AddAttribute ("HcGamma", "HydroCast max allowable alpha (s/m) of a forwarding set", DoubleValue (0.032), MakeDoubleAccessor (&AquaSimPbrx::m_hcGamma), MakeDoubleChecker<double> ())
    .AddAttribute ("HcBeta", "HydroCast clustering radius factor beta (paper: 1/2)", DoubleValue (0.5), MakeDoubleAccessor (&AquaSimPbrx::m_hcBeta), MakeDoubleChecker<double> ())
    .AddAttribute ("HcHello", "HydroCast beacon interval with distance list (s) (paper: 30)", DoubleValue (30), MakeDoubleAccessor (&AquaSimPbrx::m_hcHello), MakeDoubleChecker<double> ())
    .AddAttribute ("HcHopLimit", "HydroCast hop limit of the void-floor route discovery", IntegerValue (1), MakeIntegerAccessor (&AquaSimPbrx::m_hcHopLimit), MakeIntegerChecker<int> ())
    .AddAttribute ("HcRetx", "HydroCast retransmissions (paper: 5)", IntegerValue (5), MakeIntegerAccessor (&AquaSimPbrx::m_hcRetx), MakeIntegerChecker<int> ())
    .AddAttribute ("HcPathLife", "HydroCast recovery path lifetime (s)", DoubleValue (300), MakeDoubleAccessor (&AquaSimPbrx::m_hcPathLife), MakeDoubleChecker<double> ())
    .AddAttribute ("HcVectors", "HydroCast random vectors for void-floor surface detection (paper: 20)", IntegerValue (20), MakeIntegerAccessor (&AquaSimPbrx::m_hcVectors), MakeIntegerChecker<int> ())
    .AddAttribute ("DvorPeriod", "DVOR query period (s) (paper: 20)", DoubleValue (20), MakeDoubleAccessor (&AquaSimPbrx::m_dvPeriod), MakeDoubleChecker<double> ())
    .AddAttribute ("DvorSlot", "DVOR backoff unit tslot (s) (paper: 0.01)", DoubleValue (0.01), MakeDoubleAccessor (&AquaSimPbrx::m_dvSlot), MakeDoubleChecker<double> ())
    .AddAttribute ("DvorCW", "DVOR backoff window (IEEE 802.11 default 31)", IntegerValue (31), MakeIntegerAccessor (&AquaSimPbrx::m_dvCW), MakeIntegerChecker<int> ())
    .AddAttribute ("QlfrAlpha", "QLFR learning rate", DoubleValue (0.5), MakeDoubleAccessor (&AquaSimPbrx::m_qAlpha), MakeDoubleChecker<double> ())
    .AddAttribute ("QlfrGamma", "QLFR discount factor (paper: 0.8)", DoubleValue (0.8), MakeDoubleAccessor (&AquaSimPbrx::m_qGamma), MakeDoubleChecker<double> ())
    .AddAttribute ("QlfrH", "QLFR h in k = 2 t_max / h (Eq. 15)", DoubleValue (1), MakeDoubleAccessor (&AquaSimPbrx::m_qH), MakeDoubleChecker<double> ())
    .AddAttribute ("QlfrL", "QLFR initial priority-list length", IntegerValue (3), MakeIntegerAccessor (&AquaSimPbrx::m_qL), MakeIntegerChecker<int> ())
    .AddAttribute ("HfsBeta", "HFS-PBR angle weight beta in Eq. (1)", DoubleValue (1.0), MakeDoubleAccessor (&AquaSimPbrx::m_hfBeta), MakeDoubleChecker<double> ())
    .AddAttribute ("HfsAlpha", "HFS-PBR two-hop depth weight alpha in Eq. (3)", DoubleValue (0.8), MakeDoubleAccessor (&AquaSimPbrx::m_hfAlpha), MakeDoubleChecker<double> ())
    .AddAttribute ("PsrTh", "APCDBRP reception threshold (PSR)", DoubleValue (0.9), MakeDoubleAccessor (&AquaSimPbrx::m_psrTh), MakeDoubleChecker<double> ());
  return tid;
}

AquaSimPbrx::AquaSimPbrx () : m_isSink (false), m_index (0)
{
  m_uni = CreateObject<UniformRandomVariable> ();
  m_norm = CreateObject<NormalRandomVariable> ();
}
AquaSimPbrx::~AquaSimPbrx () {}

int64_t AquaSimPbrx::AssignStreams (int64_t stream)
{
  m_uni->SetStream (stream); m_norm->SetStream (stream + 1);
  return 2;
}

// ------------------------------------------------------------------ node helpers
Vector AquaSimPbrx::Pos () { return GetNetDevice ()->GetNode ()->GetObject<MobilityModel> ()->GetPosition (); }
double AquaSimPbrx::Depth () { return Pos ().z; }
double AquaSimPbrx::ResidualEnergy ()
{
  if (m_isSink) return m_e0;
  Ptr<AquaSimEnergyModel> em = GetNetDevice ()->EnergyModel ();
  return em ? em->GetEnergy () : m_e0;
}
bool AquaSimPbrx::Dead () { return !m_isSink && ResidualEnergy () <= 0; }

static double Thorp (double f) { double f2 = f * f; return 0.11 * f2 / (1 + f2) + 44 * f2 / (4100 + f2) + 2.75e-4 * f2 + 0.003; }
static double NoiseDb (double f)
{
  double lf = std::log10 (f);
  double v[4] = {17 - 30 * lf, 40 + 26 * lf - 60 * std::log10 (f + .03), 50 + 20 * lf - 40 * std::log10 (f + .4), -15 + 20 * lf};
  double s = 0; for (double x : v) s += std::pow (10, x / 10);
  return 10 * std::log10 (s);
}
static double TL (double d, double f) { d = std::max (d, 1.0); return 15 * std::log10 (d) + d / 1000 * Thorp (f); }

double AquaSimPbrx::Gain (double range) { return range >= m_R ? 1.0 : std::pow (10, (TL (range, m_freq) - TL (m_R, m_freq)) / 10); }

double AquaSimPbrx::Psr (double d, double range, double bits)
{
  if (d > range + 1e-9) return 0.0;
  if (!m_acoustic) return 0.98;
  double sl = m_SL - (TL (m_R, m_freq) - TL (range, m_freq));
  double snr = std::pow (10, (sl - TL (d, m_freq) - NoiseDb (m_freq) - 10 * std::log10 (m_rate)) / 10);
  double ber = 0.5 * std::erfc (std::sqrt (snr));
  return std::pow (1 - ber, bits);
}

double AquaSimPbrx::RangeFor (double d)
{
  if (!m_acoustic) return std::min (m_R, d + 1.0);
  double lo = d, hi = m_R;
  if (Psr (d, hi) < m_psrTh) return hi;
  for (int k = 0; k < 18; k++) { double mid = (lo + hi) / 2; if (Psr (d, mid) >= m_psrTh) hi = mid; else lo = mid; }
  return hi;
}

// ------------------------------------------------------------------ start / traffic
void AquaSimPbrx::Start (uint16_t index, bool isSink)
{
  m_index = index; m_isSink = isSink;
  if (s_nodes.size () <= index) s_nodes.resize (index + 1, nullptr);
  s_nodes[index] = this;
  m_tPkt = m_dataBytes * 8.0 / m_rate;
  // primary must receive (Aqua-Sim hands a packet up after 2 airtimes), contend (MAC back-off up to 0.4 s) and be heard back
  m_slot = 2 * m_tPkt + 2 * m_R / 1500.0 + 0.4;
  m_lastHelloE = m_e0;
  if (m_eedbrP0 <= 0) m_eedbrP0 = m_slot / 2;   // second node in the list waits one full slot (value doubles per position)
  if (m_wDelta <= 0) m_wDelta = m_R;
  if (m_mode == 7)
    {   // WDFAD-DBR: no hello; the neighbour table is set up by one NEIGHBOR REQUEST round, then kept from overheard packets
      if (!isSink) Simulator::Schedule (Seconds (m_uni->GetValue (0, 60)), &AquaSimPbrx::WdfadNreq, this);
      return;
    }
  if (m_mode == 0 || m_mode == 4 || m_mode == 9) return;   // DBR: no hello; APCDBRP: beacon flood; DVOR: sink queries
  if (isSink) { Simulator::Schedule (Seconds (m_uni->GetValue (0, 10)), &AquaSimPbrx::SendHello, this); return; }
  ScheduleHello (m_uni->GetValue (0, 10));
}

void AquaSimPbrx::GenData ()
{
  if (m_isSink || Dead ()) return;
  static std::map<uint16_t, uint32_t> seqs;
  PbrxHeader h;
  h.type = PbrxHeader::DATA; h.src = m_index; h.seq = seqs[m_index]++;
  h.gen = Simulator::Now ().GetSeconds ();
  s_gen++; s_winGen++;
  TRK (h, "gen");
  m_seen.insert ({h.src, h.seq});
  m_custody.insert ({h.src, h.seq});
  m_queue.push_back (h);
  TrySend ();
}

// ------------------------------------------------------------------ transmit
void AquaSimPbrx::Transmit (PbrxHeader h, uint32_t bytes, double range)
{
  if (Dead ()) return;
  h.sender = m_index; h.depth = Depth (); h.energy = ResidualEnergy (); h.vval = (m_mode == 11) ? HfAngle () : m_qV;
  h.succ = m_totCount > 0 ? m_okCount / m_totCount : 1.0;
  h.txRange = range;
  h.ok = (m_mode == 4) ? !m_hole : HasRoute ();
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  AquaSimHeader ash;
  ash.SetSize (bytes);
  ash.SetSAddr (AquaSimAddress::ConvertFrom (GetNetDevice ()->GetAddress ()));
  ash.SetDAddr (AquaSimAddress::GetBroadcast ());
  ash.SetNextHop (AquaSimAddress::GetBroadcast ());
  ash.SetDirection (AquaSimHeader::DOWN);
  ash.SetNumForwards (1);
  ash.SetTimeStamp (Simulator::Now ());
  p->AddHeader (ash);
  PbrxTxTag tag; tag.m_range = range; tag.m_power = m_isSink ? 1e-9 : m_txPowerW * Gain (range);
  p->AddPacketTag (tag);
  if (h.type == PbrxHeader::DATA) { s_tx++; m_lastData = Simulator::Now ().GetSeconds (); }
  if (h.type < 16 && !m_isSink) s_txType[h.type]++;
  SendDown (p, AquaSimAddress::GetBroadcast (), Seconds (0));
}

// ------------------------------------------------------------------ hello / NIT
void AquaSimPbrx::ScheduleHello (double delay)
{
  m_helloEvent.Cancel ();
  m_helloEvent = Simulator::Schedule (Seconds (delay), &AquaSimPbrx::SendHello, this);
}

void AquaSimPbrx::SendHello ()
{
  if (Dead ()) return;
  PbrxHeader h; h.type = PbrxHeader::HELLO;
  double now = Simulator::Now ().GetSeconds ();
  double next = m_isSink ? 20.0 : (m_mode == 8 ? m_hcHello + m_uni->GetValue (-3, 3) : m_helloInt);
  if (now < 90) next = 10 + m_uni->GetValue (-2, 2);   // bootstrap rounds (all protocols)
  h.ival = next;                                        // lets neighbours expire this entry after 3 missed hellos
  uint32_t bytes = m_helloBytes;
  if (m_mode == 8 && !m_isSink)
    {   // HydroCast beacon: neighbour list: 2-byte id + 1-byte quantised distance per fresh entry
      for (auto &e : m_nit) if (Fresh (e.second, now)) h.nbrs.push_back ({e.first, (float) e.second.dist});
      bytes = m_helloBytes + 3 * (uint32_t) h.nbrs.size ();
    }
  Transmit (h, m_isSink ? 16 : bytes, m_R);
  if (!m_isSink) { s_hello++; m_lastHelloE = ResidualEnergy (); m_lastOk = HasRoute (); }
  if (m_isSink) Simulator::Schedule (Seconds (next), &AquaSimPbrx::SendHello, this);
  else ScheduleHello (next);
}

// a neighbour expires after three missed hellos of its own advertised interval (same rule for every protocol)
bool AquaSimPbrx::Fresh (const PbrxNbr &e, double now) const
{
  double iv = e.ival > 0 ? e.ival : m_helloInt;
  return now - e.t <= 3 * iv;
}

void AquaSimPbrx::Observe (const PbrxHeader &h, double dEst)
{
  double now = Simulator::Now ().GetSeconds ();
  bool isSinkSender = h.sender >= s_numSensors;
  if (m_mode == 4 && !(m_nit.count (h.sender) || h.depth < Depth ())) return;
  PbrxNbr &e = m_nit[h.sender];
  e.t = now; e.depth = h.depth; e.dist = dEst; e.energy = isSinkSender ? m_e0 : h.energy;
  if (h.type == PbrxHeader::HELLO) e.ival = h.ival;
  e.succ = h.succ; e.ok = isSinkSender ? true : (bool) h.ok; e.v = isSinkSender ? 0.0 : h.vval;
  if (!h.nbrs.empty ()) { e.nbrs.clear (); for (auto &n : h.nbrs) e.nbrs[n.first] = n.second; }
}

bool AquaSimPbrx::HasRoute ()
{
  if (m_isSink) return true;
  double now = Simulator::Now ().GetSeconds (), d = Depth ();
  if (m_mode == 8 && HcPathValid ()) return true;   // a local minimum with a recovery path can still deliver
  for (auto &e : m_nit)
    {
      if (!Fresh (e.second, now)) continue;
      if (e.second.depth < d && e.second.ok && e.second.dist <= m_R) return true;
    }
  return false;
}

// candidates: id, depth, dist, energy, ok, succ
std::vector<std::tuple<uint16_t, double, double, double, bool, double>> AquaSimPbrx::View ()
{
  std::vector<std::tuple<uint16_t, double, double, double, bool, double>> out;
  double now = Simulator::Now ().GetSeconds ();
  for (auto &kv : m_nit)
    {
      const PbrxNbr &e = kv.second;
      if (!Fresh (e, now)) continue;
      out.push_back (std::make_tuple (kv.first, e.depth, std::max (e.dist, 1.0), e.energy, e.ok, e.succ));
    }
  return out;
}

// ------------------------------------------------------------------ relay selection
bool AquaSimPbrx::ChooseRelays (PbrxHeader &h, double &range)
{
  h.relays.clear ();
  range = m_R;
  double my = Depth ();
  uint16_t N = s_numSensors;

  if (m_mode == 0) return true;   // DBR: receiver-based
  if (m_mode == 9)
    {   // DVOR: no candidate list; a node without a valid hop count keeps silent
      double n = DvN ();
      if (n >= 1e8) return false;
      h.ival = n;
      return true;
    }
  if (m_mode == 8)
    {
      if (h.ival < 0 && !h.nbrs.empty ())
        {   // packet in recovery mode: next hop of the source route
          h.relays.push_back (h.nbrs.front ().first); h.nbrs.erase (h.nbrs.begin ());
          m_hcHoldMax = 0; return true;
        }
      h.nbrs.clear (); h.ival = 1e-4; m_hcHoldMax = 0;
    }

  if (m_mode == 4)   // APCDBRP
    {
      double best = -1, bestR = m_R; int bj = -1; double etamax = 0;
      std::vector<std::tuple<uint16_t, double, double, double, double>> c;   // id, r, eta, E, p
      for (auto &kv : m_nit)
        {
          const PbrxNbr &e = kv.second;
          if (e.depth >= my || e.dist > m_R || m_pend.excl.count (kv.first)) continue;
          if (kv.first < N && !e.ok) continue;
          double r = RangeFor (std::max (1.1 * e.dist + 10.0, 1.0));   // power control with a distance margin
          if (kv.first >= N) { if (bj < 0 || bj < (int) N || r < bestR) { bj = kv.first; bestR = r; best = 1e9; } continue; }
          double eta = (my - e.depth) / (m_txPowerW * m_tPkt * Gain (r));
          etamax = std::max (etamax, eta);
          c.push_back (std::make_tuple (kv.first, r, eta, e.energy / m_e0, e.succ));
        }
      if (bj >= (int) N) { h.relays.push_back (bj); range = bestR; return true; }
      for (auto &x : c)
        {
          double f = std::get<4> (x) * (0.5 * std::get<2> (x) / (etamax > 0 ? etamax : 1) + 0.5 * std::get<3> (x));
          if (f > best) { best = f; bj = std::get<0> (x); bestR = std::get<1> (x); }
        }
      if (bj < 0) return false;
      h.relays.push_back (bj); range = bestR;
      return true;
    }

  auto cands = View ();
  std::vector<std::tuple<uint16_t, double, double, double, bool, double>> up;
  for (auto &x : cands)
    {
      uint16_t j = std::get<0> (x);
      if (std::get<1> (x) < my && std::get<2> (x) <= m_R && (j >= N || std::get<4> (x) || s_noRouteBit)
          && !m_pend.excl.count (j))
        up.push_back (x);
    }
  if (up.empty ())
    {
      if (m_mode == 8 && HcPathValid ())
        {   // local minimum: switch to the recovery path
          h.relays.push_back (m_hcPath[0]);
          for (size_t k = 1; k < m_hcPath.size (); k++) h.nbrs.push_back ({m_hcPath[k], 0.0f});
          h.ival = -1; m_hcHoldMax = 0;
          return true;
        }
      if (m_mode == 10) m_qV = -3.0 / (1 - m_qGamma);   // no shallower neighbour: lowest possible value
      return false;
    }
  if (m_mode == 10)
    {   // QLFR Eq. (5)-(8): update Q(s_i, s_j) for every shallower neighbour, V(s_i) = max Q
      double dmax = 1e-9, now = Simulator::Now ().GetSeconds ();
      for (auto &kv : m_nit) if (Fresh (kv.second, now)) dmax = std::max (dmax, std::fabs (my - kv.second.depth));
      double ci = 1 - ResidualEnergy () / m_e0, best = -1e9;
      for (auto &x : up)
        {
          uint16_t j = std::get<0> (x);
          double cj = j >= N ? 0.0 : 1 - std::get<3> (x) / m_e0;
          double cd = 0.5 * (1 - (my - std::get<1> (x)) / dmax);
          double vj = j >= N ? 0.0 : m_nit[j].v;
          double &q = m_q[j];
          q = m_qAlpha * (-ci - cj - cd + m_qGamma * vj) + (1 - m_qAlpha) * q;
          best = std::max (best, q);
        }
      m_qV = best;
    }
  std::vector<std::tuple<uint16_t, double, double, double, bool, double>> sinks;
  for (auto &x : up) if (std::get<0> (x) >= N) sinks.push_back (x);
  if (!sinks.empty () && !s_noSinkShortcut)
    {
      std::sort (sinks.begin (), sinks.end (), [] (auto &a, auto &b) { return std::get<2> (a) < std::get<2> (b); });
      h.relays.push_back (std::get<0> (sinks[0]));
      return true;
    }

  if (m_mode == 8) return HcChoose (h, up);
  if (m_mode == 11)
    {   // HFS-PBR: the best forwarding node by Eq. (1) goes in the header
      int b = HfBfn ();
      if (b < 0) return false;
      h.relays.push_back ((uint16_t) b);
      return true;
    }
  if (m_mode == 10)
    {   // priority list: shallower neighbours by Q-value, length set by the multipath suppression scheme
      std::stable_sort (up.begin (), up.end (), [&] (auto &a, auto &b) { return m_q[std::get<0> (a)] > m_q[std::get<0> (b)]; });
      for (auto &x : up) { if ((int) h.relays.size () >= std::max (m_qL, 1)) break; h.relays.push_back (std::get<0> (x)); }
      return true;
    }

  if (m_mode == 6)   // EEDBR: all shallower neighbours, ordered by residual energy (highest first)
    {
      std::stable_sort (up.begin (), up.end (), [] (auto &a, auto &b) { return std::get<3> (a) > std::get<3> (b); });
      for (auto &x : up) { if (h.relays.size () >= 255) break; h.relays.push_back (std::get<0> (x)); }
      return true;
    }

  if (m_mode >= 1 && m_mode <= 3)   // RE-PBR / SPRE-PBR / SPRE-PBR-near
    {
      double reMax = 0, ddMax = 0;
      for (auto &x : up) { reMax = std::max (reMax, std::get<3> (x)); ddMax = std::max (ddMax, std::get<2> (x)); }
      std::vector<double> cost;
      for (auto &x : up)
        {
          double re = 1 - std::get<3> (x) / (reMax > 0 ? reMax : 1);
          double dd = std::get<2> (x) / (ddMax > 0 ? ddMax : 1);
          cost.push_back (re + ((m_mode == 3 || m_near) ? dd : 1 - dd));
        }
      std::vector<size_t> order (up.size ());
      for (size_t k = 0; k < order.size (); k++) order[k] = k;
      std::sort (order.begin (), order.end (), [&] (size_t a, size_t b) { return cost[a] < cost[b]; });
      if (m_mode == 1) { h.relays.push_back (std::get<0> (up[order[0]])); return true; }
      size_t OC = std::min<size_t> (up.size (), 4);
      std::vector<size_t> oc;
      double layers[3][2] = {{2 * m_R / 3, m_R}, {m_R / 3, 2 * m_R / 3}, {0, m_R / 3}};
      for (auto &L : layers)
        for (size_t k : order)
          {
            if (oc.size () >= OC) break;
            double diff = my - std::get<1> (up[k]);
            if (diff > L[0] && diff <= L[1] && std::find (oc.begin (), oc.end (), k) == oc.end ()) oc.push_back (k);
          }
      if (oc.empty ()) oc.assign (order.begin (), order.begin () + OC);
      size_t best = *std::min_element (oc.begin (), oc.end (), [&] (size_t a, size_t b) { return cost[a] < cost[b]; });
      h.relays.push_back (std::get<0> (up[best]));
      return true;
    }

  // WDFAD-DBR (receiver-based, the receivers decide): the sender only needs one usable shallower neighbour
  for (auto &x : up)
    if (Psr (std::get<2> (x), m_R) >= 0.2) { h.relays.push_back (std::get<0> (x)); return true; }
  return false;
}

// ------------------------------------------------------------------ sender side
void AquaSimPbrx::TrySend ()
{
  if (m_isSink || Dead () || m_pend.active || m_queue.empty ()) return;
  PbrxHeader h = m_queue.front ();
  m_queue.pop_front ();
  m_pend = Pending ();
  m_pend.h = h; m_pend.active = true;
  SendCurrent ();
}

void AquaSimPbrx::SendCurrent ()
{
  PbrxHeader &h = m_pend.h;
  double range;
  if (m_mode == 7 && m_pend.attempt == 0 && WdfadNextDepth () < 0)
    {   // no neighbour known: NEIGHBOR REQUEST, then send after the reply window
      m_pend.attempt = 1;
      WdfadNreq ();
      Simulator::Schedule (Seconds (2 * m_R / 1500.0 + 1.0 + 3 * m_tPkt + 0.4), &AquaSimPbrx::SendCurrent, this);
      return;
    }
  if (m_pend.attempt == 0 || m_mode == 4)
    {
      if (!ChooseRelays (h, range))
        {
          if (m_mode == 4 && m_pend.rec <= 2) { m_pend.rec++; Probe (); return; }
          if (m_mode == 8 && m_pend.rec == 0)
            {   // local minimum without a recovery path: discover one, then retry once
              m_pend.rec = 1;
              double now = Simulator::Now ().GetSeconds (), wait = -1;
              if (now - m_hcLastDisc < 10.0) wait = m_hcLastDisc + 10.0 - now;          // discovery in progress
              else if (now - m_hcLastDisc >= HcBackoff ()) { HcDiscover (); wait = 10.0; }
              if (wait >= 0) { Simulator::Schedule (Seconds (wait), &AquaSimPbrx::SendCurrent, this); return; }
            }
          m_pend.active = false; s_dropVoid++; TRK (h, "dropVoid"); DropCustody (h);
          if (getenv ("PBRX_DBG"))
            {
              int n = 0, sh = 0, shok = 0, shin = 0; double now = Simulator::Now ().GetSeconds ();
              for (auto &e : m_nit) { n++; if (e.second.depth < Depth ()) { sh++; if (e.second.ok) shok++; if (e.second.dist <= m_R) shin++; } }
              fprintf (stderr, "VOID t=%.0f node=%u z=%.0f nit=%d shallower=%d ok=%d inR=%d attempt=%d\n", now, m_index, Depth (), n, sh, shok, shin, m_pend.attempt);
            }
          Simulator::Schedule (Seconds (0), &AquaSimPbrx::TrySend, this);
          return;
        }
      h.txRange = range;
    }
  range = h.txRange;
  h.type = PbrxHeader::DATA;
  if (m_mode == 4) m_totCount++;
  if (s_trk) { std::string r = "tx[a" + std::to_string (m_pend.attempt) + "]->"; for (auto q : h.relays) r += std::to_string (q) + ","; TRK (h, r); }
  // EEDBR carries the forwarding list in the data packet: 2 bytes per extra ID
  uint32_t extra = ((m_mode == 6 || m_mode == 10) && h.relays.size () > 1) ? 2 * (h.relays.size () - 1) : 0;
  if (m_mode == 8) extra = (h.ival < 0) ? 2 * (1 + h.nbrs.size ()) : 19;   // source route, or 150-bit Bloom filter of the set
  Transmit (h, m_dataBytes + extra, range);
  if (NoAck ())
    {   // DBR / EEDBR: fire and forget
      m_pend.active = false;
      Simulator::Schedule (Seconds (m_tPkt + 0.01), &AquaSimPbrx::TrySend, this);
      return;
    }
  uint32_t tok = ++m_token;
  // relay hands the packet up after 2 airtimes, may back off up to 0.4 s, and its forward takes 1 airtime + propagation
  double to = std::max (m_retime, 3 * m_tPkt + 2 * range / 1500.0 + 0.4 + (h.relays.size () - 1) * m_slot + 0.2);
  if (m_mode == 8) to = std::max (m_retime, 3 * m_tPkt + 2 * range / 1500.0 + 0.4 + m_hcHoldMax + 3 * 8.0 * m_ackBytes / m_rate + 0.2);
  m_ackTimer = Simulator::Schedule (Seconds (to), &AquaSimPbrx::AckTimeout, this, tok);
}

void AquaSimPbrx::AckTimeout (uint32_t token)
{
  if (token != m_token || !m_pend.active) return;
  if (m_mode == 4)
    {   // APCDBRP: no retransmission, localized recovery
      if (m_pend.rec < 2 && !Dead ()) { m_pend.rec++; Probe (); return; }
    }
  else if (m_pend.attempt < (m_mode == 8 ? m_hcRetx : m_retx))
    {
      m_pend.attempt++; s_retx++;
      SendCurrent ();
      return;
    }
  if (m_mode == 8 && m_pend.h.ival < 0 && m_pend.h.src != 0xFFFF) m_hcPath.clear ();   // recovery hop failed: rediscover
  m_pend.active = false; s_dropRetx++; TRK (m_pend.h, "dropRetx"); DropCustody (m_pend.h);
  TrySend ();
}

void AquaSimPbrx::Confirmed ()
{
  if (!m_pend.active) return;
  TRK (m_pend.h, std::string ("confirmed:") + s_why);
  m_ackTimer.Cancel ();
  ++m_token;
  if (m_mode == 4) m_okCount++;
  m_pend.active = false;
  Simulator::Schedule (Seconds (0), &AquaSimPbrx::TrySend, this);
}

// APCDBRP localized recovery: PROBE at max power, shallower neighbours answer at max power
uint16_t AquaSimPbrx::Probe ()
{
  s_probe++;
  PbrxHeader h; h.type = PbrxHeader::PROBE;
  Transmit (h, m_ctrlBytes, m_R);
  for (uint16_t r : m_pend.h.relays) m_pend.excl.insert (r);
  // after the reply window, retry with the refreshed table (replies are observed in Recv)
  uint32_t tok = ++m_token;
  Simulator::Schedule (Seconds (2 * m_R / 1500.0 + 3 * m_tPkt + 0.4 + 0.5), [this, tok] () {
      if (tok != m_token || !m_pend.active) return;
      PbrxHeader &h = m_pend.h;
      double range;
      if (!ChooseRelays (h, range))
        {   // no reply: HOLE notification, drop the packet
          m_hole = true;
          PbrxHeader hh; hh.type = PbrxHeader::HOLE;
          Transmit (hh, m_ctrlBytes, m_R);
          m_pend.active = false; s_dropVoid++; DropCustody (m_pend.h);
          TrySend ();
          return;
        }
      h.txRange = range; h.type = PbrxHeader::DATA;
      m_totCount++;
      Transmit (h, m_dataBytes, range);
      uint32_t t2 = ++m_token;
      m_ackTimer = Simulator::Schedule (Seconds (std::max (m_retime, 3 * m_tPkt + 2 * range / 1500.0 + 0.4 + 0.2)),
                                        &AquaSimPbrx::AckTimeout, this, t2);
    });
  return 0;
}

void AquaSimPbrx::Flood (bool fromSink)
{
  PbrxHeader h; h.type = PbrxHeader::BEACON;
  h.seq = ++s_floodId;   // unique flood id
  if (fromSink) m_seen.insert ({0xFFFF, h.seq});
  Transmit (h, m_isSink ? 16 : m_ctrlBytes, m_R);
}

// ------------------------------------------------------------------ receiver side
void AquaSimPbrx::HoldExpire (PbrxHeader h)
{
  auto key = std::make_pair (h.src, h.seq);
  auto it = m_holds.find (key);
  if (it == m_holds.end ()) return;
  m_holds.erase (it);
  if (Dead () || m_custody.count (key) || (NoAck () && m_seen.count (key))) return;
  TRK (h, "holdFwd");
  m_seen.insert (key); m_custody.insert (key); s_holdFwd++;
  m_holdPkt.erase (key);
  if (m_mode == 8)
    {   // HydroCast: the winner acknowledges at once; the ACK also suppresses the other members of the set
      PbrxHeader a; a.type = PbrxHeader::ACK; a.src = h.src; a.seq = h.seq; a.relays = {h.sender};
      Transmit (a, m_ackBytes, m_R);
    }
  else if (!NoAck ()) QueueAck (h);
  m_queue.push_back (h);
  TrySend ();
}

void AquaSimPbrx::Deliver (const PbrxHeader &h)
{
  auto key = std::make_pair (h.src, h.seq);
  if (s_delivered.count (key)) { s_dup++; return; }
  s_delivered.insert (key);
  s_deliv++; s_winDel++;
  s_delaySum += Simulator::Now ().GetSeconds () - h.gen;
}

bool AquaSimPbrx::Recv (Ptr<Packet> p, const Address &dest, uint16_t protocolNumber)
{
  AquaSimHeader ash;
  p->RemoveHeader (ash);
  if (p->GetSize () < 10) return false;
  PbrxHeader h;
  p->RemoveHeader (h);
  if (Dead () || h.sender == m_index) return false;
  AquaSimPbrx *snd = (h.sender < s_nodes.size ()) ? s_nodes[h.sender] : nullptr;
  if (!snd) return false;
  double d = CalculateDistance (Pos (), snd->Pos ());
  // acoustic link errors on top of Aqua-Sim's collision model
  if (m_uni->GetValue () >= Psr (d, h.txRange, 8.0 * ash.GetSize ())) { s_rxLinkLoss++; return false; }
  s_rxOk++;
  double dEst = d + m_norm->GetValue () * m_sigmaD;
  if (m_mode == 4) dEst = d;          // APCDBRP: exact RSS-based path loss (favourable)
  auto key = std::make_pair (h.src, h.seq);
  uint16_t N = s_numSensors;

  switch (h.type)
    {
    case PbrxHeader::HELLO:
      if (!m_isSink && m_mode != 0) Observe (h, dEst);
      return true;
    case PbrxHeader::SINKACK:
      if (m_pend.active && m_pend.h.src == h.src && m_pend.h.seq == h.seq) { s_why = "sinkack"; Confirmed (); }
      if (m_holds.count (key)) { m_holds[key].Cancel (); m_holds.erase (key); m_holdPkt.erase (key); m_seen.insert (key); }
      if (m_mode == 4) Observe (h, dEst);
      return true;
    case PbrxHeader::ACK:
      if (m_mode == 4) Observe (h, dEst);
      if (!h.relays.empty () && h.relays[0] == m_index && m_pend.active && m_pend.h.src == h.src && m_pend.h.seq == h.seq)
        { s_why = h.ok ? "ack" : "ack-noroute"; Confirmed (); }
      if (m_holds.count (key) && !h.relays.empty () && h.relays[0] == m_holdFrom[key] && h.sender != m_holdFrom[key])
        { m_holds[key].Cancel (); m_holds.erase (key); m_holdPkt.erase (key); m_seen.insert (key); TRK (h, "suppressedByAck"); }
      return true;
    case PbrxHeader::PROBE:
      if (m_mode == 7)
        {   // NEIGHBOR REQUEST: learn the sender if it is shallower; answer (random wait) if I am its PFN
          if (!m_isSink) WdfadLearn (h, dEst);
          if (m_isSink || Depth () < h.depth)
            Simulator::Schedule (Seconds (m_uni->GetValue (0, 1.0)), [this] () {
                if (Dead ()) return; PbrxHeader r; r.type = PbrxHeader::PREPLY; Transmit (r, m_isSink ? 16 : m_ackBytes, m_R); });
          return true;
        }
      if (m_mode == 4 && !m_isSink)
        {
          Observe (h, dEst);
          if (Depth () < h.depth && !m_hole)
            Simulator::Schedule (Seconds (m_uni->GetValue (0, 0.3)), [this] () {
                PbrxHeader r; r.type = PbrxHeader::PREPLY; Transmit (r, m_ctrlBytes, m_R); });
        }
      else if (m_mode == 4 && m_isSink)
        Simulator::Schedule (Seconds (m_uni->GetValue (0, 0.3)), [this] () {
            PbrxHeader r; r.type = PbrxHeader::PREPLY; Transmit (r, 16, m_R); });
      return true;
    case PbrxHeader::PREPLY:
      if (m_mode == 7 && !m_isSink) WdfadLearn (h, dEst);
      if (m_mode == 4 && !m_isSink) Observe (h, dEst);
      return true;
    case PbrxHeader::HOLE:
      if (!m_isSink) m_nit.erase (h.sender);
      return true;
    case PbrxHeader::BEACON:
      if (m_mode == 9 && !m_isSink)
        {   // DVOR query (Algorithm 1): seq = query id, ival = hop count of the sender
          uint32_t q = h.seq; double n = h.ival, now = Simulator::Now ().GetSeconds ();
          if (q > m_dvQid)
            {
              m_dvQid = q; m_dvN = n + 1; m_dvT = now;
              Simulator::Schedule (Seconds (m_uni->GetValue (0, 2.0)), [this, q] () {
                  if (Dead () || q != m_dvQid) return;
                  PbrxHeader x; x.type = PbrxHeader::BEACON; x.seq = q; x.ival = m_dvN; Transmit (x, m_ctrlBytes, m_R); });
            }
          else if (q == m_dvQid && m_dvN > n + 1) m_dvN = n + 1;
          return true;
        }
      if (m_mode == 10 && !m_isSink)
        {   // QLFR: new priority-list length flooded by the sinks (carried in the gen field)
          auto fk = std::make_pair ((uint16_t) 0xFFFF, h.seq);
          if (!m_seen.count (fk))
            {
              m_seen.insert (fk); m_qL = (int) h.gen;
              PbrxHeader b = h;
              Simulator::Schedule (Seconds (m_uni->GetValue (0, 2.0)), [this, b] () {
                  if (Dead ()) return; PbrxHeader x = b; x.type = PbrxHeader::BEACON; Transmit (x, m_ctrlBytes, m_R); });
            }
          return true;
        }
      if (m_mode == 6 && !m_isSink)
        {   // delivery ratio flooded by the sinks (carried in the gen field)
          auto fk = std::make_pair ((uint16_t) 0xFFFF, h.seq);
          if (!m_seen.count (fk))
            {
              m_seen.insert (fk);
              m_eedbrDr = h.gen;
              PbrxHeader b = h;
              Simulator::Schedule (Seconds (m_uni->GetValue (0, 2.0)), [this, b] () {
                  if (Dead ()) return; PbrxHeader x = b; x.type = PbrxHeader::BEACON; Transmit (x, m_ctrlBytes, m_R); });
            }
        }
      if (m_mode == 4 && !m_isSink)
        {
          if (h.depth < Depth ())
            {
              m_hole = false;
              Observe (h, dEst);
              auto fk = std::make_pair ((uint16_t) 0xFFFF, h.seq);
              if (!m_seen.count (fk))
                {
                  m_seen.insert (fk);
                  PbrxHeader b = h;
                  Simulator::Schedule (Seconds (m_uni->GetValue (0, 2.0)), [this, b] () {
                      if (Dead ()) return; PbrxHeader x = b; x.type = PbrxHeader::BEACON; Transmit (x, m_ctrlBytes, m_R); });
                }
            }
        }
      return true;
    case PbrxHeader::RDISC:
      if (m_mode == 8 && !m_isSink) HcOnDisc (h);
      return true;
    case PbrxHeader::RREP:
      if (m_mode == 8 && !m_isSink) HcOnRrep (h);
      return true;
    default:
      break;
    }

  // ---------------- DATA
  m_lastData = Simulator::Now ().GetSeconds ();
  if (m_mode == 4 && !m_isSink) Observe (h, dEst);   // implicit routing update
  if (m_mode == 7 && !m_isSink) WdfadLearn (h, dEst);
  if (m_mode == 10 && !m_isSink) Observe (h, dEst);   // routing knowledge piggybacked on data
  if (m_isSink)
    {
      bool forMe = std::find (h.relays.begin (), h.relays.end (), m_index) != h.relays.end ();
      if (NoAck () || forMe)
        {
          Deliver (h);
          if (!NoAck ())
            {   // DBR has no acknowledgements
              PbrxHeader a; a.type = PbrxHeader::SINKACK; a.src = h.src; a.seq = h.seq;
              a.relays = {h.sender};
              Transmit (a, m_ackBytes, m_R);
            }
        }
      return true;
    }
  // overhearing: implicit ACK for my pending packet (relay forwarded it)
  if (m_pend.active && m_pend.h.src == h.src && m_pend.h.seq == h.seq &&
      std::find (m_pend.h.relays.begin (), m_pend.h.relays.end (), h.sender) != m_pend.h.relays.end ())
    { s_why = "overheard"; Confirmed (); }
  // overhearing a forward cancels my holding (suppression)
  // (a retransmission by the previous hop itself is not a forward and must not suppress)
  if (m_holds.count (key) && m_holdFrom[key] != h.sender)
    {
      bool progress = true;
      if (m_mode == 6)
        {   // EEDBR: suppress with probability = delivery ratio notified by the sink (1 when the target is met)
          if (m_eedbrKeep.count (key)) progress = false;
          else if (m_uni->GetValue () >= m_eedbrDr) { progress = false; m_eedbrKeep.insert (key); }
        }
      else if (m_mode == 9)
        progress = h.ival <= DvN ();   // DVOR: only a copy from a node with hop count <= mine cancels
      else if (m_mode == 10 || m_mode == 11)
        progress = true;               // QLFR / HFS-PBR AFN: any overheard forward of the packet cancels
      else if (m_mode != 0 && m_mode != 7)
        {   // holders in a forwarding set (HydroCast cluster members): only a forward by another member of the
            // held packet's set, or by a node shallower than the previous hop, shows that the packet moved on
          const PbrxHeader &hp = m_holdPkt[key];
          bool peer = std::find (hp.relays.begin (), hp.relays.end (), h.sender) != hp.relays.end ();
          progress = peer || h.depth < hp.depth;
        }
      if (progress)
        {
          m_holds[key].Cancel (); m_holds.erase (key); m_holdPkt.erase (key);
          bool chosen = std::find (h.relays.begin (), h.relays.end (), m_index) != h.relays.end ();
          if (!chosen || NoAck ()) { m_seen.insert (key); TRK (h, "suppressed"); }   // else: handled below as a fresh relay
        }
    }

  if (m_mode == 0)
    {   // DBR: shallower receivers contend
      double diff = h.depth - Depth ();
      if (diff <= 0 || m_seen.count (key) || m_holds.count (key)) return true;
      double hold = 2 * (m_R / 1500.0) * std::max (m_R - diff, 0.0) / m_R;
      PbrxHeader f = h;
      m_holds[key] = Simulator::Schedule (Seconds (hold), &AquaSimPbrx::HoldExpire, this, f);
      m_holdFrom[key] = h.sender;
      m_holdPkt[key] = h;
      return true;
    }

  if (m_mode == 11)
    {   // HFS-PBR: BFN forwards at once, AFN holds (Eq. 3), SAN forwards with probability 1/(n+1) (Eq. 4)
      if (m_seen.count (key) || m_holds.count (key)) return true;
      double dd1 = h.depth - Depth ();
      if (dd1 <= 0) return true;
      double now = Simulator::Now ().GetSeconds ();
      uint16_t bfn = h.relays.empty () ? 0xFFFF : h.relays[0];
      if (bfn == m_index)
        {
          m_seen.insert (key); m_custody.insert (key);
          m_queue.push_back (h); TrySend ();
          return true;
        }
      auto ib = m_nit.find (bfn);
      if (ib != m_nit.end () && Fresh (ib->second, now))
        {   // AFN
          double bd = 0; int b2 = HfBfn (&bd);
          double dd2 = b2 >= 0 ? std::max (Depth () - bd, 0.0) : 0.0;
          double w = m_hfAlpha * dd1 + (1 - m_hfAlpha) * dd2;
          double tmax = m_R / 1500.0 + m_tPkt;
          double T = ((1 - std::min (w / m_R, 1.0)) + 2 * HfAngle () / M_PI + (1 - std::max (ResidualEnergy (), 0.0) / m_e0)) * 2 * tmax / 3;
          PbrxHeader f = h;
          m_holds[key] = Simulator::Schedule (Seconds (T), &AquaSimPbrx::HoldExpire, this, f);
          m_holdFrom[key] = h.sender;
          m_holdPkt[key] = h;
          return true;
        }
      // SAN
      m_seen.insert (key);
      auto is = m_nit.find (h.sender);
      if (is == m_nit.end () || !Fresh (is->second, now)) return true;   // sender unknown: drop
      if (HfAngle () >= h.vval) return true;                             // pointing away from the sink: drop
      int cnt = 0;
      for (auto &kv : m_nit) if (Fresh (kv.second, now)) cnt++;
      if (m_uni->GetValue () < 1.0 / (cnt + 1))
        { m_custody.insert (key); m_queue.push_back (h); TrySend (); }
      return true;
    }
  if (m_mode == 9)
    {   // DVOR (Algorithm 2): every receiver with a hop count waits by Eq. (1)
      if (m_seen.count (key) || m_holds.count (key)) return true;
      double n = DvN ();
      if (n >= 1e8) return true;
      if (n >= h.ival) return true;   // only nodes closer to the sink (in hops) forward (paper Figs. 2-3: no forwarding
                                      // into a higher hop count, no detour via an equal hop count)
      double k = std::max (n - h.ival + 2, 0.0), t0 = m_R / 1500.0;
      double hold = 2 * k * t0 + std::floor (m_uni->GetValue (0, m_dvCW + 1)) * m_dvSlot * k;
      PbrxHeader f = h;
      m_holds[key] = Simulator::Schedule (Seconds (hold), &AquaSimPbrx::HoldExpire, this, f);
      m_holdFrom[key] = h.sender;
      m_holdPkt[key] = h;
      return true;
    }
  if (m_mode == 7)
    {   // WDFAD-DBR (Algorithm 1)
      double diff = h.depth - Depth ();
      if (diff <= 0 || m_seen.count (key) || m_holds.count (key)) return true;   // not a PFN / already handled
      if (!WdfadInForwardArea (diff, dEst)) return true;                          // suppressed area: wait for the next copy
      double nd = WdfadNextDepth ();
      if (nd < 0)
        {   // no PFN in the next hop: drop (void avoided in advance) and look for neighbours
          m_seen.insert (key); s_dropVoid++;
          if (Simulator::Now ().GetSeconds () - m_lastNreq > 50) WdfadNreq ();
          return true;
        }
      double H = m_wAlpha * diff + (1 - m_wAlpha) * std::max (Depth () - nd, 0.0);
      double hold = std::max (0.0, 2 * m_R / 1500.0 / m_wDelta * (m_R - std::min (H, m_R)));   // Eq. (9)
      PbrxHeader f = h;
      m_holds[key] = Simulator::Schedule (Seconds (hold), &AquaSimPbrx::HoldExpire, this, f);
      m_holdFrom[key] = h.sender;
      m_holdPkt[key] = h;
      return true;
    }

  auto it = std::find (h.relays.begin (), h.relays.end (), m_index);
  if (it == h.relays.end ()) return true;
  int rank = it - h.relays.begin ();
  if (m_mode == 10)
    {   // QLFR: holding time tau(n) = 2 t_max / h * (n - 1) (Eq. 15)
      if (Depth () >= h.depth || m_seen.count (key) || m_holds.count (key)) return true;
      double hold = 2 * (m_R / 1500.0) / m_qH * rank;
      PbrxHeader f = h;
      m_holds[key] = Simulator::Schedule (Seconds (hold), &AquaSimPbrx::HoldExpire, this, f);
      m_holdFrom[key] = h.sender;
      m_holdPkt[key] = h;
      return true;
    }
  if (m_mode == 8 && h.ival < 0)
    {   // recovery hop (may go deeper): unicast along the source route, acknowledged at once
      if (m_custody.count (key))
        { PbrxHeader a; a.type = PbrxHeader::ACK; a.src = h.src; a.seq = h.seq; a.relays = {h.sender}; Transmit (a, m_ackBytes, m_R); return true; }
      m_seen.insert (key); m_custody.insert (key);
      PbrxHeader a; a.type = PbrxHeader::ACK; a.src = h.src; a.seq = h.seq; a.relays = {h.sender};
      Transmit (a, m_ackBytes, m_R);
      m_queue.push_back (h);
      TrySend ();
      return true;
    }
  if (m_mode == 6)
    {   // EEDBR: first node in the list forwards at once; the others hold
        // T = (1 - E/E0) * max_holding_time + p, p = starting value doubled per list position
      if (Depth () >= h.depth || m_seen.count (key) || m_holds.count (key)) return true;
      double hold = rank == 0 ? 0.0
                              : (1 - ResidualEnergy () / m_e0) * m_eedbrMaxHold + m_eedbrP0 * std::pow (2.0, std::min (rank, 12));
      PbrxHeader f = h;
      m_holds[key] = Simulator::Schedule (Seconds (hold), &AquaSimPbrx::HoldExpire, this, f);
      m_holdFrom[key] = h.sender;
      m_holdPkt[key] = h;
      return true;
    }
  if (Depth () >= h.depth) return true;   // never accept a packet from a shallower node (prevents two-node loops)
  if (m_custody.count (key))
    {   // duplicate of a packet I hold or forwarded (my forward was not heard): short ACK
      s_dupAck++; TRK (h, "dupAck->" + std::to_string (h.sender));
      PbrxHeader a; a.type = PbrxHeader::ACK; a.src = h.src; a.seq = h.seq; a.relays = {h.sender};
      Transmit (a, m_ackBytes, m_mode == 4 ? RangeFor (dEst) : m_R);
      return true;
    }
  if (m_mode == 8)
    {   // HydroCast: every member of the set waits alpha * (R - progress); the first to expire ACKs and forwards
      if (m_seen.count (key)) return true;                                     // already suppressed
      if (m_holds.count (key) && m_holdFrom[key] == h.sender) return true;     // retransmission: keep the running timer
      double hold = std::max (h.ival, 0.0) * std::max (m_R - (h.depth - Depth ()), 0.0);
      PbrxHeader f = h;
      if (m_holds.count (key)) m_holds[key].Cancel ();
      m_holds[key] = Simulator::Schedule (Seconds (hold), &AquaSimPbrx::HoldExpire, this, f);
      m_holdFrom[key] = h.sender;
      m_holdPkt[key] = h;
      return true;
    }
  if (m_mode == 4)
    {   // APCDBRP: ACK only if my next power level is lower than the previous hop's
      PbrxHeader nx = h; double rnext = m_R;
      Pending save = m_pend; m_pend.excl.clear ();
      bool okn = ChooseRelays (nx, rnext);
      m_pend = save;
      if (!okn || rnext < h.txRange)
        {
          PbrxHeader a; a.type = PbrxHeader::ACK; a.src = h.src; a.seq = h.seq; a.relays = {h.sender};
          Transmit (a, m_ackBytes, RangeFor (dEst));
        }
    }
  if (m_holds.count (key)) { m_holds[key].Cancel (); m_holds.erase (key); m_holdPkt.erase (key); }
  m_seen.insert (key); m_custody.insert (key);
  TRK (h, "accepted");
  QueueAck (h);
  m_queue.push_back (h);
  TrySend ();
  return true;
}

// relay is busy (own packet in flight / queue): confirm custody at once so the previous hop does not retransmit
void AquaSimPbrx::QueueAck (const PbrxHeader &h)
{
  if (!(m_pend.active || !m_queue.empty ())) return;
  PbrxHeader a; a.type = PbrxHeader::ACK; a.src = h.src; a.seq = h.seq; a.relays = {h.sender};
  Transmit (a, m_ackBytes, m_R);
}

void AquaSimPbrx::Monitor () {}

// a node that dropped a packet no longer vouches for it: a retransmission is treated as new
void AquaSimPbrx::DropCustody (const PbrxHeader &h) { m_custody.erase (std::make_pair (h.src, h.seq)); }

uint32_t AquaSimPbrx::s_floodId = 0;

const char *AquaSimPbrx::s_why = "";
uint64_t AquaSimPbrx::s_dupAck = 0;
void AquaSimPbrx::Report ()
{
  if (s_trk)
    {
      std::map<std::string, int> hist;
      for (auto &kv : s_last) if (!s_delivered.count (kv.first)) hist[kv.second]++;
      for (auto &kv : hist) fprintf (stderr, "  lost-last %s : %d\n", kv.first.c_str (), kv.second);
      fprintf (stderr, "  dupAck=%lu\n", (unsigned long) s_dupAck);
      int shown = 0;
      for (auto &kv : s_last)
        if (!s_delivered.count (kv.first) && kv.second == "confirmed:ack" && shown++ < 12)
          fprintf (stderr, "LOST %u/%u:%s\n", kv.first.first, kv.first.second, s_hist[kv.first].c_str ());
    }
  uint64_t qd = 0, qa = 0, pd = 0, pa = 0, hd = 0;
  for (auto *n : s_nodes)
    {
      if (!n || n->m_isSink) continue;
      bool dead = n->Dead ();
      (dead ? qd : qa) += n->m_queue.size ();
      if (n->m_pend.active) (dead ? pd : pa)++;
      hd += n->m_holds.size ();
    }
  if (getenv ("PBRX_WTAB"))
    {
      int empty = 0, tot = 0, sum = 0;
      for (auto *n : s_nodes) { if (!n || n->m_isSink || n->Dead ()) continue; int c = 0; n->WdfadNextDepth (&c); tot++; sum += c; if (c == 0) empty++; }
      fprintf (stderr, "WTAB alive=%d empty=%d meanPFN=%.1f\n", tot, empty, tot ? (double) sum / tot : 0.0);
    }
  fprintf (stderr, "stranded: queueDead=%lu queueAlive=%lu pendDead=%lu pendAlive=%lu holds=%lu\n",
           (unsigned long) qd, (unsigned long) qa, (unsigned long) pd, (unsigned long) pa, (unsigned long) hd);
}

void AquaSimPbrx::EedbrNotify (double ratio)
{
  PbrxHeader h; h.type = PbrxHeader::BEACON;
  h.seq = ++s_floodId; h.gen = ratio;
  m_seen.insert ({0xFFFF, h.seq});
  Transmit (h, 16, m_R);
}

double AquaSimPbrx::s_eedbrTarget = 0;
double AquaSimPbrx::s_qlfrPth = 0.6;
bool AquaSimPbrx::s_noRouteBit = true;     // faithful default: the has-route bit is a framework add-on, off unless asked
bool AquaSimPbrx::s_noSinkShortcut = true; // faithful default: no nearest-sink shortcut unless asked

// HFS-PBR: angle between the vertical and the line to the nearest sink. Taken exact (favours HFS-PBR: the paper
// derives it from the RSSI of the sinks' network-wide "Sink Control" broadcasts, which are not simulated)
double AquaSimPbrx::HfAngle ()
{
  Vector me = Pos ();
  double best = -1, cz = 1;
  for (size_t k = s_numSensors; k < s_nodes.size (); k++)
    {
      if (!s_nodes[k]) continue;
      Vector q = s_nodes[k]->Pos ();
      double d = CalculateDistance (me, q);
      if (best < 0 || d < best) { best = d; cz = d > 1e-6 ? (me.z - q.z) / d : 1; }
    }
  return std::acos (std::max (-1.0, std::min (1.0, cz)));
}

// Eq. (1): Metric = dd/R + beta (1 - 2 theta / pi) + E/E0 over fresh, shallower, non-void neighbours
int AquaSimPbrx::HfBfn (double *depth)
{
  double now = Simulator::Now ().GetSeconds (), my = Depth (), bm = -1e9;
  int best = -1;
  for (auto &kv : m_nit)
    {
      const PbrxNbr &e = kv.second;
      if (!Fresh (e, now) || e.depth >= my || e.dist > m_R || m_pend.excl.count (kv.first)) continue;
      bool sink = kv.first >= s_numSensors;
      if (!sink && !e.ok) continue;
      double ang = sink ? 0.0 : e.v;
      double m = (my - e.depth) / m_R + m_hfBeta * (1 - 2 * ang / M_PI) + e.energy / m_e0;
      if (m > bm) { bm = m; best = kv.first; if (depth) *depth = e.depth; }
    }
  return best;
}

// DVOR: hop count is valid while queries keep arriving (expires after three missed query rounds)
double AquaSimPbrx::DvN ()
{
  if (Simulator::Now ().GetSeconds () - m_dvT > 3 * m_dvPeriod) return 1e9;
  return m_dvN;
}

void AquaSimPbrx::DvorQuery (uint32_t qid)
{
  PbrxHeader h; h.type = PbrxHeader::BEACON; h.seq = qid; h.ival = 0;
  Transmit (h, 16, m_R);
}

// ------------------------------------------------------------------ WDFAD-DBR helpers
void AquaSimPbrx::WdfadNreq ()
{
  if (Dead () || m_isSink) return;
  m_lastNreq = Simulator::Now ().GetSeconds ();
  PbrxHeader h; h.type = PbrxHeader::PROBE;
  Transmit (h, m_ackBytes, m_R);
}

// neighbour table keeps only PFNs (shallower neighbours): id, depth, distance (from RSSI), timestamp
void AquaSimPbrx::WdfadLearn (const PbrxHeader &h, double dEst)
{
  bool sink = h.sender >= s_numSensors;
  if (!sink && h.depth >= Depth ()) return;
  PbrxNbr &e = m_nit[h.sender];
  e.t = Simulator::Now ().GetSeconds (); e.depth = sink ? 0.0 : h.depth; e.dist = std::max (dEst, 1.0);
}

// Eq. (15): predicted distance in the worst case (both nodes moving apart at the node speed);
// entries predicted beyond R are deleted. Returns the smallest PFN depth, or -1 if the table is empty.
double AquaSimPbrx::WdfadNextDepth (int *count)
{
  double now = Simulator::Now ().GetSeconds (), my = Depth (), best = -1;
  int n = 0;
  for (auto it = m_nit.begin (); it != m_nit.end ();)
    {
      PbrxNbr &e = it->second;
      double h = my - e.depth;
      double ob = std::sqrt (std::max (e.dist * e.dist - h * h, 0.0)) + 2 * m_speed * (now - e.t);
      double dp = std::sqrt (ob * ob + h * h);
      if (dp > m_R) { it = m_nit.erase (it); continue; }
      if (e.depth < my) { n++; if (best < 0 || e.depth < best) best = e.depth; }
      ++it;
    }
  if (count) *count = n;
  return best;
}

// Section 4.6: primary area = Reuleaux triangle of width R with its apex at the sender (always forwards);
// the rest of the upper hemisphere is auxiliary, split into L sub-areas of equal width along the horizontal
// distance; sub-areas beyond level i are suppressed, i = smallest level with Den*(V_primary + sum V_Aj) >= n (Eq. 19),
// n from 1 - (1 - PSR)^n >= P (Eq. 20). Density = PFNs in my table / hemisphere volume.
bool AquaSimPbrx::WdfadInForwardArea (double h, double d)
{
  double R = m_R, rho = std::sqrt (std::max (d * d - h * h, 0.0));
  if (std::pow (rho + R / 2, 2) + std::pow (h - std::sqrt (3.0) * R / 2, 2) <= R * R) return true;   // primary
  int L = std::max (m_wLevels, 1);
  int sub = std::min (L, (int) std::floor (rho / (R / L)) + 1);
  // volumes (3D, revolved around the vertical axis through the sender), computed once
  static std::vector<double> vol; static double vPrim = 0, vR = -1; static int vL = -1;
  if (vR != R || vL != L)
    {
      vR = R; vL = L; vol.assign (L + 1, 0.0); vPrim = 0;
      int G = 400; double dr = R / G;
      for (int a = 0; a < G; a++)
        for (int b = 0; b < G; b++)
          {
            double r = (a + 0.5) * dr, y = (b + 0.5) * dr;
            if (r * r + y * y > R * R) continue;
            double dv = 2 * M_PI * r * dr * dr;
            if (std::pow (r + R / 2, 2) + std::pow (y - std::sqrt (3.0) * R / 2, 2) <= R * R) vPrim += dv;
            else vol[std::min (L, (int) std::floor (r / (R / L)) + 1)] += dv;
          }
    }
  int cnt = 0; WdfadNextDepth (&cnt);
  double den = cnt / (2.0 / 3.0 * M_PI * R * R * R);
  double p = Psr (0.75 * R, R, 8.0 * m_dataBytes);
  double n = (p >= 1) ? 1 : std::max (1.0, std::ceil (std::log (1 - m_wP) / std::log (std::max (1 - p, 1e-12))));
  double v = vPrim; int level = L;
  for (int i = 0; i <= L; i++)
    {
      if (i > 0) v += vol[i];
      if (den * v >= n) { level = i; break; }
    }
  return sub <= level;
}

// ------------------------------------------------------------------ HydroCast helpers
bool AquaSimPbrx::HcHasUp ()
{
  double now = Simulator::Now ().GetSeconds (), my = Depth ();
  for (auto &kv : m_nit) if (Fresh (kv.second, now) && kv.second.depth < my && kv.second.dist <= m_R) return true;
  return false;
}

bool AquaSimPbrx::HcPathValid ()
{
  if (m_hcPath.empty ()) return false;
  double now = Simulator::Now ().GetSeconds ();
  if (now - m_hcPathT > m_hcPathLife) { m_hcPath.clear (); return false; }
  auto it = m_nit.find (m_hcPath[0]);
  if (it == m_nit.end () || !Fresh (it->second, now)) { m_hcPath.clear (); return false; }
  return true;
}

// Section V-B: a node is on the void-floor surface if some random direction is not covered by a triangle of
// mutually connected neighbours (Monte Carlo, m vectors). Neighbour positions are taken exact (favours HydroCast:
// the paper obtains them with local Sweep localisation from measured distances).
bool AquaSimPbrx::HcSurface ()
{
  double now = Simulator::Now ().GetSeconds ();
  if (now - m_hcSurfT < 30) return m_hcSurf;
  m_hcSurfT = now;
  Vector me = Pos ();
  std::vector<Vector> P;
  for (auto &kv : m_nit)
    if (Fresh (kv.second, now) && kv.first < s_nodes.size () && s_nodes[kv.first])
      { Vector q = s_nodes[kv.first]->Pos (); P.push_back (Vector (q.x - me.x, q.y - me.y, q.z - me.z)); }
  if (P.size () < 3) return m_hcSurf = true;
  std::vector<Vector> V;
  for (int k = 0; k < m_hcVectors; k++)
    {
      double z = m_uni->GetValue (-1, 1), ph = m_uni->GetValue (0, 2 * M_PI), r = std::sqrt (1 - z * z);
      V.push_back (Vector (r * std::cos (ph), r * std::sin (ph), z));
    }
  std::vector<bool> dom (V.size (), false);
  size_t left = V.size ();
  auto sub = [] (const Vector &a, const Vector &b) { return Vector (a.x - b.x, a.y - b.y, a.z - b.z); };
  auto cross = [] (const Vector &a, const Vector &b) { return Vector (a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); };
  auto dot = [] (const Vector &a, const Vector &b) { return a.x * b.x + a.y * b.y + a.z * b.z; };
  auto len = [&] (const Vector &a) { return std::sqrt (dot (a, a)); };
  for (size_t a = 0; a < P.size (); a++)
    for (size_t b = a + 1; b < P.size (); b++)
      {
        if (len (sub (P[a], P[b])) > m_R) continue;
        for (size_t c = b + 1; c < P.size (); c++)
          {
            if (len (sub (P[a], P[c])) > m_R || len (sub (P[b], P[c])) > m_R) continue;
            Vector e1 = sub (P[b], P[a]), e2 = sub (P[c], P[a]);
            for (size_t k = 0; k < V.size (); k++)
              {
                if (dom[k]) continue;
                Vector pv = cross (V[k], e2); double det = dot (e1, pv);
                if (std::fabs (det) < 1e-9) continue;
                Vector tv = sub (Vector (0, 0, 0), P[a]);
                double u = dot (tv, pv) / det; if (u < 0 || u > 1) continue;
                Vector qv = cross (tv, e1);
                double v = dot (V[k], qv) / det; if (v < 0 || u + v > 1) continue;
                if (dot (e2, qv) / det > 0) { dom[k] = true; if (--left == 0) return m_hcSurf = false; }
              }
          }
      }
  return m_hcSurf = true;
}

void AquaSimPbrx::HcDiscover ()
{
  if (Dead () || m_isSink) return;
  m_hcLastDisc = Simulator::Now ().GetSeconds ();
  if (m_hcDiscSeq != 0) m_hcFail++;
  if (getenv ("PBRX_HC")) fprintf (stderr, "DISC t=%.0f node=%u z=%.0f fail=%d nit=%zu surf=%d\n", m_hcLastDisc, m_index, Depth (), m_hcFail, m_nit.size (), (int) HcSurface ());   // previous discovery got no reply: back off (60 s doubling, up to 960 s)
  PbrxHeader h; h.type = PbrxHeader::RDISC; h.src = m_index; h.seq = ++s_floodId; h.gen = Depth ();
  m_hcDiscSeq = h.seq;
  m_seen.insert ({0xFFFE, h.seq});
  Transmit (h, m_ctrlBytes, m_R);
}

// route discovery over the void floor: an escape node (shallower than the origin and able to go on) answers;
// otherwise only void-floor surface nodes rebroadcast (2D flooding), up to the hop limit
void AquaSimPbrx::HcOnDisc (const PbrxHeader &h)
{
  auto fk = std::make_pair ((uint16_t) 0xFFFE, h.seq);
  if (m_seen.count (fk) || h.src == m_index || Dead ()) return;
  m_seen.insert (fk);
  if (Depth () < h.gen && (HcHasUp () || HcPathValid ()))
    {
      PbrxHeader r; r.type = PbrxHeader::RREP; r.src = h.src; r.seq = h.seq; r.nbrs = h.nbrs;
      r.nbrs.push_back ({m_index, 0.0f});
      r.relays = {h.nbrs.empty () ? h.src : h.nbrs.back ().first};
      uint32_t bytes = m_ctrlBytes + 2 * r.nbrs.size ();
      Simulator::Schedule (Seconds (m_uni->GetValue (0, 0.3)), [this, r, bytes] () { if (!Dead ()) Transmit (r, bytes, m_R); });
      return;
    }
  if ((int) h.nbrs.size () >= m_hcHopLimit || !HcSurface ()) return;
  PbrxHeader f = h; f.nbrs.push_back ({m_index, 0.0f});
  uint32_t bytes = m_ctrlBytes + 2 * f.nbrs.size ();
  Simulator::Schedule (Seconds (m_uni->GetValue (0, 1.0)), [this, f, bytes] () { if (!Dead ()) Transmit (f, bytes, m_R); });
}

void AquaSimPbrx::HcOnRrep (const PbrxHeader &h)
{
  if (h.relays.empty () || h.relays[0] != m_index || Dead ()) return;
  if (h.src == m_index)
    {
      if (h.seq != m_hcDiscSeq) return;
      m_hcPath.clear ();
      for (auto &n : h.nbrs) m_hcPath.push_back (n.first);
      m_hcPathT = Simulator::Now ().GetSeconds (); m_hcDiscSeq = 0; m_hcFail = 0;
      return;
    }
  int k = -1;
  for (size_t q = 0; q < h.nbrs.size (); q++) if (h.nbrs[q].first == m_index) { k = q; break; }
  if (k < 0) return;
  PbrxHeader r = h; r.relays = {k == 0 ? h.src : h.nbrs[k - 1].first};
  uint32_t bytes = m_ctrlBytes + 2 * r.nbrs.size ();
  Simulator::Schedule (Seconds (m_uni->GetValue (0, 0.3)), [this, r, bytes] () { if (!Dead ()) Transmit (r, bytes, m_R); });
}

// Section IV-C: greedy clustering (MPR-like) over upward neighbours ranked by NADV = progress * p;
// seed + neighbours within beta*R, then expanded with every node within R of all members (2-hop distances
// from beacons); the minimum alpha of Eq. (7) must not exceed gamma (else drop the lowest-NADV member);
// the cluster with the highest EPA (Eq. 1) is the forwarding set, ordered by progress.
bool AquaSimPbrx::HcChoose (PbrxHeader &h, std::vector<std::tuple<uint16_t, double, double, double, bool, double>> &up)
{
  double my = Depth ();
  struct C { uint16_t id; double dp, d, p, nadv; };
  std::vector<C> c;
  for (auto &x : up)
    {
      double dp = my - std::get<1> (x), d = std::get<2> (x), p = Psr (d, m_R, 8.0 * (m_dataBytes + 19));
      c.push_back ({std::get<0> (x), dp, d, p, dp * p});
    }
  std::sort (c.begin (), c.end (), [] (const C &a, const C &b) { return a.nadv > b.nadv; });
  auto pd = [&] (uint16_t a, uint16_t b) -> double {
      auto ia = m_nit.find (a);
      if (ia != m_nit.end ()) { auto it = ia->second.nbrs.find (b); if (it != ia->second.nbrs.end ()) return it->second; }
      auto ib = m_nit.find (b);
      if (ib != m_nit.end ()) { auto it = ib->second.nbrs.find (a); if (it != ib->second.nbrs.end ()) return it->second; }
      return 1e9; };
  // receive-to-transmit of the ACK in this simulator: MAC back-off (<= 0.4 s) + ACK airtime and hand-up
  double tAck = 0.4 + 3 * 8.0 * m_ackBytes / m_rate;
  auto alphaOf = [&] (const std::vector<int> &s) {
      double a = 0;
      for (int i : s) for (int j : s)
        {
          if (c[i].dp <= c[j].dp) { if (i != j && c[i].dp == c[j].dp) return 1e9; continue; }
          double num = c[i].d / 1500.0 - c[j].d / 1500.0 + std::min (pd (c[i].id, c[j].id), m_R) / 1500.0 + tAck;
          a = std::max (a, num / (c[i].dp - c[j].dp));
        }
      return a; };
  auto epaOf = [&] (std::vector<int> s) {
      std::sort (s.begin (), s.end (), [&] (int a, int b) { return c[a].dp > c[b].dp; });
      double e = 0, miss = 1;
      for (int i : s) { e += c[i].dp * c[i].p * miss; miss *= 1 - c[i].p; }
      return e; };
  std::vector<std::vector<int>> cl;
  std::vector<bool> used (c.size (), false);
  for (size_t i = 0; i < c.size (); i++)
    {
      if (used[i]) continue;
      std::vector<int> s = {(int) i}; used[i] = true;
      for (size_t k = 0; k < c.size (); k++)
        if (!used[k] && pd (c[i].id, c[k].id) < m_hcBeta * m_R) { s.push_back (k); used[k] = true; }
      cl.push_back (s);
    }
  double bestE = -1, bestA = 0; std::vector<int> best;
  for (auto &s : cl)
    {
      for (size_t k = 0; k < c.size (); k++)
        {
          if (std::find (s.begin (), s.end (), (int) k) != s.end ()) continue;
          bool ok = true;
          for (int m : s) if (pd (c[k].id, c[m].id) >= m_R) { ok = false; break; }
          if (ok) s.push_back (k);
        }
      double a = alphaOf (s);
      while (s.size () > 1 && a > m_hcGamma)
        {   // drop the lowest-NADV member
          auto w = std::max_element (s.begin (), s.end (), [&] (int x, int y) { return c[x].nadv > c[y].nadv; });
          s.erase (w); a = alphaOf (s);
        }
      double e = epaOf (s);
      if (e > bestE) { bestE = e; best = s; bestA = a; }
    }
  if (best.empty ()) return false;
  std::sort (best.begin (), best.end (), [&] (int a, int b) { return c[a].dp > c[b].dp; });
  double minDp = 1e9;
  for (int i : best) { h.relays.push_back (c[i].id); minDp = std::min (minDp, c[i].dp); }
  h.ival = std::max (bestA, 1e-4);
  m_hcHoldMax = h.ival * std::max (m_R - minDp, 0.0);
  return true;
}

void AquaSimPbrx::Tick ()
{
  if (m_mode == 6 && !m_isSink && !Dead ())
    {   // EEDBR: hello when the residual energy changed by more than the threshold since the last hello
      if (m_lastHelloE - ResidualEnergy () >= m_eedbrETh * m_e0) { m_lastHelloE = ResidualEnergy (); ScheduleHello (m_uni->GetValue (0, 1.0)); }
      return;
    }
  if (m_mode == 8)
    {   // a local minimum keeps a recovery path
      // (after the bootstrap hello rounds, so that the neighbour tables are filled)
      if (!m_isSink && !Dead () && Simulator::Now ().GetSeconds () >= 90 && !HcHasUp () && !HcPathValid ()
          && Simulator::Now ().GetSeconds () - m_hcLastDisc >= HcBackoff ())
        HcDiscover ();
      return;
    }
  if (m_mode == 7)
    {   // periodic NEIGHBOR REQUEST with the expected period E(T) = 0.425 R / v (Eq. 16); none in a static network
      if (!m_isSink && !Dead () && m_speed > 0 && Simulator::Now ().GetSeconds () - m_lastNreq >= 0.425 * m_R / m_speed)
        WdfadNreq ();
      return;
    }
}

}  // namespace ns3
