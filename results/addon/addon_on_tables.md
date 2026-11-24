# Main benchmark results

Mean over 20 topologies (seeds 11–30) ± 95% CI for the main metric. Rank 1 = most packets delivered before 10% of sensors die.

"vs next" = paired two-sided Wilcoxon signed-rank test between rank 1 and rank 2 (same topologies).


## base — 400 nodes, 5 sinks, 1 pkt/s, acoustic, walk 1 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 2177 ± 192 | 0.624 | 11.6 | 3.17 | 3538 | 9.3 |
| 2 | RE-PBR | 20 | 2073 ± 196 | 0.615 | 12.4 | 3.35 | 3426 | 9.9 |
| 3 | QLFR | 20 | 1460 ± 102 | 0.562 | 19.0 | 1.92 | 2672 | 8.1 |
| 4 | HFS-PBR | 20 | 1350 ± 103 | 0.558 | 16.8 | 1.56 | 2506 | 14.1 |
| 5 | EEDBR | 20 | 1160 ± 85 | 0.607 | 19.4 | 7.16 | 2002 | 16.3 |
| 6 | WDFAD-DBR | 20 | 1135 ± 81 | 0.473 | 19.0 | 1.95 | 2493 | 17.2 |
| 7 | DVOR | 20 | 1005 ± 55 | 0.524 | 26.5 | 2.58 | 2004 | 8.5 |
| 8 | HydroCast | 20 | 997 ± 114 | 0.641 | 27.5 | 4.96 | 1640 | 7.8 |
| 9 | APCDBRP | 20 | 829 ± 89 | 0.455 | 26.9 | 6.34 | 1902 | 7.8 |
| 10 | DBR | 20 | 608 ± 33 | 0.739 | 31.3 | 2.22 | 915 | 53.8 |

SPRE-PBR vs RE-PBR: +5.0%, better on 19/20 topologies, p = 0.00019.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. HFS-PBR, 5. EEDBR, 6. DVOR, 7. HydroCast, 8. DBR.

## nodes100 — 100 nodes

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | QLFR | 20 | 1463 ± 182 | 0.100 | 3.8 | 0.57 | 16038 | 0.5 |
| 2 | EEDBR | 20 | 1431 ± 188 | 0.090 | 4.0 | 0.70 | 17506 | 0.4 |
| 3 | RE-PBR | 20 | 1414 ± 165 | 0.131 | 3.4 | 1.41 | 12551 | 0.8 |
| 4 | SPRE-PBR | 20 | 1409 ± 168 | 0.131 | 3.4 | 1.42 | 12515 | 0.8 |
| 5 | HFS-PBR | 20 | 1390 ± 159 | 0.094 | 3.9 | 0.53 | 16826 | 0.5 |
| 6 | APCDBRP | 20 | 989 ± 153 | 0.083 | 6.7 | 1.05 | 12593 | 0.3 |
| 7 | DVOR | 20 | 958 ± 89 | 0.089 | 3.5 | 1.31 | 23596 | 0.6 |
| 8 | DBR | 20 | 893 ± 150 | 0.143 | 6.0 | 0.87 | 6496 | 3.4 |
| 9 | WDFAD-DBR | 20 | 810 ± 165 | 0.089 | 6.7 | 0.73 | 9410 | 2.1 |
| 10 | HydroCast | 20 | 564 ± 93 | 0.189 | 10.8 | 2.62 | 3154 | 2.4 |

QLFR vs EEDBR: +2.2%, better on 14/20 topologies, p = 0.11.
Among protocols with mean PDR ≥ 0.5: none.

