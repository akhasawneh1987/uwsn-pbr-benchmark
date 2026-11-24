# Main benchmark results

Mean over 20 topologies (seeds 11–30) ± 95% CI for the main metric. Rank 1 = most packets delivered before 10% of sensors die.

"vs next" = paired two-sided Wilcoxon signed-rank test between rank 1 and rank 2 (same topologies).


## base — 400 nodes, 5 sinks, 1 pkt/s, acoustic, walk 1 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1518 ± 185 | 0.407 | 18.2 | 2.60 | 3778 | 9.5 |
| 2 | RE-PBR | 20 | 1432 ± 186 | 0.400 | 19.7 | 2.74 | 3624 | 10.2 |
| 3 | QLFR | 20 | 1352 ± 107 | 0.562 | 20.0 | 1.95 | 2491 | 9.6 |
| 4 | HFS-PBR | 20 | 1348 ± 103 | 0.558 | 16.8 | 1.56 | 2503 | 14.1 |
| 5 | WDFAD-DBR | 20 | 1135 ± 81 | 0.473 | 19.0 | 1.95 | 2493 | 17.2 |
| 6 | DVOR | 20 | 1005 ± 55 | 0.524 | 26.5 | 2.58 | 2004 | 8.5 |
| 7 | EEDBR | 20 | 987 ± 70 | 0.612 | 22.4 | 8.59 | 1705 | 20.0 |
| 8 | APCDBRP | 20 | 829 ± 89 | 0.455 | 26.9 | 6.34 | 1902 | 7.8 |
| 9 | HydroCast | 20 | 800 ± 106 | 0.483 | 36.7 | 8.87 | 1744 | 6.8 |
| 10 | DBR | 20 | 608 ± 33 | 0.739 | 31.3 | 2.22 | 915 | 53.8 |

SPRE-PBR vs RE-PBR: +6.0%, better on 18/20 topologies, p = 0.00014.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. DVOR, 4. EEDBR, 5. DBR.

## nodes100 — 100 nodes

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | HFS-PBR | 20 | 1397 ± 160 | 0.094 | 3.9 | 0.53 | 16882 | 0.5 |
| 2 | APCDBRP | 20 | 989 ± 153 | 0.083 | 6.7 | 1.05 | 12593 | 0.3 |
| 3 | DVOR | 20 | 958 ± 89 | 0.089 | 3.5 | 1.31 | 23596 | 0.6 |
| 4 | QLFR | 20 | 937 ± 147 | 0.136 | 6.1 | 0.68 | 7258 | 2.0 |
| 5 | DBR | 20 | 893 ± 150 | 0.143 | 6.0 | 0.87 | 6496 | 3.4 |
| 6 | EEDBR | 20 | 881 ± 156 | 0.116 | 6.7 | 1.15 | 7809 | 1.9 |
| 7 | WDFAD-DBR | 20 | 810 ± 165 | 0.089 | 6.7 | 0.73 | 9410 | 2.1 |
| 8 | SPRE-PBR | 20 | 627 ± 118 | 0.138 | 10.2 | 1.15 | 4667 | 3.8 |
| 9 | RE-PBR | 20 | 626 ± 117 | 0.137 | 10.1 | 1.13 | 4678 | 3.8 |
| 10 | HydroCast | 20 | 518 ± 84 | 0.165 | 12.4 | 2.75 | 3324 | 2.5 |

HFS-PBR vs APCDBRP: +41.3%, better on 20/20 topologies, p = 8.8e-05.
Among protocols with mean PDR ≥ 0.5: none.

