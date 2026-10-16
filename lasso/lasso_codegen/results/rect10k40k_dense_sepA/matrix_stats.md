# Matrix statistics

`D`: 10,000 by 40,000, 2,599,983 nonzeros (260 slots per row from 65 seeded column
permutations, duplicates summed: 257 to 260 per row, 64 or 65 per column), Gaussian values with
standard deviation 1/sqrt(260). `D^T D` would have 551,103,506 nonzeros (34.4% of its
40,000 by 40,000 entries; exact structural count), 212 times the nonzeros of `D`.

## ISTA, extracted operators (sepA)
| matrix | shape | storage | nnz |
|---|---|---|---:|
| `t0_state` | (10000, 40000) | csr | 2599983 |
| `t0_p_p0` | (10000, 10000) | csr | 10000 |
| `t1_state` | (40000, 40000) | csr | 40000 |
| `t1_t_t0` | (40000, 10000) | csr | 2599983 |
| `su_t_t1` | (40000, 40000) | csr | 40000 |

## FISTA, extracted operators (sepA)
| matrix | shape | storage | nnz |
|---|---|---|---:|
| `t0_state` | (10000, 80000) | csr | 3980620 |
| `t0_p_p0` | (10000, 10000) | csr | 10000 |
| `t1_state` | (40000, 80000) | csr | 40000 |
| `t1_t_t0` | (40000, 10000) | csr | 2599983 |
| `su_state` | (80000, 80000) | csr | 40000 |
| `su_t_t1` | (80000, 40000) | csr | 40000 |
| `t0_state__d_gamma` | (10000, 80000) | csr | 5199966 |
| `t1_state__d_gamma` | (40000, 80000) | csr | 80000 |