## nodes200 — 200 nodes

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1305 ± 182 | 0.340 | 8.2 | 3.18 | 4173 | 5.2 |
| 2 | RE-PBR | 20 | 1284 ± 176 | 0.344 | 8.3 | 3.14 | 4046 | 5.4 |
| 3 | HFS-PBR | 20 | 1271 ± 138 | 0.236 | 8.6 | 1.14 | 5814 | 3.7 |
| 4 | EEDBR | 20 | 1215 ± 151 | 0.219 | 9.2 | 2.69 | 5994 | 3.3 |
| 5 | QLFR | 20 | 1208 ± 147 | 0.287 | 9.1 | 1.61 | 4586 | 4.1 |
| 6 | WDFAD-DBR | 20 | 935 ± 148 | 0.200 | 12.9 | 1.51 | 4818 | 6.1 |
| 7 | DBR | 20 | 882 ± 123 | 0.343 | 11.9 | 1.95 | 2736 | 12.0 |
| 8 | DVOR | 20 | 844 ± 83 | 0.310 | 14.8 | 2.67 | 2866 | 5.0 |
| 9 | APCDBRP | 20 | 806 ± 125 | 0.167 | 12.7 | 2.95 | 5394 | 2.1 |
| 10 | HydroCast | 20 | 755 ± 114 | 0.384 | 18.3 | 4.98 | 2096 | 6.0 |

SPRE-PBR vs RE-PBR: +1.7%, better on 10/20 topologies, p = 0.18.
Among protocols with mean PDR ≥ 0.5: none.

## nodes300 — 300 nodes

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1783 ± 151 | 0.520 | 9.8 | 3.38 | 3539 | 8.2 |
| 2 | RE-PBR | 20 | 1768 ± 145 | 0.517 | 9.9 | 3.38 | 3530 | 8.4 |
| 3 | HFS-PBR | 20 | 1394 ± 93 | 0.420 | 11.9 | 1.48 | 3523 | 8.5 |
| 4 | EEDBR | 20 | 1289 ± 101 | 0.426 | 12.8 | 5.11 | 3208 | 8.6 |
| 5 | QLFR | 20 | 1268 ± 105 | 0.506 | 14.2 | 2.09 | 2623 | 8.2 |
| 6 | WDFAD-DBR | 20 | 1078 ± 88 | 0.366 | 15.0 | 1.90 | 3067 | 12.4 |
| 7 | DVOR | 20 | 1018 ± 61 | 0.456 | 19.3 | 2.70 | 2316 | 6.9 |
| 8 | HydroCast | 20 | 930 ± 109 | 0.536 | 21.2 | 5.02 | 1830 | 7.4 |
| 9 | APCDBRP | 20 | 744 ± 89 | 0.333 | 21.1 | 5.61 | 2433 | 5.7 |
| 10 | DBR | 20 | 730 ± 49 | 0.593 | 19.3 | 2.28 | 1340 | 30.6 |

SPRE-PBR vs RE-PBR: +0.9%, better on 12/20 topologies, p = 0.42.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. HydroCast, 5. DBR.

## static — static

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 2222 ± 200 | 0.673 | 11.0 | 3.29 | 3396 | 9.6 |
| 2 | RE-PBR | 20 | 2142 ± 209 | 0.671 | 11.7 | 3.49 | 3278 | 10.2 |
| 3 | QLFR | 20 | 1473 ± 96 | 0.552 | 18.6 | 1.73 | 2777 | 7.6 |
| 4 | HFS-PBR | 20 | 1340 ± 96 | 0.555 | 16.8 | 1.55 | 2520 | 14.0 |
| 5 | WDFAD-DBR | 20 | 1161 ± 149 | 0.226 | 20.1 | 1.60 | 5279 | 9.6 |
| 6 | EEDBR | 20 | 1130 ± 83 | 0.603 | 19.6 | 6.72 | 1974 | 16.5 |
| 7 | DVOR | 20 | 1010 ± 54 | 0.526 | 26.4 | 2.59 | 2010 | 8.5 |
| 8 | HydroCast | 20 | 1002 ± 112 | 0.645 | 27.1 | 5.05 | 1648 | 7.7 |
| 9 | APCDBRP | 20 | 964 ± 98 | 0.468 | 22.4 | 5.20 | 2154 | 7.2 |
| 10 | DBR | 20 | 612 ± 35 | 0.742 | 31.1 | 2.22 | 922 | 53.7 |

