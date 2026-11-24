#!/usr/bin/env bash
# Downloads ns-3.38, installs the Aqua-Sim NG module with the benchmark routing module, and builds the scenario.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
NS3="${NS3_DIR:-$HERE/ns-3.38}"
if [ ! -d "$NS3" ]; then
  git clone --branch ns-3.38 --depth 1 https://gitlab.com/nsnam/ns-3-dev.git "$NS3"
fi
rm -rf "$NS3/src/aqua-sim-ng"
cp -r "$HERE/ns3/aqua-sim-ng" "$NS3/src/"
cp "$HERE/ns3/scratch/pbrx-sim.cc" "$NS3/scratch/"
cd "$NS3"
./ns3 configure --build-profile=optimized --enable-modules=aqua-sim-ng
./ns3 build pbrx-sim
echo "Built: $NS3/build/scratch/ns3.38-pbrx-sim-optimized"