## nodes200 — 200 nodes

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | HFS-PBR | 20 | 1268 ± 135 | 0.237 | 8.6 | 1.14 | 5770 | 3.7 |
| 2 | EEDBR | 20 | 1037 ± 145 | 0.231 | 12.3 | 3.66 | 4712 | 5.2 |
| 3 | QLFR | 20 | 1035 ± 131 | 0.314 | 11.6 | 1.72 | 3495 | 6.4 |
| 4 | WDFAD-DBR | 20 | 935 ± 148 | 0.200 | 12.9 | 1.51 | 4818 | 6.1 |
| 5 | DBR | 20 | 882 ± 123 | 0.343 | 11.9 | 1.95 | 2736 | 12.0 |
| 6 | SPRE-PBR | 20 | 872 ± 141 | 0.215 | 15.5 | 2.14 | 4125 | 6.8 |
| 7 | RE-PBR | 20 | 860 ± 136 | 0.214 | 15.5 | 2.15 | 4094 | 6.8 |
| 8 | DVOR | 20 | 844 ± 83 | 0.310 | 14.8 | 2.67 | 2866 | 5.0 |
| 9 | APCDBRP | 20 | 806 ± 125 | 0.167 | 12.7 | 2.95 | 5394 | 2.1 |
| 10 | HydroCast | 20 | 615 ± 99 | 0.267 | 22.3 | 6.22 | 2445 | 4.9 |

HFS-PBR vs EEDBR: +22.3%, better on 19/20 topologies, p = 3.8e-06.
Among protocols with mean PDR ≥ 0.5: none.

## nodes300 — 300 nodes

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | HFS-PBR | 20 | 1401 ± 90 | 0.419 | 11.8 | 1.48 | 3556 | 8.4 |
| 2 | SPRE-PBR | 20 | 1210 ± 150 | 0.312 | 16.8 | 2.39 | 3966 | 8.4 |
| 3 | RE-PBR | 20 | 1181 ± 148 | 0.313 | 17.4 | 2.44 | 3871 | 8.7 |
| 4 | QLFR | 20 | 1164 ± 127 | 0.521 | 16.0 | 2.11 | 2300 | 10.6 |
| 5 | EEDBR | 20 | 1096 ± 90 | 0.423 | 15.8 | 6.46 | 2720 | 11.3 |
| 6 | WDFAD-DBR | 20 | 1078 ± 88 | 0.366 | 15.0 | 1.90 | 3067 | 12.4 |
| 7 | DVOR | 20 | 1018 ± 61 | 0.456 | 19.3 | 2.70 | 2316 | 6.9 |
| 8 | HydroCast | 20 | 746 ± 99 | 0.384 | 27.9 | 7.80 | 2048 | 6.2 |
| 9 | APCDBRP | 20 | 744 ± 89 | 0.333 | 21.1 | 5.61 | 2433 | 5.7 |
| 10 | DBR | 20 | 730 ± 49 | 0.593 | 19.3 | 2.28 | 1340 | 30.6 |

HFS-PBR vs SPRE-PBR: +15.8%, better on 15/20 topologies, p = 0.0012.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. DBR.

## static — static

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1520 ± 200 | 0.421 | 18.0 | 2.60 | 3694 | 9.7 |
| 2 | RE-PBR | 20 | 1422 ± 201 | 0.408 | 19.7 | 2.74 | 3564 | 10.3 |
| 3 | QLFR | 20 | 1372 ± 111 | 0.536 | 20.0 | 1.74 | 2680 | 8.6 |
| 4 | HFS-PBR | 20 | 1332 ± 94 | 0.552 | 16.9 | 1.56 | 2516 | 14.0 |
| 5 | WDFAD-DBR | 20 | 1161 ± 149 | 0.226 | 20.1 | 1.60 | 5279 | 9.6 |
| 6 | DVOR | 20 | 1010 ± 54 | 0.526 | 26.4 | 2.59 | 2010 | 8.5 |
| 7 | EEDBR | 20 | 977 ± 71 | 0.607 | 22.5 | 8.15 | 1706 | 20.0 |
| 8 | APCDBRP | 20 | 964 ± 98 | 0.468 | 22.4 | 5.20 | 2154 | 7.2 |
| 9 | HydroCast | 20 | 797 ± 118 | 0.481 | 37.0 | 8.73 | 1752 | 6.6 |
| 10 | DBR | 20 | 612 ± 35 | 0.742 | 31.1 | 2.22 | 922 | 53.7 |