SPRE-PBR vs RE-PBR: +3.7%, better on 18/20 topologies, p = 8.2e-05.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. HFS-PBR, 5. EEDBR, 6. DVOR, 7. HydroCast, 8. DBR.

## walk3 — walk 3 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1919 ± 180 | 0.498 | 14.0 | 2.82 | 3896 | 8.8 |
| 2 | RE-PBR | 20 | 1821 ± 172 | 0.487 | 15.0 | 3.01 | 3778 | 9.3 |
| 3 | QLFR | 20 | 1398 ± 108 | 0.560 | 20.0 | 2.28 | 2576 | 8.8 |
| 4 | HFS-PBR | 20 | 1331 ± 92 | 0.552 | 17.3 | 1.61 | 2500 | 14.5 |
| 5 | EEDBR | 20 | 1141 ± 91 | 0.604 | 20.0 | 8.53 | 1972 | 16.5 |
| 6 | DVOR | 20 | 1019 ± 53 | 0.525 | 26.5 | 2.57 | 2024 | 8.5 |
| 7 | HydroCast | 20 | 937 ± 109 | 0.601 | 29.8 | 4.88 | 1642 | 7.9 |
| 8 | WDFAD-DBR | 20 | 929 ± 66 | 0.483 | 23.9 | 2.04 | 2008 | 18.9 |
| 9 | APCDBRP | 20 | 722 ± 59 | 0.412 | 32.8 | 7.58 | 1840 | 7.9 |
| 10 | DBR | 20 | 606 ± 33 | 0.748 | 31.4 | 2.23 | 902 | 54.4 |

SPRE-PBR vs RE-PBR: +5.4%, better on 18/20 topologies, p = 1.9e-05.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. EEDBR, 4. DVOR, 5. HydroCast, 6. DBR.

## cur0.3 — current 0.3 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1890 ± 157 | 0.637 | 12.9 | 3.25 | 3071 | 9.6 |
| 2 | RE-PBR | 20 | 1850 ± 150 | 0.641 | 13.7 | 3.44 | 2991 | 10.5 |
| 3 | QLFR | 20 | 1364 ± 76 | 0.571 | 20.1 | 1.92 | 2503 | 8.3 |
| 4 | HFS-PBR | 20 | 1258 ± 69 | 0.571 | 18.0 | 1.55 | 2351 | 14.9 |
| 5 | WDFAD-DBR | 20 | 1183 ± 95 | 0.432 | 18.7 | 1.86 | 2863 | 15.8 |
| 6 | EEDBR | 20 | 1124 ± 69 | 0.613 | 20.3 | 7.37 | 1958 | 16.5 |
| 7 | DVOR | 20 | 967 ± 47 | 0.524 | 27.9 | 2.57 | 1960 | 8.7 |
| 8 | HydroCast | 20 | 945 ± 103 | 0.626 | 29.1 | 4.91 | 1610 | 7.8 |
| 9 | APCDBRP | 20 | 785 ± 65 | 0.457 | 28.2 | 6.77 | 1827 | 8.0 |
| 10 | DBR | 20 | 608 ± 32 | 0.744 | 31.4 | 2.21 | 916 | 54.0 |

SPRE-PBR vs RE-PBR: +2.2%, better on 15/20 topologies, p = 0.036.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. HFS-PBR, 5. EEDBR, 6. DVOR, 7. HydroCast, 8. DBR.

## cur1 — current 1 m/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 951 ± 123 | 0.499 | 23.7 | 2.93 | 2009 | 9.2 |
| 2 | QLFR | 20 | 877 ± 60 | 0.554 | 27.3 | 2.31 | 1688 | 9.5 |
| 3 | RE-PBR | 20 | 867 ± 96 | 0.495 | 26.7 | 3.25 | 1856 | 10.8 |
| 4 | HFS-PBR | 20 | 825 ± 53 | 0.559 | 25.6 | 1.56 | 1594 | 17.3 |
| 5 | EEDBR | 20 | 794 ± 51 | 0.588 | 27.3 | 9.34 | 1467 | 16.2 |
| 6 | DVOR | 20 | 688 ± 30 | 0.516 | 35.6 | 2.62 | 1438 | 10.0 |
| 7 | WDFAD-DBR | 20 | 655 ± 64 | 0.380 | 32.6 | 1.81 | 1837 | 16.4 |
| 8 | HydroCast | 20 | 638 ± 69 | 0.582 | 39.7 | 5.01 | 1194 | 8.0 |
| 9 | DBR | 20 | 555 ± 24 | 0.732 | 34.6 | 2.22 | 855 | 55.2 |
| 10 | APCDBRP | 20 | 446 ± 41 | 0.368 | 48.6 | 7.61 | 1318 | 8.3 |

