import argparse

import numpy as np


parser = argparse.ArgumentParser()
parser.add_argument('--p', type=float, required=True)
parser.add_argument('--N', type=int, required=True)
parser.add_argument('--output_file', type=str, required=True)
parser.add_argument('--seed', type=int, required=True)
args = parser.parse_args()


_HEADER_TEMPLATE = "ER Laplacian: N = %d, p = %.5f, seed = %d"


def main():
    N = args.N
    p = args.p
    np.random.seed(args.seed)

    L = np.zeros((N, N))

    for i in range(N):
        for j in range(i + 1, N):
            if np.random.random() <= p:
                value = 1.0
            else:
                value = 0.0

            L[i, j] = -value
            L[j, i] = -value

    for i in range(N):
        L[i, i] = -np.sum(L[i, :])

    header_str = _HEADER_TEMPLATE % (N, p, args.seed)
    np.savetxt(
        args.output_file,
        L,
        fmt='%.5f',
        delimiter=',',
        header=header_str)


if __name__ == '__main__':
    main()