SPRE-PBR vs RE-PBR: +6.9%, better on 17/20 topologies, p = 0.00037.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. DVOR, 4. EEDBR, 5. DBR.

## walk3 — walk 3 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1373 ± 154 | 0.343 | 20.7 | 2.37 | 4042 | 9.0 |
| 2 | HFS-PBR | 20 | 1325 ± 85 | 0.554 | 17.3 | 1.60 | 2482 | 14.5 |
| 3 | QLFR | 20 | 1319 ± 114 | 0.560 | 20.8 | 2.28 | 2440 | 10.0 |
| 4 | RE-PBR | 20 | 1288 ± 146 | 0.333 | 22.3 | 2.52 | 3902 | 9.6 |
| 5 | DVOR | 20 | 1019 ± 53 | 0.525 | 26.5 | 2.57 | 2024 | 8.5 |
| 6 | EEDBR | 20 | 977 ± 80 | 0.600 | 23.0 | 9.97 | 1713 | 19.7 |
| 7 | WDFAD-DBR | 20 | 929 ± 66 | 0.483 | 23.9 | 2.04 | 2008 | 18.9 |
| 8 | HydroCast | 20 | 774 ± 97 | 0.472 | 37.0 | 9.23 | 1724 | 7.1 |
| 9 | APCDBRP | 20 | 722 ± 59 | 0.412 | 32.8 | 7.58 | 1840 | 7.9 |
| 10 | DBR | 20 | 606 ± 33 | 0.748 | 31.4 | 2.23 | 902 | 54.4 |

SPRE-PBR vs HFS-PBR: +3.6%, better on 14/20 topologies, p = 0.43.
Among protocols with mean PDR ≥ 0.5: 1. HFS-PBR, 2. QLFR, 3. DVOR, 4. EEDBR, 5. DBR.

## cur0.3 — current 0.3 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1286 ± 147 | 0.418 | 20.2 | 2.59 | 3174 | 9.9 |
| 2 | QLFR | 20 | 1275 ± 71 | 0.564 | 21.2 | 1.93 | 2387 | 9.5 |
| 3 | HFS-PBR | 20 | 1260 ± 67 | 0.569 | 18.0 | 1.56 | 2356 | 14.8 |
| 4 | RE-PBR | 20 | 1218 ± 132 | 0.415 | 21.9 | 2.83 | 3042 | 10.9 |
| 5 | WDFAD-DBR | 20 | 1183 ± 95 | 0.432 | 18.7 | 1.86 | 2863 | 15.8 |
| 6 | DVOR | 20 | 967 ± 47 | 0.524 | 27.9 | 2.57 | 1960 | 8.7 |
| 7 | EEDBR | 20 | 960 ± 65 | 0.597 | 23.6 | 8.52 | 1722 | 19.8 |
| 8 | APCDBRP | 20 | 785 ± 65 | 0.457 | 28.2 | 6.77 | 1827 | 8.0 |
| 9 | HydroCast | 20 | 768 ± 92 | 0.480 | 37.0 | 8.90 | 1712 | 6.8 |
| 10 | DBR | 20 | 608 ± 32 | 0.744 | 31.4 | 2.21 | 916 | 54.0 |

SPRE-PBR vs QLFR: +0.9%, better on 10/20 topologies, p = 0.99.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. DVOR, 4. EEDBR, 5. DBR.