SPRE-PBR vs QLFR: +8.4%, better on 9/20 topologies, p = 0.23.
Among protocols with mean PDR ≥ 0.5: 1. QLFR, 2. HFS-PBR, 3. EEDBR, 4. DVOR, 5. HydroCast, 6. DBR.

## load0.5 — 0.5 pkt/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1874 ± 152 | 0.648 | 14.3 | 2.91 | 5867 | 9.2 |
| 2 | RE-PBR | 20 | 1812 ± 159 | 0.643 | 15.0 | 3.01 | 5716 | 9.8 |
| 3 | HFS-PBR | 20 | 1192 ± 75 | 0.590 | 19.9 | 1.53 | 4146 | 14.7 |
| 4 | EEDBR | 20 | 1043 ± 68 | 0.631 | 22.4 | 6.81 | 3417 | 16.7 |
| 5 | WDFAD-DBR | 20 | 1003 ± 60 | 0.507 | 21.5 | 1.95 | 4074 | 18.5 |
| 6 | QLFR | 20 | 969 ± 51 | 0.580 | 29.7 | 1.85 | 3440 | 8.3 |
| 7 | APCDBRP | 20 | 915 ± 94 | 0.523 | 25.8 | 5.93 | 3578 | 7.8 |
| 8 | HydroCast | 20 | 664 ± 71 | 0.686 | 42.2 | 4.50 | 2031 | 7.5 |
| 9 | DVOR | 20 | 653 ± 26 | 0.549 | 43.0 | 2.55 | 2490 | 8.6 |
| 10 | DBR | 20 | 574 ± 27 | 0.781 | 32.1 | 2.11 | 1570 | 57.7 |

SPRE-PBR vs RE-PBR: +3.4%, better on 17/20 topologies, p = 0.00048.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. HFS-PBR, 4. EEDBR, 5. WDFAD-DBR, 6. QLFR, 7. APCDBRP, 8. HydroCast, 9. DVOR, 10. DBR.

## load2 — 2 pkt/s

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 2108 ± 235 | 0.543 | 11.8 | 4.00 | 2022 | 9.4 |
| 2 | RE-PBR | 20 | 2019 ± 215 | 0.538 | 12.4 | 4.21 | 1960 | 10.0 |
| 3 | QLFR | 20 | 1824 ± 192 | 0.555 | 14.5 | 2.10 | 1736 | 8.8 |
| 4 | HFS-PBR | 20 | 1419 ± 117 | 0.516 | 15.5 | 1.61 | 1469 | 13.5 |
| 5 | DVOR | 20 | 1359 ± 89 | 0.482 | 18.6 | 2.60 | 1502 | 8.2 |
| 6 | WDFAD-DBR | 20 | 1272 ± 104 | 0.431 | 17.3 | 1.91 | 1572 | 15.1 |
| 7 | EEDBR | 20 | 1210 ± 102 | 0.574 | 18.1 | 7.56 | 1153 | 15.6 |
| 8 | HydroCast | 20 | 1164 ± 131 | 0.561 | 21.9 | 6.91 | 1130 | 8.3 |
| 9 | DBR | 20 | 677 ± 38 | 0.669 | 29.4 | 2.39 | 604 | 46.6 |
| 10 | APCDBRP | 20 | 669 ± 82 | 0.333 | 32.2 | 7.37 | 1108 | 7.2 |

