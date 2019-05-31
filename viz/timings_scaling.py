import argparse
import collections
import glob

from matplotlib import pyplot as plt
import numpy as np

from viz import output_util


parser = argparse.ArgumentParser()
parser.add_argument('--input_pattern_async', type=str, required=True)
parser.add_argument('--input_pattern_sync', type=str, required=True)
parser.add_argument('--output_file', type=str, default=None)
args = parser.parse_args()


def main():
    worker_countss = []
    timess = []

    labels = ['Asynchronous', 'Synchronous']

    for i, input_pattern in enumerate([
            args.input_pattern_async,
            args.input_pattern_sync]):
        times = []
        worker_counts = []
        for filename in glob.glob(input_pattern):
            ret = output_util.read_output_file(filename)
            if 'WORKERS' in ret and 'TIME' in ret:
                worker_counts.append(ret['WORKERS'])
                times.append(ret['TIME'])

        if i == 0:
            min_ix = np.argmin(worker_counts)
            min_worker_count = np.min(worker_counts)
            max_worker_count = np.max(worker_counts)
            max_time = times[min_ix]

            ws = np.linspace(min_worker_count, max_worker_count, 1000)
            ideal_values = []
            for w in ws:
                ideal_values.append(max_time * ws[0] / w)
            plt.plot(
                ws,
                ideal_values,
                c='black',
                label='Ideal Scaling',
                zorder=1)

        plt.scatter(
            worker_counts,
            times,
            label='Observed Times: %s' % labels[i],
            zorder=10 + i,
            alpha=0.8)

    ax = plt.gca()
    ax.set_yscale('log')
    ax.set_xscale('log', basex=2)
    ax.grid(alpha=0.3)
    ax.set_axisbelow(True)
    ax.minorticks_on()
    ax.set_xlim(0.6, 100)
    ax.set_xlabel('Number of Workers')
    ax.set_ylabel('Time (Seconds)')
    plt.legend()
    plt.tight_layout()

    if args.output_file is None:
        plt.show()
    else:
        plt.savefig(args.output_file)


if __name__ == '__main__':
    main()