## cur1 — current 1 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | QLFR | 20 | 865 ± 62 | 0.563 | 27.7 | 2.31 | 1640 | 10.7 |
| 2 | HFS-PBR | 20 | 835 ± 59 | 0.563 | 25.6 | 1.56 | 1600 | 17.4 |
| 3 | EEDBR | 20 | 725 ± 48 | 0.581 | 29.8 | 10.24 | 1358 | 19.2 |
| 4 | DVOR | 20 | 688 ± 30 | 0.516 | 35.6 | 2.62 | 1438 | 10.0 |
| 5 | SPRE-PBR | 20 | 664 ± 102 | 0.343 | 35.5 | 2.46 | 2033 | 9.6 |
| 6 | WDFAD-DBR | 20 | 655 ± 64 | 0.380 | 32.6 | 1.81 | 1837 | 16.4 |
| 7 | RE-PBR | 20 | 582 ± 76 | 0.333 | 40.8 | 2.82 | 1852 | 11.6 |
| 8 | DBR | 20 | 555 ± 24 | 0.732 | 34.6 | 2.22 | 855 | 55.2 |
| 9 | HydroCast | 20 | 492 ± 58 | 0.443 | 50.7 | 8.96 | 1208 | 7.2 |
| 10 | APCDBRP | 20 | 446 ± 41 | 0.368 | 48.6 | 7.61 | 1318 | 8.3 |

QLFR vs HFS-PBR: +3.5%, better on 13/20 topologies, p = 0.05.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. EEDBR, 4. DVOR, 5. DBR.

## load0.5 — 0.5 pkt/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1230 ± 127 | 0.407 | 23.1 | 2.40 | 6108 | 9.5 |
| 2 | HFS-PBR | 20 | 1193 ± 75 | 0.595 | 19.8 | 1.53 | 4114 | 14.7 |
| 3 | RE-PBR | 20 | 1152 ± 132 | 0.394 | 25.0 | 2.49 | 5910 | 10.1 |
| 4 | WDFAD-DBR | 20 | 1003 ± 60 | 0.507 | 21.5 | 1.95 | 4074 | 18.5 |
| 5 | QLFR | 20 | 930 ± 42 | 0.562 | 30.8 | 1.82 | 3426 | 8.9 |
| 6 | APCDBRP | 20 | 915 ± 94 | 0.523 | 25.8 | 5.93 | 3578 | 7.8 |
| 7 | EEDBR | 20 | 908 ± 61 | 0.627 | 25.4 | 8.29 | 3000 | 20.2 |
| 8 | DVOR | 20 | 653 ± 26 | 0.549 | 43.0 | 2.55 | 2490 | 8.6 |
| 9 | DBR | 20 | 574 ± 27 | 0.781 | 32.1 | 2.11 | 1570 | 57.7 |
| 10 | HydroCast | 20 | 495 ± 73 | 0.492 | 60.9 | 7.91 | 2122 | 6.5 |

SPRE-PBR vs HFS-PBR: +3.1%, better on 10/20 topologies, p = 0.44.
Among protocols with mean PDR ≥ 0.5: 1. HFS-PBR, 2. WDFAD-DBR, 3. QLFR, 4. APCDBRP, 5. EEDBR, 6. DVOR, 7. DBR.

## load2 — 2 pkt/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | QLFR | 20 | 1603 ± 187 | 0.545 | 16.1 | 2.19 | 1559 | 10.5 |
| 2 | SPRE-PBR | 20 | 1515 ± 186 | 0.367 | 17.6 | 3.13 | 2148 | 9.6 |
| 3 | HFS-PBR | 20 | 1421 ± 110 | 0.515 | 15.5 | 1.62 | 1475 | 13.4 |
| 4 | RE-PBR | 20 | 1396 ± 170 | 0.350 | 19.5 | 3.32 | 2082 | 10.2 |
| 5 | DVOR | 20 | 1359 ± 89 | 0.482 | 18.6 | 2.60 | 1502 | 8.2 |
| 6 | WDFAD-DBR | 20 | 1272 ± 104 | 0.431 | 17.3 | 1.91 | 1572 | 15.1 |
| 7 | HydroCast | 20 | 1072 ± 142 | 0.466 | 25.5 | 11.80 | 1246 | 7.3 |
| 8 | EEDBR | 20 | 1006 ± 82 | 0.568 | 21.4 | 9.01 | 978 | 19.0 |
| 9 | DBR | 20 | 677 ± 38 | 0.669 | 29.4 | 2.39 | 604 | 46.6 |
| 10 | APCDBRP | 20 | 669 ± 82 | 0.333 | 32.2 | 7.37 | 1108 | 7.2 |