SPRE-PBR vs RE-PBR: +4.4%, better on 16/20 topologies, p = 0.001.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. HFS-PBR, 5. EEDBR, 6. HydroCast, 7. DBR.

## range — ideal range channel

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 2529 ± 204 | 0.654 | 10.2 | 2.22 | 3919 | 8.3 |
| 2 | RE-PBR | 20 | 2424 ± 197 | 0.652 | 10.8 | 2.42 | 3772 | 9.1 |
| 3 | QLFR | 20 | 1996 ± 170 | 0.626 | 14.5 | 1.34 | 3249 | 5.9 |
| 4 | APCDBRP | 20 | 1866 ± 168 | 0.612 | 11.2 | 3.92 | 3110 | 6.2 |
| 5 | HFS-PBR | 20 | 1374 ± 85 | 0.691 | 15.9 | 1.45 | 2076 | 17.8 |
| 6 | DVOR | 20 | 1295 ± 65 | 0.751 | 19.8 | 2.66 | 1812 | 10.7 |
| 7 | WDFAD-DBR | 20 | 1217 ± 80 | 0.568 | 17.4 | 1.85 | 2238 | 19.5 |
| 8 | EEDBR | 20 | 1173 ± 71 | 0.722 | 18.9 | 5.86 | 1717 | 19.9 |
| 9 | HydroCast | 20 | 1095 ± 124 | 0.670 | 25.3 | 3.12 | 1714 | 6.5 |
| 10 | DBR | 20 | 580 ± 26 | 0.801 | 33.0 | 2.00 | 818 | 63.1 |

SPRE-PBR vs RE-PBR: +4.4%, better on 17/20 topologies, p = 8.2e-05.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. APCDBRP, 5. HFS-PBR, 6. DVOR, 7. WDFAD-DBR, 8. EEDBR, 9. HydroCast, 10. DBR.

## sinks1 — 1 sink

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 723 ± 101 | 0.242 | 34.9 | 4.64 | 3108 | 9.1 |
| 2 | RE-PBR | 20 | 689 ± 107 | 0.232 | 38.9 | 4.89 | 3110 | 9.6 |
| 3 | QLFR | 20 | 639 ± 65 | 0.356 | 34.4 | 3.41 | 1946 | 14.5 |
| 4 | HFS-PBR | 20 | 586 ± 76 | 0.198 | 46.0 | 1.92 | 3243 | 9.7 |
| 5 | EEDBR | 20 | 566 ± 81 | 0.263 | 50.5 | 9.19 | 2380 | 12.6 |
| 6 | DVOR | 20 | 560 ± 42 | 0.273 | 46.2 | 3.68 | 14842 | 8.1 |
| 7 | WDFAD-DBR | 20 | 389 ± 60 | 0.162 | 63.7 | 2.23 | 2512 | 16.9 |
| 8 | DBR | 20 | 343 ± 35 | 0.423 | 58.2 | 3.05 | 915 | 54.1 |
| 9 | APCDBRP | 20 | 308 ± 55 | 0.130 | 84.6 | 6.76 | 2629 | 5.5 |
| 10 | HydroCast | 20 | 306 ± 52 | 0.211 | 100.7 | 5.98 | 1546 | 8.4 |

SPRE-PBR vs RE-PBR: +5.0%, better on 12/20 topologies, p = 0.045.
Among protocols with mean PDR ≥ 0.5: none.

