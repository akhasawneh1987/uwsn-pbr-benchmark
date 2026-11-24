/*
 * PBR-family comparison in Aqua-Sim NG (ns-3.38).
 * Same scenario as the Python prototype: 1250^3 m, N sensors, S surface sinks, R = 250 m,
 * 10 kbps, 64 B data, P_tx 2 W, P_rx 0.75 W, E0 100 J, network-wide Poisson traffic,
 * static / random-walk / meandering-current mobility; run until 10% of sensors are dead.
 * Collisions, carrier sense and propagation delay come from Aqua-Sim (broadcast MAC, PhyCmn,
 * range propagation); acoustic link errors (Thorp + noise -> BPSK PSR) are applied per link.
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aqua-sim-ng-module.h"
#include "ns3/aqua-sim-routing-pbrx.h"
#include <random>
#include <cmath>

using namespace ns3;

static std::vector<Ptr<Node>> g_sensors, g_sinks;
static std::vector<AquaSimPbrx*> g_rt;
static std::mt19937_64 g_rng;
static double g_L = 1250, g_speed = 0, g_tau = 1.0, g_lambda = 1.0, g_T = 12000;
static std::string g_mob = "static";
static std::vector<Vector> g_vel;
static double g_fnd = -1, g_t10 = -1;
static uint32_t g_N = 400;

// Meandering current (Caruso et al. 2008), km units, scaled to vmax
struct Meander
{
  double A = 1.2, c = 0.12, k = 2 * M_PI / 7.5, om = 0.4, eps = 0.3, x0, y0, scale = 1;
  double psi (double x, double y, double t)
  {
    double B = A + eps * std::cos (om * t), ph = k * (x - c * t);
    return -std::tanh ((y - B * std::sin (ph)) / std::sqrt (1 + std::pow (k * B * std::cos (ph), 2)));
  }
  void raw (double xm, double ym, double ts, double &u, double &v)
  {
    double x = x0 + 4.0 * xm / 1000.0, y = y0 + 4.0 * ym / 1000.0 - 2.5, t = ts / 3600.0, h = 1e-4;
    u = -(psi (x, y + h, t) - psi (x, y - h, t)) / (2 * h);
    v = (psi (x + h, y, t) - psi (x - h, y, t)) / (2 * h);
  }
} g_mea;

static void Move ()
{
  double dt = 10.0;
  std::normal_distribution<double> nd (0, 1);
  std::uniform_real_distribution<double> ud (0, 1);
  for (size_t i = 0; i < g_sensors.size (); i++)
    {
      Ptr<MobilityModel> m = g_sensors[i]->GetObject<MobilityModel> ();
      Vector p = m->GetPosition ();
      if (g_mob == "walk")
        {
          double sp = ud (g_rng) * g_speed;
          double s = sp * std::sqrt (g_tau * dt / 2);
          p.x += nd (g_rng) * s; p.y += nd (g_rng) * s;
        }
      else if (g_mob == "meander")
        {
          double u, v; g_mea.raw (p.x, p.y, Simulator::Now ().GetSeconds (), u, v);
          p.x += u * g_mea.scale * dt; p.y += v * g_mea.scale * dt;
        }
      else if (g_mob == "random")
        { p.x += g_vel[i].x * dt; p.y += g_vel[i].y * dt; }
      for (double *c : {&p.x, &p.y})
        { if (*c < 0) *c = -*c; if (*c > g_L) *c = 2 * g_L - *c; }
      if (g_speed > 0)
        p.z = std::min (std::max (p.z + nd (g_rng) * 0.05 * g_speed * dt, 1.0), g_L);
      m->SetPosition (p);
    }
  Simulator::Schedule (Seconds (dt), &Move);
}

static void Traffic ()
{
  std::vector<uint32_t> alive;
  for (uint32_t i = 0; i < g_N; i++) if (!g_rt[i]->Dead ()) alive.push_back (i);
  if (!alive.empty ())
    {
      std::uniform_int_distribution<size_t> pick (0, alive.size () - 1);
      g_rt[alive[pick (g_rng)]]->GenData ();
    }
  std::exponential_distribution<double> ex (g_lambda);
  Simulator::Schedule (Seconds (ex (g_rng)), &Traffic);
}

static void Monitor ()
{
  uint32_t dead = 0;
  for (uint32_t i = 0; i < g_N; i++) { if (g_rt[i]->Dead ()) dead++; else g_rt[i]->Tick (); }
  double now = Simulator::Now ().GetSeconds ();
  if (dead > 0 && g_fnd < 0) g_fnd = now;
  if (dead >= 0.10 * g_N) { g_t10 = now; Simulator::Stop (); return; }
  Simulator::Schedule (Seconds (10), &Monitor);
}

static void GlobalRecovery ()
{   // APCDBRP: sinks share their reception statistics and re-flood when PDR is low
  double ratio = AquaSimPbrx::s_winGen > 0 ? (double) AquaSimPbrx::s_winDel / AquaSimPbrx::s_winGen : 1.0;
  if (g_rt[0]->Mode () == 6)
    {   // EEDBR: sinks notify the delivery ratio when it is below the application target
      static bool low = false;
      bool nowLow = ratio < AquaSimPbrx::s_eedbrTarget;
      if (nowLow || low)
        for (size_t s = 0; s < g_sinks.size (); s++) g_rt[g_N + s]->EedbrNotify (nowLow ? ratio : 1.0);
      low = nowLow;
    }
  else if (g_rt[0]->Mode () == 10)
    {   // QLFR multipath suppression: shorten the priority list above the PDR threshold, lengthen it below
      static int L = -1;
      if (L < 0) L = 3;
      int nl = (AquaSimPbrx::s_winGen == 0) ? L : (ratio > AquaSimPbrx::s_qlfrPth ? std::max (1, L - 1) : std::min (8, L + 1));
      if (nl != L) { L = nl; for (size_t s = 0; s < g_sinks.size (); s++) g_rt[g_N + s]->EedbrNotify (L); }
    }
  else if (AquaSimPbrx::s_winGen > 0 && ratio < 0.6)
    for (size_t s = 0; s < g_sinks.size (); s++) g_rt[g_N + s]->Flood (true);
  AquaSimPbrx::s_winGen = AquaSimPbrx::s_winDel = 0;
  Simulator::Schedule (Seconds (200), &GlobalRecovery);
}

int main (int argc, char *argv[])
{
  int mode = 0, seed = 1, sinks = 5;
  std::string channelType = "acoustic";
  double sigmaD = 5, erx = 0.75, idleP = 0.0;
  bool nearq = false;
  CommandLine cmd;
  cmd.AddValue ("mode", "0 DBR,1 RE-PBR,2 SPRE-PBR,3 SPRE-near,4 APCDBRP,6 EEDBR,7 WDFAD-DBR,8 HydroCast,9 DVOR,10 QLFR,11 HFS-PBR", mode);
  cmd.AddValue ("seed", "topology seed", seed);
  cmd.AddValue ("nodes", "sensors", g_N);
  cmd.AddValue ("sinks", "surface sinks", sinks);
  cmd.AddValue ("channel", "acoustic | range", channelType);
  cmd.AddValue ("mobility", "static | random | walk | meander", g_mob);
  cmd.AddValue ("speed", "max speed (m/s)", g_speed);
  cmd.AddValue ("lambda", "network traffic (pkt/s)", g_lambda);
  cmd.AddValue ("tmax", "max simulated time (s)", g_T);
  cmd.AddValue ("sigmaD", "ranging error (m)", sigmaD);
  cmd.AddValue ("erx", "receive power (W)", erx);
  cmd.AddValue ("idle", "idle-listening power (W), 0 = not charged", idleP);
  cmd.AddValue ("near", "RE-PBR/SPRE-PBR link quality decreasing with distance", nearq);
  cmd.AddValue ("eedbrTarget", "EEDBR desired delivery ratio (0 = off)", AquaSimPbrx::s_eedbrTarget);
  cmd.AddValue ("noRouteBit", "sensitivity: ignore the has-route bit", AquaSimPbrx::s_noRouteBit);
  cmd.AddValue ("noSinkShortcut", "sensitivity: no nearest-sink shortcut", AquaSimPbrx::s_noSinkShortcut);
  cmd.AddValue ("qlfrPth", "QLFR PDR threshold of the multipath suppression scheme", AquaSimPbrx::s_qlfrPth);
  cmd.Parse (argc, argv);
  if (mode < 0 || mode > 11 || mode == 5) { fprintf (stderr, "unknown mode %d\n", mode); return 1; }
  Config::SetDefault ("ns3::AquaSimPbrx::Near", BooleanValue (nearq));
  Config::SetDefault ("ns3::AquaSimPbrx::NodeSpeed", DoubleValue (g_speed));

  RngSeedManager::SetSeed (12345);
  RngSeedManager::SetRun (seed);
  g_rng.seed (1000 + seed);
  AquaSimPbrx::ResetStats ();
  AquaSimPbrx::s_numSensors = g_N;

  NodeContainer sensors, sinkNodes;
  sensors.Create (g_N);
  sinkNodes.Create (sinks);

  AquaSimChannelHelper channel = AquaSimChannelHelper::Default ();
  channel.SetPropagation ("ns3::AquaSimRangePropagation");
  AquaSimHelper as = AquaSimHelper::Default ();
  as.SetChannel (channel.Create ());
  as.SetMac ("ns3::AquaSimBroadcastMac", "PacketHeaderSize", IntegerValue (0));
  as.SetRouting ("ns3::AquaSimPbrx", "Mode", IntegerValue (mode),
                 "Acoustic", BooleanValue (channelType == "acoustic"),
                 "SigmaD", DoubleValue (sigmaD));
  as.SetEnergyModel ("ns3::AquaSimEnergyModel", "InitialEnergy", DoubleValue (100.0),
                     "TxPower", DoubleValue (2.0), "RxPower", DoubleValue (erx),
                     "IdlePower", DoubleValue (idleP));

  std::uniform_real_distribution<double> uL (0, g_L), u01 (0, 1);
  MobilityHelper mob;
  mob.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mob.Install (sensors);
  mob.Install (sinkNodes);
  for (uint32_t i = 0; i < g_N; i++)
    {
      double px = uL (g_rng), py = uL (g_rng), pz = uL (g_rng);   // explicit draw order (x, y, z)
      sensors.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (px, py, pz));
      double ang = u01 (g_rng) * 2 * M_PI, sp = u01 (g_rng) * g_speed;
      g_vel.push_back (Vector (sp * std::cos (ang), sp * std::sin (ang), 0));
      g_sensors.push_back (sensors.Get (i));
    }
  for (int s = 0; s < sinks; s++)
    {
      double sxr = uL (g_rng), syr = uL (g_rng);
      sinkNodes.Get (s)->GetObject<MobilityModel> ()->SetPosition (Vector (sxr, syr, 0));
      g_sinks.push_back (sinkNodes.Get (s));
    }

  NetDeviceContainer devs;
  int64_t stream = 100 + 10 * seed;
  auto install = [&] (Ptr<Node> n, bool sink, uint16_t idx) {
      Ptr<AquaSimNetDevice> d = CreateObject<AquaSimNetDevice> ();
      devs.Add (as.Create (n, d));
      d->GetPhy ()->SetTransRange (250);
      d->GetPhy ()->SetAttribute ("Preamble", DoubleValue (0.0));   // Aqua-Sim default 1.5 s; Python model has none
      if (sink) d->EnergyModel ()->SetEnergy (1e9);
      Ptr<AquaSimPbrx> r = DynamicCast<AquaSimPbrx> (d->GetRouting ());
      r->AssignStreams (stream + 7 * idx);
      g_rt.push_back (PeekPointer (r));
      return r;
    };
  std::vector<Ptr<AquaSimPbrx>> keep;
  for (uint32_t i = 0; i < g_N; i++) keep.push_back (install (sensors.Get (i), false, i));
  for (int s = 0; s < sinks; s++) keep.push_back (install (sinkNodes.Get (s), true, g_N + s));
  for (uint32_t i = 0; i < g_N + sinks; i++)
    {
      g_rt[i]->Start (i, i >= g_N);
    }

  if (g_mob == "meander")
    {
      g_mea.x0 = u01 (g_rng) * 7.5; g_mea.y0 = u01 (g_rng) * 2 - 1;
      double vmax = 1e-9;
      for (int a = 0; a < 25; a++) for (int b = 0; b < 25; b++)
        { double u, v; g_mea.raw (a * g_L / 24, b * g_L / 24, 0, u, v); vmax = std::max (vmax, std::hypot (u, v)); }
      g_mea.scale = g_speed / vmax;
    }
  Simulator::Schedule (Seconds (10), &Move);
  Simulator::Schedule (Seconds (100), &Traffic);
  Simulator::Schedule (Seconds (10), &Monitor);
  if (mode == 6 && AquaSimPbrx::s_eedbrTarget > 0) Simulator::Schedule (Seconds (300), &GlobalRecovery);
  if (mode == 10) Simulator::Schedule (Seconds (300), &GlobalRecovery);
  if (mode == 9)
    {   // DVOR: all sinks send query round q at the same time (sonobuoys coordinate by radio)
      double per = 20;
      { DoubleValue dv; g_rt[g_N]->GetAttribute ("DvorPeriod", dv); per = dv.Get (); }
      for (uint32_t q = 1; q * per < g_T + per; q++)
        for (int s = 0; s < sinks; s++)
          Simulator::Schedule (Seconds (1 + (q - 1) * per + 0.1 * s), &AquaSimPbrx::DvorQuery, g_rt[g_N + s], q);
    }
  if (mode == 4)
    {
      for (int s = 0; s < sinks; s++) Simulator::Schedule (Seconds (1 + 0.5 * s), &AquaSimPbrx::Flood, g_rt[g_N + s], true);
      Simulator::Schedule (Seconds (200), &GlobalRecovery);
    }

  Simulator::Stop (Seconds (g_T));
  Simulator::Run ();
  double t = Simulator::Now ().GetSeconds ();
  double used = 0;
  for (uint32_t i = 0; i < g_N; i++) used += 100.0 - std::max (0.0, g_rt[i]->ResidualEnergy ());
  const char *names[] = {"DBR", "RE-PBR", "SPRE-PBR", "SPRE-PBR-near", "APCDBRP", "", "EEDBR", "WDFAD-DBR", "HydroCast", "DVOR", "QLFR", "HFS-PBR"};
  double pdr = AquaSimPbrx::s_gen ? (double) AquaSimPbrx::s_deliv / AquaSimPbrx::s_gen : 0;
  printf ("{\"proto\":\"%s\",\"seed\":%d,\"channel\":\"%s\",\"mobility\":\"%s\",\"speed\":%.2f,"
          "\"nodes\":%u,\"delivered\":%lu,\"gen\":%lu,\"pdr\":%.4f,\"energy\":%.1f,\"J_per_deliv\":%.3f,"
          "\"delay\":%.3f,\"FND\":%.1f,\"T10\":%.1f,\"tx_per_pkt\":%.3f,\"hello\":%lu,\"probes\":%lu,"
          "\"data_tx\":%lu,\"acks\":%lu,\"ctrl\":%lu,\"retx\":%lu,\"sink_dup\":%lu,\"hold_fwd\":%lu,\"drop_void\":%lu,\"drop_retx\":%lu}\n",
          names[mode], seed, channelType.c_str (), g_mob.c_str (), g_speed, g_N,
          (unsigned long) AquaSimPbrx::s_deliv, (unsigned long) AquaSimPbrx::s_gen, pdr, used,
          used / std::max<double> (1, AquaSimPbrx::s_deliv),
          AquaSimPbrx::s_deliv ? AquaSimPbrx::s_delaySum / AquaSimPbrx::s_deliv : 0.0,
          g_fnd < 0 ? t : g_fnd, g_t10 < 0 ? t : g_t10,
          AquaSimPbrx::s_gen ? (double) AquaSimPbrx::s_tx / AquaSimPbrx::s_gen : 0.0,
          (unsigned long) AquaSimPbrx::s_hello, (unsigned long) AquaSimPbrx::s_probe,
          (unsigned long) AquaSimPbrx::s_txType[0], (unsigned long) AquaSimPbrx::s_txType[2],
          (unsigned long) (AquaSimPbrx::s_txType[3] + AquaSimPbrx::s_txType[4] + AquaSimPbrx::s_txType[5] + AquaSimPbrx::s_txType[6] + AquaSimPbrx::s_txType[8] + AquaSimPbrx::s_txType[9]),
          (unsigned long) AquaSimPbrx::s_retx, (unsigned long) AquaSimPbrx::s_dup, (unsigned long) AquaSimPbrx::s_holdFwd,
          (unsigned long) AquaSimPbrx::s_dropVoid, (unsigned long) AquaSimPbrx::s_dropRetx);
  fprintf (stderr, "txType data=%lu hello=%lu ack=%lu probe=%lu preply=%lu hole=%lu beacon=%lu | retx=%lu dropRetx=%lu dropVoid=%lu rxOk=%lu rxLinkLoss=%lu macDrop=%lu\n",
           (unsigned long) AquaSimPbrx::s_txType[0], (unsigned long) AquaSimPbrx::s_txType[1], (unsigned long) AquaSimPbrx::s_txType[2],
           (unsigned long) AquaSimPbrx::s_txType[3], (unsigned long) AquaSimPbrx::s_txType[4], (unsigned long) AquaSimPbrx::s_txType[5],
           (unsigned long) AquaSimPbrx::s_txType[6], (unsigned long) AquaSimPbrx::s_retx, (unsigned long) AquaSimPbrx::s_dropRetx,
           (unsigned long) AquaSimPbrx::s_dropVoid, (unsigned long) AquaSimPbrx::s_rxOk, (unsigned long) AquaSimPbrx::s_rxLinkLoss, (unsigned long) AquaSimBroadcastMac::s_drops);
  fprintf (stderr, "rdisc=%lu rrep=%lu\n", (unsigned long) AquaSimPbrx::s_txType[8], (unsigned long) AquaSimPbrx::s_txType[9]);
  AquaSimPbrx::Report ();
  fprintf (stderr, "holdFwd=%lu dup=%lu\n", (unsigned long) AquaSimPbrx::s_holdFwd, (unsigned long) AquaSimPbrx::s_dup);
  fprintf (stderr, "phy rxN=%lu rxErr=%lu rxT=%.1f txT=%.1f\n", (unsigned long) AquaSimPbrx::s_phyRxN, (unsigned long) AquaSimPbrx::s_phyRxErr, AquaSimPbrx::s_phyRxT, AquaSimPbrx::s_phyTxT);
  Simulator::Destroy ();
  return 0;
}