QLFR vs SPRE-PBR: +5.8%, better on 13/20 topologies, p = 0.055.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. EEDBR, 4. DBR.

## range — ideal range channel

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | APCDBRP | 20 | 1866 ± 168 | 0.612 | 11.2 | 3.92 | 3110 | 6.2 |
| 2 | QLFR | 20 | 1805 ± 169 | 0.607 | 15.9 | 1.38 | 3031 | 7.1 |
| 3 | SPRE-PBR | 20 | 1716 ± 199 | 0.430 | 16.1 | 1.94 | 4027 | 8.8 |
| 4 | RE-PBR | 20 | 1588 ± 199 | 0.420 | 17.9 | 2.14 | 3822 | 9.6 |
| 5 | HFS-PBR | 20 | 1376 ± 89 | 0.693 | 15.9 | 1.44 | 2069 | 17.7 |
| 6 | DVOR | 20 | 1295 ± 65 | 0.751 | 19.8 | 2.66 | 1812 | 10.7 |
| 7 | WDFAD-DBR | 20 | 1217 ± 80 | 0.568 | 17.4 | 1.85 | 2238 | 19.5 |
| 8 | EEDBR | 20 | 956 ± 60 | 0.698 | 22.8 | 7.26 | 1460 | 24.3 |
| 9 | HydroCast | 20 | 892 ± 110 | 0.528 | 31.7 | 6.24 | 1775 | 5.9 |
| 10 | DBR | 20 | 580 ± 26 | 0.801 | 33.0 | 2.00 | 818 | 63.1 |

APCDBRP vs QLFR: +3.4%, better on 12/20 topologies, p = 0.23.
Among protocols with mean PDR ≥ 0.5: 1. APCDBRP, 2. QLFR, 3. HFS-PBR, 4. DVOR, 5. WDFAD-DBR, 6. EEDBR, 7. HydroCast, 8. DBR.

## sinks1 — 1 sink

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | HFS-PBR | 20 | 586 ± 76 | 0.198 | 46.0 | 1.92 | 3243 | 9.7 |
| 2 | DVOR | 20 | 560 ± 42 | 0.273 | 46.2 | 3.68 | 14842 | 8.1 |
| 3 | QLFR | 20 | 519 ± 53 | 0.375 | 42.9 | 3.45 | 1480 | 21.1 |
| 4 | EEDBR | 20 | 442 ± 71 | 0.271 | 70.3 | 11.42 | 1734 | 19.7 |
| 5 | SPRE-PBR | 20 | 415 ± 85 | 0.126 | 85.2 | 2.75 | 3364 | 10.5 |
| 6 | RE-PBR | 20 | 398 ± 87 | 0.123 | 148.1 | 2.78 | 3278 | 11.2 |
| 7 | WDFAD-DBR | 20 | 389 ± 60 | 0.162 | 63.7 | 2.23 | 2512 | 16.9 |
| 8 | DBR | 20 | 343 ± 35 | 0.423 | 58.2 | 3.05 | 915 | 54.1 |
| 9 | APCDBRP | 20 | 308 ± 55 | 0.130 | 84.6 | 6.76 | 2629 | 5.5 |
| 10 | HydroCast | 20 | 217 ± 41 | 0.134 | 208.5 | 8.50 | 1710 | 7.1 |

HFS-PBR vs DVOR: +4.7%, better on 13/20 topologies, p = 0.16.
Among protocols with mean PDR ≥ 0.5: none.

