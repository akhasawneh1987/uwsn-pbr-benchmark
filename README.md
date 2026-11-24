# Benchmarking Pressure-Based Routing Protocols in Underwater Wireless Sensor Networks

Source code, scripts and raw results of the article

> A. M. Khasawneh, "Benchmarking Pressure-Based Routing Protocols in Underwater Wireless Sensor Networks Under Unified Settings," submitted to *IEEE Access*, 2026.

Ten pressure-based routing protocols are implemented in one routing module of **Aqua-Sim NG** on **ns-3.38** and evaluated under the same physical, energy, medium access, mobility and traffic models. The repository contains everything needed to rebuild the simulator, repeat every run reported in the article and regenerate its tables and figures.

## Protocols

All protocols were re-implemented from their published descriptions. The `--mode` value selects the protocol.

| Mode | Protocol | Reference |
|---|---|---|
| 0 | DBR | Yan, Shi and Cui, NETWORKING 2008 |
| 1 | RE-PBR | Khasawneh et al., Wireless Networks, 2018 |
| 2 | SPRE-PBR | Khasawneh et al., IEEE Systems Journal, 2020 |
| 4 | APCDBRP | Wang et al., J. Mar. Sci. Eng., 2023 |
| 6 | EEDBR | Wahid and Kim, Int. J. Distrib. Sensor Netw., 2012 |
| 7 | WDFAD-DBR | Yu et al., Ad Hoc Networks, 2016 |
| 8 | HydroCast | Noh et al., IEEE Trans. Veh. Technol., 2016 |
| 9 | DVOR | Guan et al., IEEE Internet Things J., 2019 |
| 10 | QLFR | Zhou, Cao and Xiang, IEEE Internet Things J., 2021 |
| 11 | HFS-PBR | Yan and Xiao, Proc. SPIE, 2023 |

Mode 3 is a SPRE-PBR variant that is not used in the article. RE-PBR and SPRE-PBR are run with `--near=1` (link-quality term that decreases with distance). The implementation choices for values that the papers leave open are documented in Section III of the article and in comments in `ns3/aqua-sim-ng/model/aqua-sim-routing-pbrx.cc`.

**Disclosure.** RE-PBR and SPRE-PBR were proposed in earlier work of the author. Both follow exactly the same rules as the other eight protocols. The code and the raw data are published so that anyone can check or repeat the study.

## Repository layout

```
ns3/aqua-sim-ng/            Aqua-Sim NG module with the routing module (model/aqua-sim-routing-pbrx.{h,cc})
ns3/scratch/pbrx-sim.cc     simulation scenario (deployment, mobility, traffic, energy, output)
scripts/                    run scripts (main study, add-on experiment, robustness, tuning, sensitivity)
analysis/                   scripts that produce the tables and figures of the article
results/main/               main study: 14 scenarios x 10 protocols x 20 topologies (2,800 runs)
results/addon/              add-on experiment: route flag + nearest-sink shortcut on (2,800 runs)
results/robustness/         interaction, idle-listening and tuning-sensitivity runs (1,300 runs) and derived rankings
results/sensitivity/        add-on set with one feature switched off (base case and 1 m/s current)
results/tuning/             tuning runs on the training topologies (seeds 101-103)
figures/                    figures of the article (PDF)
setup.sh                    downloads ns-3.38, installs the module and builds the scenario
```

## Build

Requirements: a C++ compiler supported by ns-3.38, CMake, Python 3 (with `numpy`, `scipy` and `matplotlib` for the analysis).

```bash
./setup.sh            # clones ns-3.38 into ./ns-3.38, copies the module and the scenario, builds
```

or manually:

```bash
git clone --branch ns-3.38 --depth 1 https://gitlab.com/nsnam/ns-3-dev.git ns-3.38
cp -r ns3/aqua-sim-ng ns-3.38/src/
cp ns3/scratch/pbrx-sim.cc ns-3.38/scratch/
cd ns-3.38
./ns3 configure --build-profile=optimized --enable-modules=aqua-sim-ng
./ns3 build pbrx-sim
```

With recent compilers (GCC 13 or newer) ns-3.38 may need `#include <cstdint>` added to a few core headers; the build error names the file.

## Running one simulation

```bash
cd ns-3.38
export LD_LIBRARY_PATH=build/lib
./build/scratch/ns3.38-pbrx-sim-optimized --mode=11 --seed=11 --nodes=400 --sinks=5 \
    --lambda=1 --channel=acoustic --mobility=walk --speed=1 --near=1 --tmax=200000
```

Main options: `--mode`, `--seed` (topology), `--nodes`, `--sinks`, `--lambda` (packets/s in the network), `--channel` (`acoustic` or `range`), `--mobility` (`static`, `walk`, `meander`), `--speed` (m/s), `--idle` (idle-listening power in W, 0 by default), `--noRouteBit` and `--noSinkShortcut` (both 1 by default, which gives the protocols as published). A run stops when 10% of the sensors have exhausted their energy. The last line of the output is one JSON record.

Output fields: `delivered` (packets delivered before T10, the main metric), `gen`, `pdr`, `energy`, `J_per_deliv`, `delay`, `FND`, `T10`, `tx_per_pkt`, `hello`, `probes`, `data_tx`, `acks`, `ctrl`, `retx`, `sink_dup`, `hold_fwd`, `drop_void`, `drop_retx`.

## Reproducing the article

All scripts are resumable and take a time budget in seconds and the number of parallel runs, for example `python3 scripts/run_main.py 3600 4`. Set `NS3_DIR` if ns-3.38 is not in `./ns-3.38`.

| Result | Script | Output |
|---|---|---|
| Main study (Tables 4 to 7, Figs. 2 to 9) | `scripts/run_main.py` | `results/main/main_faithful.jsonl` |
| Add-on experiment (Fig. 11) | `scripts/run_addon.py` | `results/addon/addon_on.jsonl` |
| One add-on switched off | `scripts/run_sens2.py BUDGET PAR route\|sink` | `results/sensitivity/` |
| Interaction, idle energy, tuning sensitivity (Fig. 10, Table 8) | `scripts/run_extra.py` | `results/robustness/extra.jsonl` |
| Tuning on training topologies (Table 3) | `scripts/tune.py` | `results/tuning/` |

Tables and figures:

```bash
python3 scripts/analyze_main.py results/main/main_faithful.jsonl results/main/main_tables.md
python3 analysis/group_a.py        # alternative rankings, TOPSIS, FND, overhead
python3 analysis/figs.py           # heatmap, sweeps, trade-off, energy/delay, add-on figures
python3 analysis/figs_a.py         # critical difference diagram, overhead figure
python3 analysis/fig_interact.py   # density x mobility figure
python3 analysis/extra_an.py idle_base idle_static n200_static n300_static n200_cur1 n300_cur1
```

All runs use fixed random seeds, so the results are reproduced exactly on the same platform.

## License

The simulation code is distributed under the GNU General Public License version 2 (see `LICENSE`), the license of ns-3 and Aqua-Sim NG. The result files and figures are released under the Creative Commons Attribution 4.0 International license (CC BY 4.0).

## Acknowledgments

The routing module is built on Aqua-Sim NG (R. Martin, S. Rajasekaran and Z. Peng, University of Connecticut) and ns-3.

## Citation

If you use this code or data, please cite the article (citation will be updated after publication) and the archived release (DOI on the Zenodo record).