## sinks3 — 3 sinks

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 1717 ± 158 | 0.532 | 14.2 | 3.45 | 3300 | 9.6 |
| 2 | RE-PBR | 20 | 1632 ± 150 | 0.523 | 15.1 | 3.68 | 3193 | 10.3 |
| 3 | QLFR | 20 | 1337 ± 125 | 0.544 | 19.3 | 2.25 | 2538 | 10.2 |
| 4 | HFS-PBR | 20 | 1165 ± 72 | 0.462 | 19.5 | 1.68 | 2630 | 13.3 |
| 5 | EEDBR | 20 | 1011 ± 64 | 0.534 | 21.8 | 7.87 | 2006 | 16.0 |
| 6 | WDFAD-DBR | 20 | 940 ± 56 | 0.393 | 22.8 | 2.10 | 2507 | 17.1 |
| 7 | DVOR | 20 | 897 ± 48 | 0.470 | 29.7 | 2.80 | 2002 | 8.6 |
| 8 | HydroCast | 20 | 825 ± 82 | 0.549 | 32.5 | 5.63 | 1596 | 8.1 |
| 9 | APCDBRP | 20 | 666 ± 76 | 0.350 | 32.6 | 6.54 | 2002 | 7.5 |
| 10 | DBR | 20 | 556 ± 26 | 0.687 | 33.9 | 2.44 | 906 | 54.0 |

SPRE-PBR vs RE-PBR: +5.2%, better on 18/20 topologies, p = 0.00032.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. QLFR, 4. EEDBR, 5. HydroCast, 6. DBR.

## sinks7 — 7 sinks

| rank | protocol | n | Delivered before T10 | PDR | J / delivered | Delay (s) | T10 (s) | Data tx / generated |
|---|---|---|---|---|---|---|---|---|
| 1 | SPRE-PBR | 20 | 2494 ± 193 | 0.686 | 10.3 | 3.00 | 3690 | 9.1 |
| 2 | RE-PBR | 20 | 2384 ± 184 | 0.678 | 10.9 | 3.13 | 3572 | 9.7 |
| 3 | HFS-PBR | 20 | 1509 ± 107 | 0.639 | 15.0 | 1.47 | 2449 | 14.8 |
| 4 | QLFR | 20 | 1461 ± 45 | 0.578 | 19.7 | 1.67 | 2622 | 6.8 |
| 5 | WDFAD-DBR | 20 | 1304 ± 70 | 0.545 | 16.4 | 1.83 | 2496 | 17.2 |
| 6 | EEDBR | 20 | 1245 ± 83 | 0.665 | 17.9 | 6.54 | 1970 | 16.6 |
| 7 | HydroCast | 20 | 1170 ± 104 | 0.735 | 23.0 | 4.63 | 1681 | 7.4 |
| 8 | DVOR | 20 | 1098 ± 54 | 0.567 | 24.4 | 2.43 | 2028 | 8.4 |
| 9 | APCDBRP | 20 | 994 ± 85 | 0.538 | 22.5 | 6.06 | 1934 | 7.9 |
| 10 | DBR | 20 | 652 ± 31 | 0.787 | 29.0 | 2.06 | 926 | 53.4 |

SPRE-PBR vs RE-PBR: +4.6%, better on 17/20 topologies, p = 8.2e-05.
Among protocols with mean PDR ≥ 0.5: 1. SPRE-PBR, 2. RE-PBR, 3. HFS-PBR, 4. QLFR, 5. WDFAD-DBR, 6. EEDBR, 7. HydroCast, 8. DVOR, 9. APCDBRP, 10. DBR.

## Overall (14 cases)

| protocol | mean rank | best rank | worst rank | cases ranked 1st | mean rank among PDR ≥ 0.5 (cases) |
|---|---|---|---|---|---|
| SPRE-PBR | 1.2 | 1 | 4 | 13 | 1.0 (9) |
| RE-PBR | 2.1 | 2 | 3 | 0 | 2.0 (9) |
| QLFR | 3.4 | 1 | 6 | 1 | 3.0 (11) |
| HFS-PBR | 3.9 | 3 | 5 | 0 | 3.4 (9) |
| EEDBR | 5.1 | 2 | 8 | 0 | 4.8 (10) |
| WDFAD-DBR | 6.3 | 5 | 9 | 0 | 5.7 (3) |
| DVOR | 6.9 | 5 | 9 | 0 | 6.1 (8) |
| HydroCast | 8.4 | 7 | 10 | 0 | 6.4 (11) |
| APCDBRP | 8.4 | 4 | 10 | 0 | 6.7 (3) |
| DBR | 9.4 | 7 | 10 | 0 | 7.6 (11) |