## sinks3 — 3 sinks

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | QLFR | 20 | 1186 ± 118 | 0.547 | 21.3 | 2.33 | 2250 | 12.5 |
| 2 | HFS-PBR | 20 | 1164 ± 73 | 0.464 | 19.5 | 1.68 | 2618 | 13.4 |
| 3 | SPRE-PBR | 20 | 1155 ± 154 | 0.323 | 24.4 | 2.70 | 3648 | 9.9 |
| 4 | RE-PBR | 20 | 1082 ± 158 | 0.314 | 26.6 | 2.83 | 3516 | 10.5 |
| 5 | WDFAD-DBR | 20 | 940 ± 56 | 0.393 | 22.8 | 2.10 | 2507 | 17.1 |
| 6 | DVOR | 20 | 897 ± 48 | 0.470 | 29.7 | 2.80 | 2002 | 8.6 |
| 7 | EEDBR | 20 | 861 ± 64 | 0.531 | 25.8 | 9.50 | 1716 | 19.9 |
| 8 | APCDBRP | 20 | 666 ± 76 | 0.350 | 32.6 | 6.54 | 2002 | 7.5 |
| 9 | HydroCast | 20 | 627 ± 99 | 0.382 | 48.2 | 9.26 | 1730 | 6.9 |
| 10 | DBR | 20 | 556 ± 26 | 0.687 | 33.9 | 2.44 | 906 | 54.0 |

QLFR vs HFS-PBR: +1.9%, better on 11/20 topologies, p = 0.31.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. EEDBR, 3. DBR.

## sinks7 — 7 sinks

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1829 ± 187 | 0.473 | 15.0 | 2.51 | 3927 | 9.2 |
| 2 | RE-PBR | 20 | 1717 ± 190 | 0.462 | 16.3 | 2.65 | 3776 | 9.8 |
| 3 | HFS-PBR | 20 | 1503 ± 110 | 0.631 | 15.1 | 1.47 | 2468 | 14.7 |
| 4 | QLFR | 20 | 1397 ± 51 | 0.572 | 20.3 | 1.68 | 2545 | 7.7 |
| 5 | WDFAD-DBR | 20 | 1304 ± 70 | 0.545 | 16.4 | 1.83 | 2496 | 17.2 |
| 6 | DVOR | 20 | 1098 ± 54 | 0.567 | 24.4 | 2.43 | 2028 | 8.4 |
| 7 | EEDBR | 20 | 1086 ± 76 | 0.665 | 20.3 | 7.68 | 1729 | 19.7 |
| 8 | APCDBRP | 20 | 994 ± 85 | 0.538 | 22.5 | 6.06 | 1934 | 7.9 |
| 9 | HydroCast | 20 | 968 ± 105 | 0.578 | 28.7 | 8.57 | 1760 | 6.6 |
| 10 | DBR | 20 | 652 ± 31 | 0.787 | 29.0 | 2.06 | 926 | 53.4 |

SPRE-PBR vs RE-PBR: +6.6%, better on 20/20 topologies, p = 8.8e-05.
Among protocols with mean PDR ≥ 0.5: 1. HFS-PBR, 2. QLFR, 3. WDFAD-DBR, 4. DVOR, 5. EEDBR, 6. APCDBRP, 7. HydroCast, 8. DBR.

## Overall (14 cases)

| protocol | mean rank | best rank | worst rank | cases ranked 1st | mean rank among PDR ≥ 0.5 (cases) |
|---|---|---|---|---|---|
| HFS-PBR | 2.4 | 1 | 5 | 4 | 1.8 (9) |
| QLFR | 2.8 | 1 | 5 | 3 | 1.5 (11) |
| SPRE-PBR | 2.9 | 1 | 8 | 6 | — (0) |
| RE-PBR | 4.4 | 2 | 9 | 0 | — (0) |
| DVOR | 5.6 | 2 | 8 | 0 | 3.8 (8) |
| WDFAD-DBR | 5.6 | 4 | 7 | 0 | 3.3 (3) |
| EEDBR | 6.0 | 2 | 8 | 0 | 4.0 (10) |
| APCDBRP | 7.5 | 1 | 10 | 1 | 3.7 (3) |
| DBR | 8.9 | 5 | 10 | 0 | 5.2 (11) |
| HydroCast | 9.0 | 7 | 10 | 0 | 7.0 (2) |
